#include "gpu_function.h"

#include <sstream>
#include <iomanip>
#include "cuda.h"
#include "nvvm.h"

#include "cuda.h"
#include "nvvm.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

#include "error.h"
#include "graph.h"
#include "indices.h"
#include "ir.h"
#include "llvm_codegen.h"

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
  return std::to_string(num);
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

GPUFunction::GPUFunction(ir::Func simitFunc, llvm::Function *llvmFunc,
                         llvm::Module *llvmModule, struct GPUSharding sharding)
    : Function(simitFunc), llvmFunc(llvmFunc),
      llvmModule(llvmModule), sharding(sharding){}
GPUFunction::~GPUFunction() {
  // llvmFunc will be destroyed when the harness funtion object is destroyed
  // because it was claimed by a CallInst
  llvmFunc.release();
}

llvm::Value *GPUFunction::pushArg(
    Actual& actual,
    std::map<void*, DeviceDataHandle> &pushedBufs) {
  switch (actual.getType().kind()) {
    case ir::Type::Tensor: {
      CUdeviceptr *devBuffer = new CUdeviceptr();
      const ir::TensorType *ttype = actual.getType().toTensor();
      // std::cout << "Tensor type, size: "
      //           << ttype->size() * ttype->componentType.bytes() << std::endl;
      checkCudaErrors(cuMemAlloc(
          devBuffer, ttype->size() * ttype->componentType.bytes()));
      const ir::Literal &literal = *(ir::to<ir::Literal>(*actual.getTensor()));
      // std::cout << "[";
      // char* data = reinterpret_cast<char*>(literal.data);
      // for (int i = 0; i < literal.size; ++i) {
      //   if (i != 0) std::cout << ",";
      //   std::cout << std::hex << (int) data[i];
      // }
      // std::cout << "]" << std::endl;
      checkCudaErrors(cuMemcpyHtoD(*devBuffer, literal.data, literal.size));
      // std::cout << "Stored in: " << *devBuffer << std::endl;
      pushedBufs.emplace(literal.data,
                         DeviceDataHandle(devBuffer, literal.size));
      return llvmPtr(actual.getType(), reinterpret_cast<void*>(*devBuffer));
    }
    case ir::Type::Element: ierror << "Element arg not supported";
    case ir::Type::Set: {
      SetBase *set = actual.getSet();
      const ir::SetType *setType = actual.getType().toSet();

      llvm::StructType *llvmSetType = createLLVMType(setType);
      std::vector<llvm::Constant*> setData;

      // Set size
      setData.push_back(llvmInt(set->getSize()));

      // Edge indices
      if (setType->endpointSets.size() > 0) {
        // Endpoints index
        int *endpoints = set->getEndpointsData();
        CUdeviceptr *endpointBuffer = new CUdeviceptr();
        size_t size = set->getSize() * set->getCardinality() * sizeof(int);
        checkCudaErrors(cuMemAlloc(endpointBuffer, size));
        checkCudaErrors(cuMemcpyHtoD(*endpointBuffer, endpoints, size));
        pushedBufs.emplace(endpoints,
                           DeviceDataHandle(endpointBuffer, size, false));
        setData.push_back(llvmPtr(LLVM_INTPTR,
                                  reinterpret_cast<void*>(*endpointBuffer)));

        // Edges index
        // TODO

        // Neighbor index
        const internal::NeighborIndex *nbrs = set->getNeighborIndex();
        const int *startIndex = nbrs->getStartIndex();
        size_t startSize = set->getSize() * sizeof(int);
        const int *nbrIndex = nbrs->getNeighborIndex();
        size_t nbrSize = nbrs->getSize() * sizeof(int);
        CUdeviceptr *startBuffer = new CUdeviceptr();
        CUdeviceptr *nbrBuffer = new CUdeviceptr();

        checkCudaErrors(cuMemAlloc(startBuffer, startSize));
        checkCudaErrors(cuMemcpyHtoD(*startBuffer, startIndex, startSize));
        // Pushed bufs expects non-const pointers, because some are written two.
        // Because we specify shouldPull=false in DeviceDataHandle, this will
        // not be written, but must be casted anyway.
        pushedBufs.emplace(const_cast<int*>(startIndex),
                           DeviceDataHandle(startBuffer, startSize, false));
        setData.push_back(llvmPtr(LLVM_INTPTR,
                                  reinterpret_cast<void*>(*startBuffer)));

        checkCudaErrors(cuMemAlloc(nbrBuffer, nbrSize));
        checkCudaErrors(cuMemcpyHtoD(*nbrBuffer, nbrIndex, nbrSize));
        // Pushed bufs expects non-const pointers, because some are written two.
        // Because we specify shouldPull=false in DeviceDataHandle, this will
        // not be written, but must be casted anyway.
        pushedBufs.emplace(const_cast<int*>(nbrIndex),
                           DeviceDataHandle(nbrBuffer, nbrSize, false));
        setData.push_back(llvmPtr(LLVM_INTPTR,
                                  reinterpret_cast<void*>(*nbrBuffer)));
      }

      // Fields
      ir::Type etype = setType->elementType;
      iassert(etype.isElement()) << "Set element type must be ElementType.";

      for (auto field : etype.toElement()->fields) {
        CUdeviceptr *devBuffer = new CUdeviceptr();
        ir::Type ftype = field.type;
        iassert(ftype.isTensor()) << "Element field must be tensor type";
        const ir::TensorType *ttype = ftype.toTensor();
        void *fieldData = set->getFieldData(field.name);
        size_t size = set->getSize() * ttype->size() * ttype->componentType.bytes();
        checkCudaErrors(cuMemAlloc(devBuffer, size));
        checkCudaErrors(cuMemcpyHtoD(*devBuffer, fieldData, size));
        pushedBufs.emplace(fieldData, DeviceDataHandle(devBuffer, size));
        setData.push_back(llvmPtr(ftype, reinterpret_cast<void*>(*devBuffer)));
      }

      return llvm::ConstantStruct::get(llvmSetType, setData);
    }
    case ir::Type::Tuple: ierror << "Tuple arg not supported";
    default: ierror << "Unknown arg type";
  }
}

