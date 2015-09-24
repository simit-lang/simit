#include "gpu_function.h"

#include <sstream>
#include <iomanip>
#include <fstream>

#include "cuda.h"
#include "nvvm.h"

#if LLVM_MAJOR_VERSION <= 3 && LLVM_MINOR_VERSION <= 4
#include "llvm/Analysis/Verifier.h"
#else
#include "llvm/IR/Verifier.h"
#endif

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"

#include "error.h"
#include "graph.h"
#include "indices.h"
#include "ir.h"
#include "tensor_index.h"
#include "path_indices.h"
#include "backend/actual.h"
#include "backend/llvm/llvm_codegen.h"
#include "util/collections.h"

using namespace std;

namespace simit {
namespace backend {

size_t GPUFunction::DeviceDataHandle::total_allocations = 0;

GPUFunction::GPUFunction(
    ir::Func simitFunc, llvm::Function *llvmFunc,
    llvm::Module *module,
    std::shared_ptr<llvm::EngineBuilder> engineBuilder,
    std::map<ir::Var, llvm::Value*> globalBufs,
    const ir::Storage& storage)
    : LLVMFunction(simitFunc, storage, llvmFunc, module, engineBuilder),
      globalBufs(globalBufs), cudaModule(nullptr) {
  // CUDA runtime
  CUdevice device;
  int devCount;

  // CUDA setup
  checkCudaErrors(cuInit(0));
  checkCudaErrors(cuDeviceGetCount(&devCount));
  checkCudaErrors(cuDeviceGet(&device, 0));

  char name[128];
  checkCudaErrors(cuDeviceGetName(name, 128, device));
  // TODO(gkanwar): Figure out logging system
  std::cout << "Using CUDA Device [0]: " << name << std::endl;

  checkCudaErrors(cuDeviceComputeCapability(&cuDevMajor, &cuDevMinor, device));
  std::cout << "Device Compute Capability: "
            << cuDevMajor << "." << cuDevMinor << std::endl;
  iassert((cuDevMajor == 3 && cuDevMinor >= 5) || cuDevMajor > 3) << "ERROR: Device 0 is not SM 3.5 or greater";

  size_t bytes;
  checkCudaErrors(cuDeviceTotalMem(&bytes, device));
  std::cout << "Total memory: " << bytes << std::endl;

  // Create driver context
  cudaContext = new CUcontext();
  checkCudaErrors(cuCtxCreate(cudaContext, 0, device));

  // Check managed memory, required for pushing globals
  int attrVal;
  checkCudaErrors(cuDeviceGetAttribute(&attrVal, CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING, device));
  iassert(attrVal == 1);
}

GPUFunction::~GPUFunction() {
  for (DeviceDataHandle *handle : pushedBufs) {
    freeArg(handle);
  }

  // Clear CUDA module, if any
  if (cudaModule) {
    checkCudaErrors(cuModuleUnload(*cudaModule));
    delete cudaModule;
  }

  size_t free, total;
  cuMemGetInfo(&free, &total);
  std::cerr << "CUDA mem info: " << free << " free of " << total << std::endl;

  // Release driver context, if any
  if (cudaContext) {
    checkCudaErrors(cuCtxDestroy(*cudaContext));
    delete cudaContext;
  }
}

void GPUFunction::mapArgs() {
  // Pull args back from GPU -> CPU
  for (DeviceDataHandle *handle : pushedBufs) {
    if (handle->devDirty) pullArg(handle);
  }
}

void GPUFunction::unmapArgs(bool updated) {
  // Skip unmapping if args are not updated
  if (!updated) return;

  for (DeviceDataHandle *handle : pushedBufs) {
    // Push non-null args from CPU -> GPU
    if (handle->hostBuffer) {
      checkCudaErrors(cuMemcpyHtoD(
          *handle->devBuffer, handle->hostBuffer, handle->size));
    }
  }
}

// LLVMFunction binds arguments the way GPUFunction wants to, but
// is too permissive with dirtying the initialized bit. GPUFunction
// needs to push any data to the GPU, so we must always initialize
// after a bind.
void GPUFunction::bind(const std::string& name, simit::Set* set) {
  LLVMFunction::bind(name, set);
  initialized = false;
}
void GPUFunction::bind(const std::string& name, void* data) {
  LLVMFunction::bind(name, data);
  initialized = false;
}
void GPUFunction::bind(const std::string& name, const int* rowPtr,
                       const int* colInd, void* data) {
  LLVMFunction::bind(name, rowPtr, colInd, data);
  initialized = false;
}


llvm::Value *GPUFunction::pushArg(std::string name, Actual* actual) {
  // TODO: Use ActualVisitor

  ir::Type argType = getArgType(name);

  // std::cout << "Push arg: " << formal << std::endl;
  if (isa<TensorActual>(actual)) {
    TensorActual* tActual = to<TensorActual>(actual);
    CUdeviceptr *devBuffer = new CUdeviceptr();
    const ir::TensorType *ttype = argType.toTensor();
    // std::cout << "[";
    // char* data = reinterpret_cast<char*>(literal.data);
    // for (size_t i = 0; i < literal.size; ++i) {
    //   if (i != 0) std::cout << ",";
    //   std::cout << std::hex << (int) data[i];
    // }
    // std::cout << "]" << std::dec << std::endl;
    if (!isArgResult(name) && isScalar(argType)) {
      switch (ttype->componentType.kind) {
        case ir::ScalarType::Int:
          return llvmInt(*(int*)tActual->getData());
        case ir::ScalarType::Float:
          tassert(ir::ScalarType::floatBytes == sizeof(float))
              << "GPUFunction requires single precision floats";
          return llvmFP(*(float*)tActual->getData());
        case ir::ScalarType::Boolean:
          return llvmBool(*(bool*)tActual->getData());
        default:
          ierror << "Unknown ScalarType: " << ttype->componentType.kind;
      }
    }
    else {
      size_t size = ttype->size() * ttype->componentType.bytes();
      checkCudaErrors(cuMemAlloc(devBuffer, size));
      checkCudaErrors(cuMemcpyHtoD(*devBuffer, tActual->getData(), size));
      // std::cout << literal.data << " -> " << (void*)(*devBuffer) << std::endl;
      pushedBufs.push_back(
          new DeviceDataHandle(tActual->getData(), devBuffer, size));
      std::vector<DeviceDataHandle*> argBufs = { pushedBufs.back() };
      argBufMap.emplace(name, argBufs);
      return llvmPtr(*ttype, reinterpret_cast<void*>(*devBuffer));
    }
  }
  else if (isa<SetActual>(actual)) {
    SetActual* sActual = to<SetActual>(actual);
    Set *set = sActual->getSet();
    const ir::SetType *setType = argType.toSet();

    llvm::StructType *llvmSetType = llvmType(*setType);
    std::vector<llvm::Constant*> setData;

    // Set size
    setData.push_back(llvmInt(set->getSize()));

    // Edge indices
    if (setType->endpointSets.size() > 0) {
      // Endpoints index
      int *endpoints = set->getEndpointsData();
      CUdeviceptr *endpointBuffer = new CUdeviceptr();
      size_t size = set->getSize() * set->getCardinality() * sizeof(int);
      if (size != 0) {
        checkCudaErrors(cuMemAlloc(endpointBuffer, size));
        checkCudaErrors(cuMemcpyHtoD(*endpointBuffer, endpoints, size));
        pushedBufs.push_back(new DeviceDataHandle(
            endpoints, endpointBuffer, size));
      }
      setData.push_back(llvmPtr(LLVM_INT_PTR,
                                reinterpret_cast<void*>(*endpointBuffer)));

      // Edges index
      // TODO

      // Neighbor index
      const internal::NeighborIndex *nbrs = set->getNeighborIndex();
      const int *startIndex = nbrs->getStartIndex();
      size_t startSize = (set->getEndpointSet(0)->getSize()+1) * sizeof(int);
      const int *nbrIndex = nbrs->getNeighborIndex();
      size_t nbrSize = nbrs->getSize() * sizeof(int);
      // Sentinel is present and correct
      iassert(startIndex[set->getEndpointSet(0)->getSize()] == nbrs->getSize())
          << "Sentinel: " << startIndex[set->getEndpointSet(0)->getSize()]
          << " does not match neighbor size: " << nbrs->getSize();
      CUdeviceptr *startBuffer = new CUdeviceptr();
      CUdeviceptr *nbrBuffer = new CUdeviceptr();

      if (startSize != 0) {
        checkCudaErrors(cuMemAlloc(startBuffer, startSize));
        checkCudaErrors(cuMemcpyHtoD(*startBuffer, startIndex, startSize));
        // Pushed bufs expects non-const pointers, because some are written to.
        pushedBufs.push_back(new DeviceDataHandle(
            const_cast<int*>(startIndex), startBuffer, startSize));
      }
      setData.push_back(llvmPtr(LLVM_INT_PTR,
                                reinterpret_cast<void*>(*startBuffer)));

      if (nbrSize != 0) {
        checkCudaErrors(cuMemAlloc(nbrBuffer, nbrSize));
        checkCudaErrors(cuMemcpyHtoD(*nbrBuffer, nbrIndex, nbrSize));
        // Pushed bufs expects non-const pointers, because some are written to.
        pushedBufs.push_back(new DeviceDataHandle(
            const_cast<int*>(nbrIndex), nbrBuffer, nbrSize));
      }
      setData.push_back(llvmPtr(LLVM_INT_PTR,
                                reinterpret_cast<void*>(*nbrBuffer)));
    }

    // Fields
    ir::Type etype = setType->elementType;
    iassert(etype.isElement()) << "Set element type must be ElementType.";

    std::vector<DeviceDataHandle*> fieldHandles;
    for (auto field : etype.toElement()->fields) {
      CUdeviceptr *devBuffer = new CUdeviceptr();
      ir::Type ftype = field.type;
      iassert(ftype.isTensor()) << "Element field must be tensor type";
      const ir::TensorType *ttype = ftype.toTensor();
      void *fieldData = set->getFieldData(field.name);
      size_t size = set->getSize() * ttype->size() * ttype->componentType.bytes();
      if (size != 0) {
        checkCudaErrors(cuMemAlloc(devBuffer, size));
        checkCudaErrors(cuMemcpyHtoD(*devBuffer, fieldData, size));
        pushedBufs.push_back(
            new DeviceDataHandle(fieldData, devBuffer, size));
        fieldHandles.push_back(pushedBufs.back());
        // std::cout << "Push field: " << field.name << std::endl;
        // std::cout << "[";
        // char* data = reinterpret_cast<char*>(fieldData);
        // for (size_t i = 0; i < size; ++i) {
        //   if (i != 0) std::cout << ",";
        //   std::cout << std::hex << (int) data[i];
        // }
        // std::cout << "]" << std::dec << std::endl;
        // std::cout << fieldData << " -> " << (void*)(*devBuffer) << std::endl;
      }
      setData.push_back(llvmPtr(*ttype, reinterpret_cast<void*>(*devBuffer)));
    }
    argBufMap.emplace(name, fieldHandles);

    return llvm::ConstantStruct::get(llvmSetType, setData);
  }

  ierror << "Unhandle actual: " << actual;
  return NULL;
}

void GPUFunction::pullArg(DeviceDataHandle* handle) {
  // std::cout << "Pull arg: " << (void*)(*handle->devBuffer)
  //           << " -> " << handle->hostBuffer
  //           << " (" << handle->size << ")" << std::endl;
  checkCudaErrors(cuMemcpyDtoH(
      handle->hostBuffer, *handle->devBuffer, handle->size));
  handle->devDirty = false;
  // std::cout << "[";
  // char* data = reinterpret_cast<char*>(handle->hostBuffer);
  // for (size_t i = 0; i < handle->size; ++i) {
  //   if (i != 0) std::cout << ",";
  //   std::cout << std::hex << (int) data[i];
  // }
  // std::cout << "]" << std::dec << std::endl;
}

void GPUFunction::freeArg(DeviceDataHandle* handle) {
  checkCudaErrors(cuMemFree(*handle->devBuffer));
  delete handle->devBuffer;
  delete handle;
}

void GPUFunction::print(std::ostream &os) const {
  std::string moduleStr;
  llvm::raw_string_ostream str(moduleStr);
  str << *module;
  os << moduleStr << std::endl << std::endl;
  // TODO(gkanwar): Print out CUDA data setup aspects as well
}

// Takes ownership of kernel pointer, because it is passed to llvm::CallInst
llvm::Function *GPUFunction::createHarness(
    const llvm::SmallVector<llvm::Value*, 8> &args,
    llvm::Function *kernel,
    llvm::Module *module) {
  std::string kernelName = kernel->getName().str();
  const std::string harnessName = kernelName + std::string("_harness");
  llvm::Function *harness = createPrototype(harnessName, {}, {},
                                            module, true, false);

  auto entry = llvm::BasicBlock::Create(LLVM_CTX, "entry", harness);
  // Ensure the function declaration is present in harness module
  // TODO(gkanwar): Just using the kernel type here gives a bug in LLVM
  // parsing of the kernel declaration (incorrect type)
  std::vector<llvm::Type*> argTys;
  for (auto arg : args) {
    argTys.push_back(arg->getType());
  }
  module->getOrInsertFunction(
      kernelName,
      llvm::FunctionType::get(LLVM_VOID, argTys, false));
  // Note: CallInst takes ownership of kernel
  llvm::CallInst *call = llvm::CallInst::Create(
      kernel, args, "", entry);
  call->setCallingConv(kernel->getCallingConv());
  llvm::ReturnInst::Create(LLVM_CTX, entry);

  // Kernel metadata
  llvm::Value *mdVals[] = {
    harness, llvm::MDString::get(LLVM_CTX, "kernel"), llvmInt(1)
  };
  llvm::MDNode *kernelMD = llvm::MDNode::get(LLVM_CTX, mdVals);
  llvm::NamedMDNode *nvvmAnnot = module
      ->getOrInsertNamedMetadata("nvvm.annotations");
  nvvmAnnot->addOperand(kernelMD);

  return harness;
}

backend::Function::FuncType
GPUFunction::init() {
  CUlinkState linker;
  CUfunction cudaFunction;

  pe::PathIndexBuilder piBuilder;

  // Free any old device data
  for (DeviceDataHandle *handle : pushedBufs) {
    freeArg(handle);
  }
  pushedBufs.clear();
  argBufMap.clear();

  // Push data and build harness
  llvm::SmallVector<llvm::Value*, 8> args;
  for (auto& kv : arguments) {
    std::string name = kv.first;
    Actual *actual = kv.second.get();
    args.push_back(pushArg(name, actual));
    if (isa<SetActual>(actual)) {
      Set* set = to<SetActual>(actual)->getSet();
      piBuilder.bind(name, set);
    }
  }

  const ir::Environment& env = getEnvironment();

  // Initialize indices
  initIndices(piBuilder, env);

  // TODO: Temporaries are handled by globalBufs construct, use env instead

  // Create harnesses for kernel args
  llvm::Function *harness = createHarness(args, llvmFunc, module);

  // Validate LLVM module
  iassert(!llvm::verifyModule(*module))
      << "LLVM module does not pass verification";

  // Generate harness PTX
  std::cout << "Create PTX" << std::endl;
  std::string ptxStr = generatePtx(
      module, cuDevMajor, cuDevMinor,
      module->getModuleIdentifier().c_str());

  std::ofstream ptxFile("simit.ptx", std::ofstream::trunc);
  ptxFile << ptxStr << std::endl;
  ptxFile.close();

  // JIT linker and final CUBIN
  char linkerInfo[16384];
  char linkerErrors[16384];
  CUjit_option linkerOptions[] = {
    CU_JIT_INFO_LOG_BUFFER,
    CU_JIT_INFO_LOG_BUFFER_SIZE_BYTES,
    CU_JIT_ERROR_LOG_BUFFER,
    CU_JIT_ERROR_LOG_BUFFER_SIZE_BYTES,
    CU_JIT_LOG_VERBOSE
  };
  void *linkerOptionValues[] = {
    linkerInfo,
    reinterpret_cast<void*>(16384),
    linkerErrors,
    reinterpret_cast<void*>(16384),
    reinterpret_cast<void*>(1)
  };

  checkCudaErrors(cuLinkCreate(5, linkerOptions, linkerOptionValues, &linker));
  checkCudaErrors(cuLinkAddData(
      linker, CU_JIT_INPUT_PTX, (void*)ptxStr.c_str(),
      ptxStr.size(), "<compiled-ptx>", 0, NULL, NULL));
  // libcudadevrt.a
  checkCudaErrors(cuLinkAddFile(linker, CU_JIT_INPUT_LIBRARY,
                                LIBCUDADEVRT, 0, NULL, NULL));

  void *cubin;
  size_t cubinSize;
  checkCudaErrors(cuLinkComplete(linker, &cubin, &cubinSize));

  std::ofstream linkerLog("simit.linker.log", std::ofstream::trunc);
  linkerLog << linkerInfo << std::endl
            << linkerErrors << std::endl;
  linkerLog.close();

  // Create CUDA module for binary object, possibly removing previous module
  if (cudaModule) {
    checkCudaErrors(cuModuleUnload(*cudaModule));
  }
  else {
    cudaModule = new CUmodule();
  }
  checkCudaErrors(cuModuleLoadDataEx(cudaModule, cubin, 0, 0, 0));
  checkCudaErrors(cuLinkDestroy(linker));

  // Alloc global buffers and set global pointers
  for (auto& buf : globalBufs) {
    const ir::Var &bufVar = buf.first;
    llvm::Value *bufVal = buf.second;

    CUdeviceptr globalPtr;
    size_t globalPtrSize;
    checkCudaErrors(cuModuleGetGlobal(&globalPtr, &globalPtrSize,
                                      *cudaModule, bufVal->getName().data()));
    iassert(globalPtrSize == sizeof(void*))
        << "Global pointers should all be pointer-sized. Got: "
        << globalPtrSize << " bytes";

    // Checks for appropriate memory type
    unsigned int attrVal;
    checkCudaErrors(cuPointerGetAttribute(
        &attrVal, CU_POINTER_ATTRIBUTE_IS_MANAGED, globalPtr));
    iassert(attrVal == 1);
    checkCudaErrors(cuPointerGetAttribute(
        &attrVal, CU_POINTER_ATTRIBUTE_MEMORY_TYPE, globalPtr));
    iassert(attrVal == CU_MEMORYTYPE_DEVICE);

    const ir::TensorType* ttype = bufVar.getType().toTensor();
    size_t bufSize = ttype->componentType.bytes();
    if (!isScalar(bufVar.getType())) {
      bufSize *= ttype->getBlockType().toTensor()->size();
      // Vectors have dense allocation
      if (ttype->order() == 1) {
        bufSize *= size(ttype->getDimensions()[0]);
      }
      else if (ttype->order() == 2) {
        const pe::PathExpression& pexpr =
            env.getTensorIndex(bufVar).getPathExpression();
        iassert(util::contains(pathIndices, pexpr));
        bufSize *= pathIndices.at(pexpr).numNeighbors();
      }
      else {
        ierror << "Higher-order tensor allocation not supported";
      }
    }
    CUdeviceptr *devBuffer = new CUdeviceptr();
    iassert(bufSize > 0)
        << "Cannot allocate size 0 global buffer for var: " << bufVar;
    checkCudaErrors(cuMemAlloc(devBuffer, bufSize));
    pushedBufs.push_back(new DeviceDataHandle(nullptr, devBuffer, bufSize));
    void *devBufferPtr = reinterpret_cast<void*>(*devBuffer);

    void *globalPtrHost;
    checkCudaErrors(cuPointerGetAttribute(
        &globalPtrHost, CU_POINTER_ATTRIBUTE_HOST_POINTER, globalPtr));
    *((void**)globalPtrHost) = devBufferPtr;
  }

  // Get reference to CUDA function
  checkCudaErrors(cuModuleGetFunction(
      &cudaFunction, *cudaModule, harness->getName().data()));

  return [this, cudaFunction](){
    // std::cerr << "Allocated GPU memory: "
    //           << DeviceDataHandle::total_allocations << "\n";
    void **kernelParamsArr = new void*[0]; // TODO leaks
    checkCudaErrors(cuLaunchKernel(cudaFunction,
                                   1, 1, 1, // grid size
                                   1, 1, 1, // block size
                                   0, NULL,
                                   kernelParamsArr, NULL));
    // Set device dirty bit for all output arg buffers
    for (auto& pair : arguments) {
      std::string name = pair.first;
      if (isArgResult(name)) {
        // std::cout << "Dirtying " << formal << std::endl;
        for (auto &handle : argBufMap[name]) {
          handle->devDirty = true;
        }
      }
    }
  };
}

}
}
