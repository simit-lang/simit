#include "gpu_function.h"

#include <sstream>
#include "cuda.h"
#include "nvvm.h"

#include "cuda.h"
#include "nvvm.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

#include "error.h"
#include "ir.h"

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
  std::cout << "DEBUG: generatePtx" << std::endl
	    << module << std::endl;
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

  std::cout << "Ptx code: " << std::endl << std::string(ptx) << std::endl;

  return std::string(ptx);
}

}  // namespace

GPUFunction::GPUFunction(simit::ir::Func simitFunc,
                         llvm::Function *llvmFunc, llvm::Module *llvmModule)
    : Function(simitFunc), llvmFunc(llvmFunc), llvmModule(llvmModule) {}
GPUFunction::~GPUFunction() {}

CUdeviceptr GPUFunction::allocArg(const ir::Type& type) {
  switch (type.kind()) {
    case ir::Type::Tensor: {
      const ir::TensorType *ttype = type.toTensor();
      std::cout << "Tensor type, size: "
                << ttype->size() * ttype->componentType.bytes() << std::endl;
      CUdeviceptr devBuffer;
      checkCudaErrors(cuMemAlloc(
          &devBuffer, ttype->size() * ttype->componentType.bytes()));
      return devBuffer;
    }
    case ir::Type::Element:
    case ir::Type::Set:
    case ir::Type::Tuple: {
      std::cout << "ERROR ERROR ERROR!" << std::endl;
      exit(1);
      not_supported_yet;
      break;
    }
  }
}

const ir::Literal& GPUFunction::getArgData(Actual& actual) {
  switch (actual.getType().kind()) {
    case ir::Type::Tensor: {
      return *(ir::to<ir::Literal>(actual.getTensor()->expr()));
    }
    case ir::Type::Element:
    case ir::Type::Set:
    case ir::Type::Tuple: {
      std::cout << "ERROR ERROR ERROR!" << std::endl;
      exit(1);
      not_supported_yet;
      break;
    }
  }
}

void GPUFunction::pushArg(const ir::Literal& literal, CUdeviceptr &devBuffer) {
  std::cout << "[";
  char* data = reinterpret_cast<char*>(literal.data);
  for (int i = 0; i < literal.size; ++i) {
    if (i != 0) std::cout << ",";
    std::cout << std::hex << (int) data[i];
  }
  std::cout << "]" << std::endl;
  checkCudaErrors(cuMemcpyHtoD(devBuffer, literal.data, literal.size));
}

void GPUFunction::pullArg(const ir::Literal& literal, CUdeviceptr &devBuffer) {
  checkCudaErrors(cuMemcpyDtoH(literal.data, devBuffer, literal.size));
  std::cout << "Pulled: [";
  for (int i = 0; i < literal.size; ++i) {
    if (i != 0) std::cout << ",";
    std::cout << std::hex << (int) ((char*)literal.data)[i];
  }
  std::cout << "]" << std::endl;
}

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
    

  return [this, function, &formals, &actuals,
          cudaModule, context](){
    // Push data to GPU
    std::map<std::string, CUdeviceptr> kernelParams;
    int width = 1;
    int height = 1;
    for (const std::string& formal : formals) {
      assert(actuals.find(formal) != actuals.end());
      Actual &actual = actuals.at(formal);
      const ir::Literal &literal = getArgData(actual);
      CUdeviceptr devBuffer = allocArg(actual.getType());
      pushArg(literal, devBuffer);
      kernelParams[formal] = devBuffer;
    }

    // TODO(gkanwar): For now, fixed block sizes
    unsigned blockSizeX = 1;
    unsigned blockSizeY = 1;
    unsigned blockSizeZ = 1;
    unsigned gridSizeX = width/1;
    unsigned gridSizeY = height;
    unsigned gridSizeZ = 1;
    std::cout << "Launching CUDA kernel with block size ("
	      << blockSizeX << ","
	      << blockSizeY << ","
	      << blockSizeZ << ") and grid size ("
	      << gridSizeX << ","
	      << gridSizeY << ","
	      << gridSizeZ << ")" << std::endl;

    void **kernelParamsArr = new void*[kernelParams.size()];
    int i = 0; 
    for (const std::string& formal : formals) {
      kernelParamsArr[i] = reinterpret_cast<void*>(&(kernelParams[formal]));
      ++i;
    }
    checkCudaErrors(cuLaunchKernel(function, gridSizeX, gridSizeY, gridSizeZ,
                                   blockSizeX, blockSizeY, blockSizeZ, 0, NULL,
                                   kernelParamsArr, NULL));

    // TODO(gkanwar): Don't pull back input args
    for (const std::string &formal : formals) {
      assert(actuals.find(formal) != actuals.end());
      Actual &actual = actuals.at(formal);
      const ir::Literal &literal = getArgData(actual);
      CUdeviceptr &devPtr = kernelParams[formal];
      pullArg(literal, devPtr);
    }

    // Clean up
    for (auto kv : kernelParams) {
      checkCudaErrors(cuMemFree(kv.second));
    }
    // checkCudaErrors(cuCtxDestroy(context));
    checkCudaErrors(cuModuleUnload(cudaModule));
  };
}

}
}