void GPUFunction::pullArgAndFree(void *hostPtr, DeviceDataHandle handle) {
  if (handle.shouldPull) {
    checkCudaErrors(cuMemcpyDtoH(hostPtr, *handle.devBuffer, handle.size));
  }
  checkCudaErrors(cuMemFree(*handle.devBuffer));
}

void GPUFunction::print(std::ostream &os) const {
  std::string fstr;
  llvm::raw_string_ostream rsos(fstr);
  llvmFunc->print(rsos);
  os << fstr;
  // TODO(gkanwar): Print out CUDA data setup aspects as well
}

llvm::Function *GPUFunction::createHarness(
    const llvm::SmallVector<llvm::Value*, 8> &args) {
  const std::string harnessName = "kernel";
  llvm::Function *harness = createFunction(harnessName, {}, {},
                                           llvmModule.get(), true, false);

  auto entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", harness);
  // Note: CallInst takes ownership of llvmFunc. This is resolved in
  // the destructor
  llvm::CallInst *call = llvm::CallInst::Create(
      llvmFunc.get(), args, "", entry);
  call->setCallingConv(llvmFunc->getCallingConv());
  llvm::ReturnInst::Create(LLVM_CONTEXT, entry);

  // Kernel metadata
  llvm::Value *mdVals[] = {
    harness, llvm::MDString::get(LLVM_CONTEXT, harnessName), llvmInt(1)
  };
  llvm::MDNode *kernelMD = llvm::MDNode::get(LLVM_CONTEXT, mdVals);
  llvm::NamedMDNode *nvvmAnnot = llvmModule
      ->getOrInsertNamedMetadata("nvvm.annotations");
  nvvmAnnot->addOperand(kernelMD);

  return harness;
}

