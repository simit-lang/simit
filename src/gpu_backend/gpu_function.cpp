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

GPUFunction::GPUFunction(
    ir::Func simitFunc, llvm::Module *module,
    std::vector< std::pair<llvm::Function *, GPUSharding> > kernels,
    struct GPUSharding sharding, int cuDevMajor, int cuDevMinor)
    : Function(simitFunc), module(module),
      kernels(kernels), sharding(sharding),
      cuDevMajor(cuDevMajor), cuDevMinor(cuDevMinor) {}

GPUFunction::~GPUFunction() {
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
        return llvmPtr(actual.getType(), reinterpret_cast<void*>(*devBuffer));
      }
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
        pushedBufs.emplace(fieldData, DeviceDataHandle(devBuffer, size, shouldPull));
        setData.push_back(llvmPtr(ftype, reinterpret_cast<void*>(*devBuffer)));
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
  std::cout << "Pull arg: " << *handle.devBuffer << " -> " << hostPtr
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
  std::vector<CUfunction> functions;

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
  // TODO(gkanwar): Is this a memory leak?
  std::vector<llvm::Function *> harnesses;
  for (auto kernel : kernels) {
    // Build harness
    harnesses.push_back(createHarness(args, kernel.first, module.get()));
  }

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
  checkCudaErrors(cuLinkAddData(
      linker, CU_JIT_INPUT_PTX, (void*)ptxStr.c_str(),
      ptxStr.size(), "<kernels-ptx>", 0, NULL, NULL));

  void *cubin;
  size_t cubinSize;
  checkCudaErrors(cuLinkComplete(linker, &cubin, &cubinSize));

  std::cout << "Linker log:" << std::endl
            << linkerInfo << std::endl
            << linkerErrors << std::endl;

  // Create CUDA module for binary object
  checkCudaErrors(cuModuleLoadDataEx(&cudaModule, cubin, 0, 0, 0));
  checkCudaErrors(cuLinkDestroy(linker));
  // Create vector of kernels to be called
  for (auto harness : harnesses) {
    functions.emplace_back();
    checkCudaErrors(cuModuleGetFunction(
        &(functions.back()), cudaModule, harness->getName().data()));
  }

  return [this, functions, cudaModule](){
    iassert(kernels.size() == functions.size())
        << "Number of generated functions must equal number of kernels";
    for (int i = 0; i < kernels.size(); ++i) {
      GPUSharding sharding = kernels[i].second;
      
      std::cerr << "Sharded with: " << sharding;
      
      unsigned blockSizeX = sharding.xSharded ?
          findShardSize(sharding.xDomain) : 1;
      unsigned blockSizeY = sharding.ySharded ?
          findShardSize(sharding.yDomain) : 1;
      unsigned blockSizeZ = sharding.zSharded ?
          findShardSize(sharding.zDomain) : 1;
      unsigned gridSizeX = 1;
      unsigned gridSizeY = 1;
      unsigned gridSizeZ = 1;
      std::cout << "Launching CUDA kernel with block size ("
                << blockSizeX << ","
                << blockSizeY << ","
                << blockSizeZ << ") and grid size ("
                << gridSizeX << ","
                << gridSizeY << ","
                << gridSizeZ << ")" << std::endl;

      void **kernelParamsArr = new void*[0]; // TODO leaks
      checkCudaErrors(cuLaunchKernel(functions[i], gridSizeX, gridSizeY, gridSizeZ,
                                     blockSizeX, blockSizeY, blockSizeZ, 0, NULL,
                                     kernelParamsArr, NULL));
    }

    // Clean up
    // for (auto kv : kernelParams) {
    //   for (auto kv2 : kv.second.devBufferFields) {
    //     checkCudaErrors(cuMemFree(*kv2.second));
    //   }
    // }
    // checkCudaErrors(cuCtxDestroy(context));
    checkCudaErrors(cuModuleUnload(cudaModule));

    // Hack to ensure that data is pushed again, and the harness is
    // reconstructed with the new pointers.
    // TODO(gkanwar): On a bind to a pushed argument, note that the
    // argument should be repushed, and include in this returned lambda
    // instructions to push dirtied args.
    initRequired = true;
  };
}

}
}
