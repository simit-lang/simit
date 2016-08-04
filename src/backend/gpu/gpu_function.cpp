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
#include "graph_indices.h"
#include "ir.h"
#include "stencils.h"
#include "tensor_index.h"
#include "tensor_data.h"
#include "path_indices.h"
#include "backend/actual.h"
#include "backend/llvm/llvm_codegen.h"
#include "backend/llvm/llvm_versions.h"
#include "util/collections.h"

using namespace std;

namespace simit {
namespace backend {

size_t GPUFunction::DeviceDataHandle::total_allocations = 0;

static CUdeviceptr getGlobalDevPtr(CUmodule& cudaModule, std::string name,
                                   size_t expectedSize) {
  CUdeviceptr globalPtr;
  size_t globalPtrSize;
  checkCudaErrors(cuModuleGetGlobal(&globalPtr, &globalPtrSize,
                                    cudaModule, name.data()));
  iassert(globalPtrSize == expectedSize)
      << "Global pointer for " << name << " is wrong size. Got "
      << globalPtrSize << ", but expected " << expectedSize;

  // Checks for appropriate memory type
  unsigned int attrVal;
  checkCudaErrors(cuPointerGetAttribute(
      &attrVal, CU_POINTER_ATTRIBUTE_IS_MANAGED, globalPtr));
  iassert(attrVal == 1);

  return globalPtr;
}

GPUFunction::GPUFunction(
    ir::Func simitFunc, llvm::Function *llvmFunc,
    llvm::Module *module,
    std::shared_ptr<llvm::EngineBuilder> engineBuilder,
    const ir::Storage& storage)
    : LLVMFunction(simitFunc, storage, llvmFunc, module, engineBuilder),
      cudaModule(nullptr) {
  // CUDA runtime
  CUdevice device;
  int devCount;

  // CUDA setup
  checkCudaErrors(cuInit(0));
  checkCudaErrors(cuDeviceGetCount(&devCount));
  checkCudaErrors(cuDeviceGet(&device, 0));

  char name[128];
  checkCudaErrors(cuDeviceGetName(name, 128, device));
#ifdef SIMIT_DEBUG
  std::cout << "Using CUDA Device [0]: " << name << std::endl;
#endif

  checkCudaErrors(cuDeviceComputeCapability(&cuDevMajor, &cuDevMinor, device));
#ifdef SIMIT_DEBUG
  std::cout << "Device Compute Capability: "
            << cuDevMajor << "." << cuDevMinor << std::endl;
#endif
  iassert((cuDevMajor == 3 && cuDevMinor >= 5) || cuDevMajor > 3) << "ERROR: Device 0 is not SM 3.5 or greater";

#ifdef SIMIT_DEBUG
  size_t bytes;
  checkCudaErrors(cuDeviceTotalMem(&bytes, device));
  std::cout << "Total memory: " << bytes << std::endl;
#endif

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
  for (auto &kv : tensorData) {
    // Delete TensorData objects
    delete kv.second;
  }

  // Clear CUDA module, if any
  if (cudaModule) {
    checkCudaErrors(cuModuleUnload(*cudaModule));
    delete cudaModule;
  }

#ifdef SIMIT_DEBUG
  size_t free, total;
  cuMemGetInfo(&free, &total);
  std::cerr << "CUDA mem info: " << free << " free of " << total << std::endl;
#endif

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
    if (handle->hostBuffer ||
        handle->hostBufferConst) {
      // Short-circuit on size-zero buffer, because the CUDA API
      // doesn't like size-zero copies
      if (handle->size == 0) return;
      checkCudaErrors(cuMemcpyHtoD(
          *handle->devBuffer, handle->hostBuffer ?
          handle->hostBuffer : handle->hostBufferConst, handle->size));
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
void GPUFunction::bind(const std::string& name, TensorData& data) {
  LLVMFunction::bind(name, data);
  // We need to track the data sizes from TensorData, which LLVMFunction
  // doesn't do, so we maintain our own mapping
  tensorData[name] = new TensorData(data);
  initialized = false;
}

GPUFunction::SetData GPUFunction::pushSetData(Set* set, const ir::SetType* setType) {
  GPUFunction::SetData data = {0};
  // Set size(s)
  if (setType->isa<ir::UnstructuredSetType>()) {
    data.setSize = set->getSize();
  }
  else if (setType->isa<ir::LatticeLinkSetType>()) {
    CUdeviceptr *setSizesBuffer = new CUdeviceptr;
    unsigned dims = setType->to<ir::LatticeLinkSetType>()->dimensions;
    uassert(dims == set->getDimensions().size())
        << "Set bound to type with wrong number of dimensions: "
        << set->getDimensions().size() << " vs "
        << setType->to<ir::LatticeLinkSetType>()->dimensions;
    size_t size = dims*sizeof(int);
    checkCudaErrors(cuMemAlloc(setSizesBuffer, size));
    checkCudaErrors(cuMemcpyHtoD(
        *setSizesBuffer, set->getDimensions().data(), size));
    DeviceDataHandle *setSizesHandle = new DeviceDataHandle(
        set->getDimensions().data(), setSizesBuffer, size);
    pushedBufs.push_back(setSizesHandle);
    data.setSizes = setSizesHandle;
  }
  else {
    not_supported_yet;
  }

  // Short-circuit on a size-zero set, because the CUDA API doesn't like
  // size-zero allocations and copies
  if (set->getSize() == 0) {
    uwarning << "Binding a size-zero set: " << set->getName() << std::endl;
    CUdeviceptr *nullDevPtr = new CUdeviceptr;
    DeviceDataHandle *nullHandle = new DeviceDataHandle(
        (const void*)nullptr, nullDevPtr, 0);
    pushedBufs.push_back(nullHandle);
    data.endpoints = nullHandle;
    data.startIndex = nullHandle;
    data.nbrIndex = nullHandle;
    return data;
  }

  // Edge indices
  if (setType->isa<ir::UnstructuredSetType>() &&
      setType->to<ir::UnstructuredSetType>()->endpointSets.size() > 0) {
    uassert(set->getKind() == Set::Unstructured)
        << "Cannot bind non-unstructured set " << set->getName()
        << " to unstructured type.";

    // Endpoints index
    int *endpoints = set->getEndpointsData();
    CUdeviceptr *endpointBuffer = new CUdeviceptr();
    size_t size = set->getSize() * set->getCardinality() * sizeof(int);
    iassert(size != 0)
        << "Cannot allocate edge set with size-zero endpoints: "
        << set->getName();
    checkCudaErrors(cuMemAlloc(endpointBuffer, size));
    checkCudaErrors(cuMemcpyHtoD(*endpointBuffer, endpoints, size));
    DeviceDataHandle *endpointsHandle = new DeviceDataHandle(
        endpoints, endpointBuffer, size);
    pushedBufs.push_back(endpointsHandle);
    data.endpoints = endpointsHandle;
    // setData.push_back(llvmPtr(LLVM_INT_PTR,
    //                           reinterpret_cast<void*>(*endpointBuffer)));

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

    iassert(startSize != 0)
        << "Cannot allocate edge set with zero-sized start array: "
        << set->getName();
    checkCudaErrors(cuMemAlloc(startBuffer, startSize));
    checkCudaErrors(cuMemcpyHtoD(*startBuffer, startIndex, startSize));
    // Pushed bufs expects non-const pointers, because some are written to.
    DeviceDataHandle *startIndexHandle = new DeviceDataHandle(
        const_cast<int*>(startIndex), startBuffer, startSize);
    pushedBufs.push_back(startIndexHandle);
    data.startIndex = startIndexHandle;
    // setData.push_back(llvmPtr(LLVM_INT_PTR,
    //                           reinterpret_cast<void*>(*startBuffer)));

    iassert(nbrSize != 0)
        << "Cannot allocate edge set with zero-sized neighbor array: "
        << set->getName();
    checkCudaErrors(cuMemAlloc(nbrBuffer, nbrSize));
    checkCudaErrors(cuMemcpyHtoD(*nbrBuffer, nbrIndex, nbrSize));
    // Pushed bufs expects non-const pointers, because some are written to.
    DeviceDataHandle *nbrIndexHandle = new DeviceDataHandle(
        const_cast<int*>(nbrIndex), nbrBuffer, nbrSize);
    pushedBufs.push_back(nbrIndexHandle);
    data.nbrIndex = nbrIndexHandle;
    // setData.push_back(llvmPtr(LLVM_INT_PTR,
    //                           reinterpret_cast<void*>(*nbrBuffer)));
  }
  else if (setType->isa<ir::LatticeLinkSetType>()) {
    // Lattice link set type format leaves room for eps, nbrs_start and nbrs
    // pointers in the struct.
    CUdeviceptr *nullDevPtr = new CUdeviceptr;
    DeviceDataHandle *nullHandle = new DeviceDataHandle(
        (const void*)nullptr, nullDevPtr, 0);
    pushedBufs.push_back(nullHandle);
    data.endpoints = nullHandle;
    data.startIndex = nullHandle;
    data.nbrIndex = nullHandle;
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
    size_t size = set->getSize() * ttype->size() * ttype->getComponentType().bytes();
    iassert(size != 0)
        << "Cannot allocate set field of size 0: " << field.name;
    checkCudaErrors(cuMemAlloc(devBuffer, size));
    checkCudaErrors(cuMemcpyHtoD(*devBuffer, fieldData, size));
    DeviceDataHandle* handle = new DeviceDataHandle(fieldData, devBuffer, size);
    pushedBufs.push_back(handle);
    // std::cout << "Push field: " << field.name << std::endl;
    // std::cout << "[";
    // char* data = reinterpret_cast<char*>(fieldData);
    // for (size_t i = 0; i < size; ++i) {
    //   if (i != 0) std::cout << ",";
    //   std::cout << std::hex << (int) data[i];
    // }
    // std::cout << "]" << std::dec << std::endl;
    // std::cout << fieldData << " -> " << (void*)(*devBuffer) << std::endl;
    data.fields.push_back(handle);
    // setData.push_back(llvmPtr(*ttype, reinterpret_cast<void*>(*devBuffer)));
  }

  return data;
}

GPUFunction::DeviceDataHandle*
GPUFunction::pushGlobalTensor(
    const ir::Environment& env, const ir::Storage& storage,
    const ir::Var& bufVar, const ir::TensorType* ttype) {
  size_t bufSize = ttype->getComponentType().bytes();
  if (!isScalar(bufVar.getType())) {
    bufSize *= ttype->getBlockType().toTensor()->size();
    // Vectors have dense allocation
    if (ttype->order() == 1) {
      bufSize *= size(ttype->getOuterDimensions()[0]);
    }
    else if (ttype->order() == 2) {
      const ir::TensorStorage& ts = storage.getStorage(bufVar);
      switch (ts.getKind()) {
        case ir::TensorStorage::Kind::Dense: {
          // Only multiply size over outer dimensions, because we already
          // included block size
          for (auto &dim : ttype->getOuterDimensions()) {
            bufSize *= size(dim);
          }
          break;
        }
        case ir::TensorStorage::Kind::Diagonal: {
          // Just grab first outer dimension
          bufSize *= size(ttype->getOuterDimensions()[0]);
          break;
        }
        case ir::TensorStorage::Kind::Indexed: {
          if (env.hasTensorIndex(bufVar)) {
            const pe::PathExpression& pexpr =
                env.getTensorIndex(bufVar).getPathExpression();
            iassert(util::contains(pathIndices, pexpr));
            bufSize *= pathIndices.at(pexpr).numNeighbors();
          }
          // In the LLVM backend, we choose to initialize non tensor-index
          // tensors dynamically at runtime. The GPU does allocation here,
          // so we need to "statically" compute the right size.
          else {
	    iassert(ts.hasTensorIndex());
            const pe::PathExpression& pexpr =
	      ts.getTensorIndex().getPathExpression();
            iassert (util::contains(pathIndices, pexpr));
            bufSize *= pathIndices.at(pexpr).numNeighbors();
          }
          break;
        }
        case ir::TensorStorage::Stencil: {
          if (env.hasTensorIndex(bufVar)) {
            iassert(env.getTensorIndex(bufVar).getKind()
                    == ir::TensorIndex::Sten);
            const ir::StencilLayout& stencil =
                env.getTensorIndex(bufVar).getStencilLayout();
            // Size of stencil-base matrix is stencil size * dim[0]
            bufSize *= stencil.getLayout().size();
            bufSize *= size(ttype->getOuterDimensions()[0]);
          }
          else {
            iassert(ts.hasTensorIndex());
            iassert(ts.getTensorIndex().getKind() == ir::TensorIndex::Sten);
            const ir::StencilLayout& stencil =
                ts.getTensorIndex().getStencilLayout();
            // Size of stencil-base matrix is stencil size * dim[0]
            bufSize *= stencil.getLayout().size();
            bufSize *= size(ttype->getOuterDimensions()[0]);
          }
          break;
        }
        default: {
          ierror << "Can't compute matrix size for unknown TensorStorage: "
                 << bufVar;
        }
      }
    }
    else {
      tassert(!isSystemTensorType(ttype))
          << "Higher-order system tensor allocation not supported: " << bufVar;
      bufSize = ttype->size();
    }
  }
  CUdeviceptr *devBuffer = new CUdeviceptr();
  iassert(bufSize > 0)
      << "Cannot allocate size 0 global buffer for var: " << bufVar;
  checkCudaErrors(cuMemAlloc(devBuffer, bufSize));

  // Find the host-side data, if it exists
  DeviceDataHandle* handle;
  if (hasGlobal(bufVar.getName())) {
    iassert(util::contains(externPtrs, bufVar.getName()) &&
            externPtrs.at(bufVar.getName()).size() == 1);
    void *hostPtr = *externPtrs.at(bufVar.getName())[0];
    handle = new DeviceDataHandle(hostPtr, devBuffer, bufSize);
  }
  else {
    handle = new DeviceDataHandle((const void*)nullptr, devBuffer, bufSize);
  }

  return handle;
}

GPUFunction::SparseTensorData
GPUFunction::pushExternSparseTensor(const ir::Environment& env,
                                    const ir::Var& bufVar,
                                    const ir::TensorType* ttype) {
  iassert(tensorData.count(bufVar.getName()));
  TensorData* data = tensorData[bufVar.getName()];
  CUdeviceptr *dataBuffer = new CUdeviceptr();
  CUdeviceptr *rowPtrBuffer = new CUdeviceptr();
  CUdeviceptr *colIndBuffer = new CUdeviceptr();
  size_t dataSize = ttype->getComponentType().bytes() *
      ttype->getBlockType().toTensor()->size() *
      data->getDataLen();
  size_t colIndSize = sizeof(int)*data->getDataLen();
  size_t rowPtrSize = sizeof(int)*data->getRowLen();
  checkCudaErrors(cuMemAlloc(dataBuffer, dataSize));
  checkCudaErrors(cuMemAlloc(rowPtrBuffer, rowPtrSize));
  checkCudaErrors(cuMemAlloc(colIndBuffer, colIndSize));
  checkCudaErrors(cuMemcpyHtoD(
      *dataBuffer, data->getData(), dataSize));
  checkCudaErrors(cuMemcpyHtoD(
      *rowPtrBuffer, (void*)data->getRowPtr(), rowPtrSize));
  checkCudaErrors(cuMemcpyHtoD(
      *colIndBuffer, (void*)data->getColInd(), colIndSize));
  GPUFunction::SparseTensorData out;
  DeviceDataHandle *dataHandle = new DeviceDataHandle(
      data->getData(), dataBuffer, dataSize);
  DeviceDataHandle *rowPtrHandle = new DeviceDataHandle(
      (void*)data->getRowPtr(), rowPtrBuffer, rowPtrSize);
  DeviceDataHandle *colIndHandle = new DeviceDataHandle(
      (void*)data->getColInd(), colIndBuffer, colIndSize);
  pushedBufs.push_back(dataHandle);
  pushedBufs.push_back(rowPtrHandle);
  pushedBufs.push_back(colIndHandle);
  out.data = dataHandle;
  out.rowPtr = rowPtrHandle;
  out.colInd = colIndHandle;
  return out;
}

llvm::Value *GPUFunction::pushArg(std::string name, ir::Type& argType, Actual* actual) {
  // TODO: Use ActualVisitor

  if (isa<TensorActual>(actual)) {
    TensorActual* tActual = to<TensorActual>(actual);
    CUdeviceptr *devBuffer = new CUdeviceptr();
    const ir::TensorType *ttype = argType.toTensor();
    // TODO: How to handle scalar extern values??
    if (!isResult(name) && isScalar(argType)) {
      switch (ttype->getComponentType().kind) {
        case ir::ScalarType::Int:
          return llvmInt(*(int*)tActual->getData());
        case ir::ScalarType::Float:
          tassert(ir::ScalarType::floatBytes == sizeof(float))
              << "GPUFunction requires single precision floats";
          return llvmFP(*(float*)tActual->getData());
        case ir::ScalarType::Boolean:
          return llvmBool(*(bool*)tActual->getData());
        case ir::ScalarType::Complex:
          tassert(ir::ScalarType::floatBytes == sizeof(float))
              << "GPUFunction requires single precision floats";
          return llvmComplex(((float*)tActual->getData())[0],
                             ((float*)tActual->getData())[1]);
        default:
          ierror << "Unknown ScalarType: " << ttype->getComponentType().kind;
      }
    }
    else {
      size_t size = ttype->size() * ttype->getComponentType().bytes();
      checkCudaErrors(cuMemAlloc(devBuffer, size));
      checkCudaErrors(cuMemcpyHtoD(*devBuffer, tActual->getData(), size));
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
    GPUFunction::SetData pushedData = pushSetData(set, setType);

    llvm::StructType *llvmSetType = llvmType(setType);
    std::vector<llvm::Constant*> setData;
    // Set size
    if (setType->isa<ir::UnstructuredSetType>()) {
      setData.push_back(llvmInt(pushedData.setSize));
    }
    else if (setType->isa<ir::LatticeLinkSetType>()) {
      setData.push_back(llvmPtr(LLVM_INT_PTR, reinterpret_cast<void*>(
          *(pushedData.setSizes->devBuffer))));
    }
    else {
      unreachable;
    }
    // Edge set
    if (pushedData.endpoints != nullptr) {
      iassert(pushedData.startIndex != nullptr);
      iassert(pushedData.nbrIndex != nullptr);
      setData.push_back(llvmPtr(LLVM_INT_PTR, reinterpret_cast<void*>(
          *(pushedData.endpoints->devBuffer))));
      setData.push_back(llvmPtr(LLVM_INT_PTR, reinterpret_cast<void*>(
          *(pushedData.startIndex->devBuffer))));
      setData.push_back(llvmPtr(LLVM_INT_PTR, reinterpret_cast<void*>(
          *(pushedData.nbrIndex->devBuffer))));
    }
    // Fields
    ir::Type ety = setType->elementType;
    iassert(ety.isElement()) << "Set element type must be ElementType.";
    const ir::ElementType* etype = ety.toElement();
    std::vector<DeviceDataHandle*> fieldHandles;
    for (int i = 0; i < etype->fields.size(); ++i) {
      ir::Type fty = etype->fields[i].type;
      iassert(fty.isTensor()) << "Field type must be tensor";
      const ir::TensorType* ttype = fty.toTensor();
      setData.push_back(llvmPtr(*ttype, reinterpret_cast<void*>(
          *(pushedData.fields[i]->devBuffer))));
      fieldHandles.push_back(pushedData.fields[i]);
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
  // Short-circuit on size-zero buffer, because the CUDA API
  // doesn't like size-zero copies
  if (handle->size == 0) return;
  iassert(handle->hostBuffer)
      << "Must define mutable hostBuffer to allow pulling arg";
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
  if (handle->size > 0) {
    checkCudaErrors(cuMemFree(*handle->devBuffer));
  }
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
  LLVM_Metadata *mdVals[] = {
    LLVM_MD_WRAP(harness),
    llvm::MDString::get(LLVM_CTX, "kernel"),
    LLVM_MD_WRAP(llvmInt(1))
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

  const ir::Environment& env = getEnvironment();

  // Alloc and push arguments and globals, then build harness
  llvm::SmallVector<llvm::Value*, 8> args;
  vector<string> formals = getArgs();
  for (const std::string& name : formals) {
    ir::Type argType = getArgType(name);
    Actual *actual = arguments[name].get();
    args.push_back(pushArg(name, argType, actual));
    if (isa<SetActual>(actual)) {
      Set* set = to<SetActual>(actual)->getSet();
      piBuilder.bind(name, set);
    }
  }

  // Initialize indices
  initIndices(piBuilder, env);

  // Create harnesses for kernel args
  llvm::Function *harness = createHarness(args, llvmFunc, module);

  // Validate LLVM module
  iassert(!llvm::verifyModule(*module))
      << "LLVM module does not pass verification";

  // Generate harness PTX
#ifdef SIMIT_DEBUG
  std::cout << "Create PTX" << std::endl;
#endif
  std::string ptxStr = generatePtx(
      module, cuDevMajor, cuDevMinor,
      module->getModuleIdentifier().c_str());

#ifdef SIMIT_DEBUG
  std::ofstream ptxFile("simit.ptx", std::ofstream::trunc);
  ptxFile << ptxStr << std::endl;
  ptxFile.close();
#endif

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

  // Allocate all globals (i.e. externs and temporaries) and
  // set the CUDA global pointers
  std::vector<ir::Var> globalVars = env.getExternVars();
  globalVars.insert(globalVars.begin(),
                    env.getTemporaries().begin(), env.getTemporaries().end());
  for (auto& bufVar : globalVars) {
    if (bufVar.getType().isTensor()) {
      const ir::TensorType* ttype = bufVar.getType().toTensor();
      // Special case for extern sparse tensors, which get bound
      // differently
      if (env.hasExtern(bufVar.getName()) &&
          externPtrs.at(bufVar.getName()).size() > 1) {
        iassert(externPtrs.at(bufVar.getName()).size() == 3)
            << "Extern sparse tensors should have three extern "
            << "pointers for: data, rowPtr, colInd";
        GPUFunction::SparseTensorData pushedData =
            pushExternSparseTensor(env, bufVar, ttype);

        // Arg buf map does not include the indices, since we do not
        // write that data, and thus is never needs to be dirtied.
        std::vector<DeviceDataHandle*> handleVec = {pushedData.data};
        argBufMap.emplace(bufVar.getName(), handleVec);

        const ir::VarMapping& mapping = env.getExtern(bufVar.getName());
        iassert(mapping.getMappings().size() == 3)
            << "Extern sparse tensor should be mapped to three vars, for "
            << "rowPtr, colInd, and data.";
        const ir::Var& dataVar = mapping.getMappings()[0];
        const ir::Var& rowPtrVar = mapping.getMappings()[1];
        const ir::Var& colIndVar = mapping.getMappings()[2];
        CUdeviceptr globalPtr = getGlobalDevPtr(
            *cudaModule, dataVar.getName(), sizeof(void*));
        cuMemcpyHtoD(globalPtr, reinterpret_cast<void*>(
            pushedData.data->devBuffer), sizeof(void*));
        globalPtr = getGlobalDevPtr(
            *cudaModule, rowPtrVar.getName(), sizeof(void*));
        cuMemcpyHtoD(globalPtr, reinterpret_cast<void*>(
            pushedData.rowPtr->devBuffer), sizeof(void*));
        globalPtr = getGlobalDevPtr(
            *cudaModule, colIndVar.getName(), sizeof(void*));
        cuMemcpyHtoD(globalPtr, reinterpret_cast<void*>(
            pushedData.colInd->devBuffer), sizeof(void*));
      }
      else {
        DeviceDataHandle *handle = pushGlobalTensor(
            env, storage, bufVar, ttype);
        pushedBufs.push_back(handle);

        std::vector<DeviceDataHandle*> handleVec = {handle};
        argBufMap.emplace(bufVar.getName(), handleVec);

        void *devBufferPtr = reinterpret_cast<void*>(handle->devBuffer);
        CUdeviceptr globalPtr = getGlobalDevPtr(
            *cudaModule, bufVar.getName(), sizeof(void*));
        cuMemcpyHtoD(globalPtr, devBufferPtr, sizeof(void*));
      }
    }
    else if (bufVar.getType().isSet()) {
      // Set globals must correspond to host-side data, because we don't know
      // how big the allocation must be in general.
      iassert(hasGlobal(bufVar.getName()))
          << "Cannot allocate unbound set extern: " << bufVar.getName();
      const ir::SetType* setType = bufVar.getType().toSet();
      Set* set = to<SetActual>(globals[bufVar.getName()].get())->getSet();
      GPUFunction::SetData pushedData = pushSetData(set, setType);
      std::vector<DeviceDataHandle*> handleVec;
      
      vector<uint8_t> setData;
      if (setType->isa<ir::UnstructuredSetType>()) {
        // setSize
        int idx = setData.size();
        setData.resize(setData.size() + sizeof(int));
        memcpy(setData.data()+idx, (uint8_t*)(&pushedData.setSize), sizeof(int));
      }
      else if (setType->isa<ir::LatticeLinkSetType>()) {
        // setSizes
        int idx = setData.size();
        setData.resize(setData.size() + sizeof(void*));
        memcpy(setData.data()+idx, (uint8_t*)(&pushedData.setSizes), sizeof(void*));
      }
      else {
        unreachable;
      }
      // Padding to 8-byte alignment (only needed if something comes afterwards)
      if ((setType->isa<ir::UnstructuredSetType>() &&
           setType->to<ir::UnstructuredSetType>()->getCardinality() > 0) ||
          setType->isa<ir::LatticeLinkSetType>() ||
          pushedData.fields.size() > 0) {
        if (setData.size() % 8 != 0) {
          setData.resize(setData.size() + (8-setData.size()%8));
          iassert(setData.size() % 8 == 0); // double check this math
        }
      }
      // Neighbors structures
      if ((setType->isa<ir::UnstructuredSetType>() &&
           setType->to<ir::UnstructuredSetType>()->getCardinality() > 0) ||
          setType->isa<ir::LatticeLinkSetType>()) {
        int idx = setData.size();
        setData.resize(setData.size() + 3*sizeof(void*));
        memcpy(setData.data()+idx, (void*)(pushedData.endpoints->devBuffer), sizeof(void*));
        idx += sizeof(void*);
        memcpy(setData.data()+idx, (void*)(pushedData.startIndex->devBuffer), sizeof(void*));
        idx += sizeof(void*);
        memcpy(setData.data()+idx, (void*)(pushedData.nbrIndex->devBuffer), sizeof(void*));
        idx += sizeof(void*);
        iassert(idx == setData.size());
        handleVec.push_back(pushedData.endpoints);
        handleVec.push_back(pushedData.startIndex);
        handleVec.push_back(pushedData.nbrIndex);
      }
      // Fields
      // NOTE: This code assumes the width of void* is the same as
      // and float*/int* on the GPU.
      for (DeviceDataHandle *fieldHandle : pushedData.fields) {
        int idx = setData.size();
        setData.resize(setData.size() + sizeof(void*));
        memcpy(setData.data()+idx, (void*)(fieldHandle->devBuffer), sizeof(void*));
        handleVec.push_back(fieldHandle);
      }
      argBufMap.emplace(bufVar.getName(), handleVec);

      CUdeviceptr globalPtr = getGlobalDevPtr(
          *cudaModule, bufVar.getName(), setData.size());
      cuMemcpyHtoD(globalPtr, (void*)setData.data(), setData.size());
    }
  }
  // for (auto& kv : globals) {
  //   std::string name = kv.first;
  //   std::cout << "Set global ptr: " << name << std::endl;
  //   std::vector<DeviceDataHandle*> handles = argBufMap[name];
  //   iassert(handles.size() == 1)
  //       << "Globals must correspond to exactly one pushed buffer" << std::endl
  //       << "Found: " << handles.size();
  //   DeviceDataHandle* handle = handles[0];
  //   CUdeviceptr *devBuffer = handle->devBuffer;
  //   void *devBufferPtr = reinterpret_cast<void*>(*devBuffer);
  //   void *globalPtrHost = getGlobalHostPtr(*cudaModule, name);
  //   *((void**)globalPtrHost) = devBufferPtr;
  //   std::cout << "store in: " << globalPtrHost << std::endl;
  //   std::cout << "globalPtrHost: " << *((void**)globalPtrHost) << std::endl;
  // }

  // Initialize tensor index data
  for (const ir::TensorIndex& tensorIndex : env.getTensorIndices()) {
    if (tensorIndex.getKind() != ir::TensorIndex::PExpr) {
      continue;
    }
    const pe::PathExpression& pexpr = tensorIndex.getPathExpression();
    const uint32_t** rowptrDataPtr = tensorIndexPtrs[pexpr].first;
    const uint32_t** colidxDataPtr = tensorIndexPtrs[pexpr].second;
    CUdeviceptr *devRowptrBuffer = new CUdeviceptr();
    CUdeviceptr *devColidxBuffer = new CUdeviceptr();

    // Push data to GPU
    const pe::PathIndex& pidx = pathIndices[pexpr];
    if (isa<pe::SegmentedPathIndex>(pidx)) {
      const pe::SegmentedPathIndex* spidx = to<pe::SegmentedPathIndex>(pidx);
      size_t rowptrSize = (spidx->numElements()+1)*sizeof(uint32_t);
      size_t colidxSize = spidx->numNeighbors()*sizeof(uint32_t);
      checkCudaErrors(cuMemAlloc(devRowptrBuffer, rowptrSize));
      checkCudaErrors(cuMemcpyHtoD(
          *devRowptrBuffer, *rowptrDataPtr, rowptrSize));
      
      checkCudaErrors(cuMemAlloc(devColidxBuffer, colidxSize));
      checkCudaErrors(cuMemcpyHtoD(
          *devColidxBuffer, *colidxDataPtr, colidxSize));
    }
    else {
      not_supported_yet;
    }

    const ir::Var& rowptrs = tensorIndex.getRowptrArray();
    CUdeviceptr rowptrsPtr = getGlobalDevPtr(
        *cudaModule, rowptrs.getName(), sizeof(void*));
    cuMemcpyHtoD(rowptrsPtr, reinterpret_cast<void*>(devRowptrBuffer),
                 sizeof(void*));

    const ir::Var& colidx = tensorIndex.getColidxArray();
    CUdeviceptr colidxPtr = getGlobalDevPtr(
        *cudaModule, colidx.getName(), sizeof(void*));
    cuMemcpyHtoD(colidxPtr, reinterpret_cast<void*>(devColidxBuffer),
                 sizeof(void*));
  }

  // Get reference to CUDA function
  checkCudaErrors(cuModuleGetFunction(
      &cudaFunction, *cudaModule, harness->getName().data()));

  // Mark initialized
  initialized = true;

  return [this, env, cudaFunction](){
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
      if (isResult(name)) {
        for (auto &handle : argBufMap[name]) {
          handle->devDirty = true;
        }
      }
    }
    for (auto& ext : env.getExternVars()) {
      for (auto &handle : argBufMap[ext.getName()]) {
        handle->devDirty = true;
      }
    }
  };
}

}
}
