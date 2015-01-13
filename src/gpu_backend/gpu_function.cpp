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

GPUFunction::~GPUFunction() {
  // llvmFunc will be destroyed when the harness funtion object is destroyed
  // because it was claimed by a CallInst
  llvmFunc.release();

  for (auto &kv : pushedBufs) {
    freeArg(kv.second);
  }
}

void GPUFunction::mapArgs() {
  // Pull args back to CPU
  for (auto &kv : pushedBufs) {
    if (kv.second.shouldPull) pullArg(kv.first, kv.second);
  }
}

void GPUFunction::unmapArgs(bool updated) {
  // TODO implement
}

llvm::Value *GPUFunction::pushArg(Actual& actual, bool shouldPull) {
  std::cout << "Push arg" << std::endl;
  switch (actual.getType().kind()) {
    case ir::Type::Tensor: {
      CUdeviceptr *devBuffer = new CUdeviceptr();
      const ir::TensorType *ttype = actual.getType().toTensor();
      checkCudaErrors(cuMemAlloc(
          devBuffer, ttype->size() * ttype->componentType.bytes()));
      const ir::Literal &literal = *(ir::to<ir::Literal>(*actual.getTensor()));
      std::cout << "[";
      char* data = reinterpret_cast<char*>(literal.data);
      for (size_t i = 0; i < literal.size; ++i) {
        if (i != 0) std::cout << ",";
        std::cout << std::hex << (int) data[i];
      }
      std::cout << "]" << std::dec << std::endl;
      checkCudaErrors(cuMemcpyHtoD(*devBuffer, literal.data, literal.size));
      std::cout << literal.data << " -> " << *devBuffer << std::endl;
      pushedBufs.emplace(literal.data,
                         DeviceDataHandle(devBuffer, literal.size, shouldPull));
      if (!actual.isOutput() && isScalar(actual.getType())) {
        switch (ttype->componentType.kind) {
          case ir::ScalarType::Int:
            return llvmInt(*(int*)literal.data);
          case ir::ScalarType::Float:
            return llvmFP(literal.getFloatVal(0));
          case ir::ScalarType::Boolean:
            return llvmBool(*(bool*)literal.data);
          default:
            ierror << "Unknown ScalarType: " << ttype->componentType.kind;
        }
      }
      else {
        return llvmPtr(actual.getType(), reinterpret_cast<void*>(*devBuffer),
                       LLVM_GLOBAL_ADDRSPACE);
      }
    }
    case ir::Type::Element: ierror << "Element arg not supported";
    case ir::Type::Set: {
      SetBase *set = actual.getSet();
      const ir::SetType *setType = actual.getType().toSet();

      llvm::StructType *llvmSetType =
          createLLVMType(setType, LLVM_GLOBAL_ADDRSPACE);
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
        setData.push_back(llvmPtr(LLVM_INTPTR_GLOBAL,
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
        setData.push_back(llvmPtr(LLVM_INTPTR_GLOBAL,
                                  reinterpret_cast<void*>(*startBuffer)));

        checkCudaErrors(cuMemAlloc(nbrBuffer, nbrSize));
        checkCudaErrors(cuMemcpyHtoD(*nbrBuffer, nbrIndex, nbrSize));
        // Pushed bufs expects non-const pointers, because some are written two.
        // Because we specify shouldPull=false in DeviceDataHandle, this will
        // not be written, but must be casted anyway.
        pushedBufs.emplace(const_cast<int*>(nbrIndex),
                           DeviceDataHandle(nbrBuffer, nbrSize, false));
        setData.push_back(llvmPtr(LLVM_INTPTR_GLOBAL,
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
        pushedBufs.emplace(fieldData, DeviceDataHandle(devBuffer, size, shouldPull));
        setData.push_back(llvmPtr(ftype, reinterpret_cast<void*>(*devBuffer),
                                  LLVM_GLOBAL_ADDRSPACE));
      }

      return llvm::ConstantStruct::get(llvmSetType, setData);
    }
    case ir::Type::Tuple: ierror << "Tuple arg not supported";
    default: ierror << "Unknown arg type";
  }
  assert(false && "unreachable");
  return NULL;
}

void GPUFunction::pullArg(void *hostPtr, DeviceDataHandle handle) {
  std::cout << "Pull arg: " << (void*)(*handle.devBuffer) << " -> " << hostPtr
            << " (" << handle.size << ")" << std::endl;
  checkCudaErrors(cuMemcpyDtoH(hostPtr, *handle.devBuffer, handle.size));
}

/* TODO
void GPUFunction::pushArg(void *hostPtr, DeviceDataHandle handle) {
  std::cout << "Push arg: " << *handle.devBuffer << " <- " << hostPtr
            << " (" << handle.size << ")" << std::endl;
  checkCudaErrors(cuMemcpyHtoD(*handle.devBuffer, hostPtr, handle.size));
}
*/

void GPUFunction::freeArg(DeviceDataHandle handle) {
  checkCudaErrors(cuMemFree(*handle.devBuffer));
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
  const std::string harnessName = kernel->getName().str() + "_harness";
  llvm::Function *harness = createPrototype(harnessName, {}, {},
                                            module, true, false);

  auto entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", harness);
  // Ensure the function declaration is present in harness module
  // TODO(gkanwar): Just using the kernel type here gives a bug in LLVM
  // parsing of the kernel declaration (incorrect type)
  std::vector<llvm::Type*> argTys;
  for (auto arg : args) {
    argTys.push_back(arg->getType());
  }
  module->getOrInsertFunction(
      kernel->getName(),
      llvm::FunctionType::get(LLVM_VOID, argTys, false));
  // Note: CallInst takes ownership of kernel
  llvm::CallInst *call = llvm::CallInst::Create(
      kernel, args, "", entry);
  call->setCallingConv(kernel->getCallingConv());
  llvm::ReturnInst::Create(LLVM_CONTEXT, entry);

  // Kernel metadata
  llvm::Value *mdVals[] = {
    harness, llvm::MDString::get(LLVM_CONTEXT, "kernel"), llvmInt(1)
  };
  llvm::MDNode *kernelMD = llvm::MDNode::get(LLVM_CONTEXT, mdVals);
  llvm::NamedMDNode *nvvmAnnot = module
      ->getOrInsertNamedMetadata("nvvm.annotations");
  nvvmAnnot->addOperand(kernelMD);

  return harness;
}

int GPUFunction::findShardSize(ir::IndexSet domain) {
  if (domain.getKind() == ir::IndexSet::Range) {
    return domain.getSize();
  }
  else if (domain.getKind() == ir::IndexSet::Set) {
    return actuals[ir::to<struct ir::VarExpr>(domain.getSet())->var.getName()]
        .getSet()->getSize();
  }
  else {
    ierror << "Invalid domain kind: " << domain.getKind();
  }
  assert(false && "unreachable");
  return -1;
}

simit::Function::FuncType GPUFunction::init(
    const std::vector<std::string> &formals,
    std::map<std::string, Actual> &actuals) {
  CUlinkState linker;
  CUmodule cudaModule;
  CUfunction cudaFunction;

  // Push data and build harness
  llvm::SmallVector<llvm::Value*, 8> args;
  for (const std::string& formal : formals) {
    assert(actuals.find(formal) != actuals.end());
    Actual &actual = actuals.at(formal);
    if (formal == "$shared") {
      SetBase sharedSet;
      std::cout << "Building shared..." << std::endl;
      sharedSet.buildSetFields(actual.getType().toSet()
                               ->elementType.toElement());
      sharedSet.add();
      std::cout << "Built shared." << std::endl;
      actual.bind(&sharedSet);
      args.push_back(pushArg(actual, false));
    }
    else {
      args.push_back(pushArg(actual, true));
    }
  }
  // Create harnesses for kernel args
  llvm::Function *harness = createHarness(args, llvmFunc.get(), module.get());

  // Export IR to string
  std::string moduleStr;
  llvm::raw_string_ostream str(moduleStr);
  str << *module;
  std::cout << "Module:" << std::endl
            << moduleStr << std::endl;

  // Generate harness PTX
  std::cout << "Create PTX" << std::endl;
  std::string ptxStr = generatePtx(
      moduleStr, cuDevMajor, cuDevMinor,
      module->getModuleIdentifier().c_str());
  std::cout << "PTX:" << std::endl
            << ptxStr << std::endl;

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

  std::cout << "Linker log:" << std::endl
            << linkerInfo << std::endl
            << linkerErrors << std::endl
            << "End linker log." << std::endl;

  // Create CUDA module for binary object
  checkCudaErrors(cuModuleLoadDataEx(&cudaModule, cubin, 0, 0, 0));
  checkCudaErrors(cuLinkDestroy(linker));

  // Alloc global buffers and set global pointers
  for (auto& buf : globalBufs) {
    const ir::Var &bufVar = buf.first;
    llvm::Value *bufVal = buf.second;

    CUdeviceptr globalPtr;
    size_t globalPtrSize;
    checkCudaErrors(cuModuleGetGlobal(&globalPtr, &globalPtrSize,
                                      cudaModule, bufVar.getName().c_str()));
    std::cout << "Pointer size: " << globalPtrSize << std::endl;
    iassert(globalPtrSize == sizeof(void*))
        << "Global pointers should all be pointer-sized. Got: "
        << globalPtrSize << " bytes";

    // Checks
    unsigned int attrVal;
    checkCudaErrors(cuPointerGetAttribute(&attrVal, CU_POINTER_ATTRIBUTE_IS_MANAGED, globalPtr));
    if (!attrVal) continue;
    iassert(attrVal == 1);
    checkCudaErrors(cuPointerGetAttribute(&attrVal, CU_POINTER_ATTRIBUTE_MEMORY_TYPE, globalPtr));
    iassert(attrVal == CU_MEMORYTYPE_DEVICE);

    size_t bufSize = size(*bufVar.getType().toTensor(), storage);
    CUdeviceptr devBuffer;
    checkCudaErrors(cuMemAlloc(&devBuffer, bufSize));
    void *devBufferPtr = reinterpret_cast<void*>(devBuffer);

    std::cout << std::hex << devBuffer << " alloc'd for " << std::dec << bufSize << std::endl;
    std::cout << "Copying: " << sizeof(void*) << std::endl;
    void *globalPtrHost;
    checkCudaErrors(cuPointerGetAttribute(&globalPtrHost, CU_POINTER_ATTRIBUTE_HOST_POINTER, globalPtr));
    *((void**)globalPtrHost) = devBufferPtr;
  }

  // Get reference to CUDA function
  checkCudaErrors(cuModuleGetFunction(
      &cudaFunction, cudaModule, harness->getName().data()));

  return [this, cudaFunction, cudaModule](){
    void **kernelParamsArr = new void*[0]; // TODO leaks
    checkCudaErrors(cuLaunchKernel(cudaFunction,
                                   1, 1, 1, // grid size
                                   1, 1, 1, // block size
                                   0, NULL,
                                   kernelParamsArr, NULL));
  };
}

}
}