int GPUFunction::findShardSize(ir::IndexSet domain) {
  if (domain.getKind() == ir::IndexSet::Range) {
    return domain.getSize();
  }
  else if (domain.getKind() == ir::IndexSet::Set) {
    actuals[ir::to<struct ir::VarExpr>(domain.getSet())->var.getName()]
        .getSet()->getSize();
  }
  else {
    ierror << "Invalid domain kind: " << domain.getKind();
  }
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
  iassert(devMajor >= 2) << "ERROR: Device 0 is not SM 2.0 or greater";

  // Create driver context
  checkCudaErrors(cuCtxCreate(&context, 0, device));

  // Push data and build harness
  llvm::SmallVector<llvm::Value*, 8> args;
  std::map<void*, DeviceDataHandle> pushedBufs;
  for (const std::string& formal : formals) {
    assert(actuals.find(formal) != actuals.end());
    Actual &actual = actuals.at(formal);
    args.push_back(pushArg(actual, pushedBufs));
  }
  // TODO(gkanwar): For now, fixed block sizes
  unsigned blockSizeX = sharding.xSharded ?
      findShardSize(sharding.xDomain) : 1;
  unsigned blockSizeY = sharding.ySharded ?
      findShardSize(sharding.yDomain) : 1;
  unsigned blockSizeZ = sharding.zSharded ?
      findShardSize(sharding.zDomain) : 1;
  unsigned gridSizeX = 1;
  unsigned gridSizeY = 1;
  unsigned gridSizeZ = 1;
  // Create harness as the kernel to be run
  createHarness(args);

  // Export IR to string
  std::string moduleStr;
  llvm::raw_string_ostream str(moduleStr);
  str << *(llvmModule.get());

  // Generate PTX
  std::string ptx = generatePtx(moduleStr, devMajor, devMinor,
                                llvmModule->getModuleIdentifier().c_str());

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
                               

  return [this, function, cudaModule, context, pushedBufs,
          blockSizeX, blockSizeY, blockSizeZ,
          gridSizeX, gridSizeY, gridSizeZ](){
    // Push data to GPU
    std::cout << "Launching CUDA kernel with block size ("
	      << blockSizeX << ","
	      << blockSizeY << ","
	      << blockSizeZ << ") and grid size ("
	      << gridSizeX << ","
	      << gridSizeY << ","
	      << gridSizeZ << ")" << std::endl;

    void **kernelParamsArr = new void*[0];
    // int i = 0; 
    // for (const std::string& formal : formals) {
    //   GPUFunction::GPUArgHandle &handle = kernelParams[formal];
    //   if (actuals.at(formal).getType().isSet()) {
    //     for (auto &kv : handle.devBufferFields) {
    //       kernelParamsArr[i++] = reinterpret_cast<void*>(kv.second);
    //     }
    //   }
    //   else {
    //     kernelParamsArr[i++] = reinterpret_cast<void*>(
    //         handle.devBufferFields["main"]);
    //   }
    // }
    // for (i = 0; i < numArgs; ++i) {
    //     std::cout << "Kernel param loc: "
    //               << kernelParamsArr[i] << std::endl;
    // }
    checkCudaErrors(cuLaunchKernel(function, gridSizeX, gridSizeY, gridSizeZ,
                                   blockSizeX, blockSizeY, blockSizeZ, 0, NULL,
                                   kernelParamsArr, NULL));

    // Pull args back to CPU, clean up device buffers
    for (auto &kv : pushedBufs) {
      pullArgAndFree(kv.first, kv.second);
    }

    // Clean up
    // for (auto kv : kernelParams) {
    //   for (auto kv2 : kv.second.devBufferFields) {
    //     checkCudaErrors(cuMemFree(*kv2.second));
    //   }
    // }
    // checkCudaErrors(cuCtxDestroy(context));
    checkCudaErrors(cuModuleUnload(cudaModule));
  };
}

}
}
