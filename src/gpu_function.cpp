#include "gpu_function.h"

#include <sstream>

#include "cuda.h"
#include "nvvm.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

#include "error.h"

namespace simit {
namespace internal {

namespace {

// This will output the proper CUDA error strings in the event that a CUDA
// host call returns an error
#define checkCudaErrors(err)  __checkCudaErrors (err, __FILE__, __LINE__)

// These are the inline versions for all of the SDK helper functions
void __checkCudaErrors(CUresult err, const char *file, const int line) {
  if(CUDA_SUCCESS != err) {
    ierror << "checkCudaErrors() Driver API error = " << err
           << "from file <" << file
           << ", line " << line;
  }
}

/// checkNVVMCall - Verifies that NVVM result code is success, or terminate
/// otherwise.
void checkNVVMCall(nvvmResult res) {
  iassert(res == NVVM_SUCCESS) << "libnvvm call failed";
}

std::string utostr(uint num) {
  return static_cast<std::ostringstream*>( &(std::ostringstream() << num) )->str();
}

// Uses libnvvm to compile an LLVM IR module to PTX.
std::string generatePtx(const std::string &module,
                        int devMajor, int devMinor,
                        const char *moduleName) {
  nvvmProgram compileUnit;
  nvvmResult res;

  // NVVM Initialization
  checkNVVMCall(nvvmCreateProgram(&compileUnit));

  // Create NVVM compilation unit from LLVM IR
  checkNVVMCall(nvvmAddModuleToProgram(compileUnit,
                                       module.c_str(), module.size(),
                                       moduleName));

  std::string computeArg = "-arch=compute_";
  computeArg += utostr(devMajor);
  computeArg += utostr(devMinor);

  const char *options[] = { computeArg.c_str() };

  // Compile LLVM IR into PTX
  res = nvvmCompileProgram(compileUnit, 1, options);
  if (res != NVVM_SUCCESS) {
    std::cerr << "nvvmCompileProgram failed!" << std::endl;
    size_t logSize;
    nvvmGetProgramLogSize(compileUnit, &logSize);
    char *msg = new char[logSize];
    nvvmGetProgramLog(compileUnit, msg);
    std::cerr << msg << std::endl;
    delete [] msg;
    ierror << "nvvmCompileProgram failed";
  }

  size_t ptxSize;
  checkNVVMCall(nvvmGetCompiledResultSize(compileUnit, &ptxSize));
  char *ptx = new char[ptxSize];
  checkNVVMCall(nvvmGetCompiledResult(compileUnit, ptx));
  checkNVVMCall(nvvmDestroyProgram(&compileUnit));

  return std::string(ptx);
}

}  // namespace

GPUFunction::GPUFunction(simit::ir::Func simitFunc,
                         llvm::Function *llvmFunc, llvm::Module *llvmModule)
    : Function(simitFunc), llvmFunc(llvmFunc), llvmModule(llvmModule) {}
GPUFunction::~GPUFunction() {}

void GPUFunction::print(std::ostream &os) const {
  std::string fstr;
  llvm::raw_string_ostream rsos(fstr);
  llvmFunc->print(rsos);
  os << fstr;
  // TODO(gkanwar): Print out CUDA data setup aspects as well
}

simit::Function::FuncType GPUFunction::init(
    const std::vector<std::string> &formals,
    std::map<std::string, Actual> &actuals) {
  // CUDA runtime
  CUdevice device;
  CUmodule cudaModule;
  CUcontext context;
  CUfunction function;
  CUlinkState linker;
  int devCount;

  // CUDA setup
  checkCudaErrors(cuInit(0));
  checkCudaErrors(cuDeviceGetCount(&devCount));
  checkCudaErrors(cuDeviceGet(&device, 0));

  char name[128];
  checkCudaErrors(cuDeviceGetName(name, 128, device));
  // TODO(gkanwar): Figure out logging system
  std::cout << "Using CUDA Device [0]: " << name << std::endl;

  int devMajor, devMinor;
  checkCudaErrors(cuDeviceComputeCapability(&devMajor, &devMinor, device));
  std::cout << "Device Compute Capability: "
            << devMajor << "." << devMinor << std::endl;
  iassert(devMajor < 2) << "ERROR: Device 0 is not SM 2.0 or greater";

  // Export IR to string
  std::string moduleStr;
  llvm::raw_string_ostream str(moduleStr);
  str << *(llvmModule.get());

  // Generate PTX
  std::string ptx = generatePtx(moduleStr, devMajor, devMinor,
                                llvmModule->getModuleIdentifier().c_str());

  // Create driver context
  checkCudaErrors(cuCtxCreate(&context, 0, device));

  // JIT linker and final CUBIN
  char linkerInfo[1024];
  char linkerErrors[1024];
  CUjit_option linkerOptions[] = {
    CU_JIT_INFO_LOG_BUFFER,
    CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES,
    CU_JIT_ERROR_LOG_BUFFER,
    CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES,
    CU_JIT_LOG_VERBOSE
  };
  void *linkerOptionValues[] = {
    linkerInfo,
    reinterpret_cast<void*>(1024),
    linkerErrors,
    reinterpret_cast<void*>(1024),
    reinterpret_cast<void*>(1)
  };

  checkCudaErrors(cuLinkCreate(5, linkerOptions, linkerOptionValues, &linker));
  checkCudaErrors(cuLinkAddData(linker, CU_JIT_INPUT_PTX, (void*)ptx.c_str(),
                                ptx.size(), "<compiled-ptx>", 0, NULL, NULL));

  void *cubin;
  size_t cubinSize;
  checkCudaErrors(cuLinkComplete(linker, &cubin, &cubinSize));

  std::cout << "Linker log:" << std::endl
            << linkerInfo << std::endl
            << linkerErrors << std::endl;

  // Create CUDA module for binary object
  checkCudaErrors(cuModuleLoadDataEx(&cudaModule, cubin, 0, 0, 0));
  checkCudaErrors(cuLinkDestroy(linker));
  checkCudaErrors(cuModuleGetFunction(&function, cudaModule, "kernel"));
    
  // Push data to GPU
  std::vector<void*> kernelParams;
  iassert(formals.size() == llvmFunc->arg_size());
  int width = 1;
  int height = 1;
  auto formal = formals.begin();
  auto arg = llvmFunc->arg_begin();
  while (formal != formals.end()) {
    iassert (arg != llvmFunc->arg_end());
    iassert(actuals.find(*formal) != actuals.end());
    iassert(*formal == arg->getName());

    Actual &actual = actuals.at(*formal);
    switch (actual.getType().kind()) {
      case ir::Type::Tensor: {
        auto actualPtr = ir::to<ir::Literal>(actual.getTensor()->expr());
        CUdeviceptr devBuffer;
        checkCudaErrors(cuMemAlloc(&devBuffer, actualPtr->size));
        checkCudaErrors(cuMemcpyHtoD(devBuffer, actualPtr->data, actualPtr->size));
        kernelParams.push_back(&devBuffer);
        // Computation size
        width = actual.getTensor()->type().toTensor()->size();
        break;
      }
      case ir::Type::Element: {
        not_supported_yet;
        break;
      }
      case ir::Type::Set: {
        // TODO(gkanwar)
        ierror;
        break;
      }
      case ir::Type::Tuple: {
        not_supported_yet;
        break;
      }
    }

    ++formal;
    ++arg;
  }

  return [&function, &kernelParams, &cudaModule, &context, &width, &height](){
    // TODO(gkanwar): For now, fixed block sizes
    unsigned blockSizeX = 16;
    unsigned blockSizeY = 1;
    unsigned blockSizeZ = 1;
    unsigned gridSizeX = width/16;
    unsigned gridSizeY = height;
    unsigned gridSizeZ = 1;

    void **kernelParamsArr = new void*[kernelParams.size()];
    for (int i = 0; i < kernelParams.size(); ++i) {
      kernelParamsArr[i] = kernelParams[i];
    }
    checkCudaErrors(cuLaunchKernel(function, gridSizeX, gridSizeY, gridSizeZ, blockSizeX, blockSizeY, blockSizeZ, 0, NULL, kernelParamsArr, NULL));
    // TODO(gkanwar): Get data back

    // Clean up
    for (void *devPtr : kernelParams) {
      checkCudaErrors(cuMemFree(*reinterpret_cast<CUdeviceptr*>(devPtr)));
      checkCudaErrors(cuModuleUnload(cudaModule));
      checkCudaErrors(cuCtxDestroy(context));
    }
  };
}

}
}
