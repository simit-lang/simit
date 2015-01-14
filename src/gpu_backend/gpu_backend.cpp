#include "gpu_backend.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

#include "error.h"
#include "gpu_codegen.h"
#include "gpu_function.h"
#include "ir.h"
#include "ir_queries.h"
#include "llvm_codegen.h"
#include "types.h"

#ifndef NASSERT
#define ASSERT(cond) \
  do { \
    if (!(cond)) { \
      std::cerr << "Assert error: " << __FILE__ << ":" << __LINE__ << std::endl; \
      exit(1); \
    } \
  } while (0)
#else
#define ASSERT(cond) do { (void)sizeof(cond); } while (0)
#endif

namespace simit {
namespace internal {

GPUBackend::GPUBackend() {
  // TODO: move into GPUFunction::init or similar?
  // CUDA runtime
  CUdevice device;
  CUcontext context;
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

  // Create driver context
  checkCudaErrors(cuCtxCreate(&context, 0, device));

  int attrVal;
  checkCudaErrors(cuDeviceGetAttribute(&attrVal, CU_DEVICE_ATTRIBUTE_UNIFIED_ADDRESSING, device));
  iassert(attrVal == 1);
}

GPUBackend::~GPUBackend() {}

simit::Function *GPUBackend::compile(simit::ir::Func irFunc) {
  std::cout << "GPUBackend compile: " << irFunc.getName() << std::endl;
  this->irFunc = irFunc;
  this->module = createNVVMModule("kernels-module");
  this->dataLayout.reset(new llvm::DataLayout(module));
  this->storage = irFunc.getStorage();

  std::vector<ir::Func> callTree = ir::getCallTree(irFunc);
  std::reverse(callTree.begin(), callTree.end());

  this->storage = ir::Storage();
  symtable.clear();
  buffers.clear();
  inKernel = false;

  // TODO(gkanwar): Why do we sometimes get duplicates of functions being
  // generated? How do we properly handle this without duplicates?
  for (auto &f : callTree) {
    std::cout << "calltree, f: " << f.getName() << std::endl;
    if (f.getKind() != ir::Func::Internal) continue;
    iassert(f.getBody().defined());

    this->storage.add(f.getStorage());

    // Emit function
    func = emitEmptyFunction(f.getName(), f.getArguments(), f.getResults(),
                      false, false);

    // Add constants to symbol table
    for (auto &global : f.getEnvironment().globals) {
      symtable.insert(global.first, compile(global.second));
    }

    f.getBody().accept(this);
    builder->CreateRetVoid();
    symtable.clear();
    std::cout << "Calltree done with f" << std::endl;
  }
  iassert(func);

  return new GPUFunction(irFunc, func, module, buffers, fieldStorage,
                         cuDevMajor, cuDevMinor);
}

llvm::Value *GPUBackend::compile(const ir::Expr &expr) {
  expr.accept(this);
  llvm::Value *tmp = val;
  val = nullptr;
  return tmp;
}

void GPUBackend::visit(const ir::FieldRead *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::TensorRead *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::TupleRead *op) {
  std::cerr << "GPUBackend::visit unsupported node:\n\n" << *op << "\n";
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::IndexRead *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Length *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Map *op) {
  std::cerr << "GPUBackend::visit unsupported node:\n\n" << *op << "\n";
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::IndexedTensor *op) {
  std::cerr << "GPUBackend::visit unsupported node:\n\n" << *op << "\n";
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::IndexExpr *op) {
  std::cerr << "GPUBackend::visit unsupported node:\n\n" << *op << "\n";
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::TensorWrite *op) {
  LLVMBackend::visit(op);
}

void GPUBackend::visit(const ir::Literal *op) {
  const ir::TensorType *type = op->type.toTensor();
  if (type->order() == 0) {
    // Delegate scalar literals to generic LLVM backend
    LLVMBackend::visit(op);
  }
  else {
    // Put the data in global memory and generate a pointer
    ir::ScalarType ctype = type->componentType;
    llvm::Constant *dataConstant;
    switch (ctype.kind) {
      case ir::ScalarType::Int: {
        iassert(ctype.bytes() == sizeof(uint32_t))
            << "Incorrect native types used for constant data array";
        iassert(op->size % sizeof(uint32_t) == 0)
            << "Literal data size not a multiple of element size";
        dataConstant = llvm::ConstantDataArray::get(
            LLVM_CONTEXT, llvm::ArrayRef<uint32_t>(
                reinterpret_cast<uint32_t*>(op->data),
                op->size/sizeof(uint32_t)));
        break;
      }
      case ir::ScalarType::Float: {
        if (ir::ScalarType::floatBytes == sizeof(float)) {
          iassert(op->size % sizeof(float) == 0)
              << "Literal data size not a multiple of element size";
          dataConstant = llvm::ConstantDataArray::get(
              LLVM_CONTEXT, llvm::ArrayRef<float>(
                  reinterpret_cast<float*>(op->data),
                  op->size/sizeof(float)));
        }
        else if (ir::ScalarType::floatBytes == sizeof(double)) {
          iassert(op->size % sizeof(double) == 0)
              << "Literal data size not a multiple of element size";
          dataConstant = llvm::ConstantDataArray::get(
              LLVM_CONTEXT, llvm::ArrayRef<double>(
                  reinterpret_cast<double*>(op->data),
                  op->size/sizeof(double)));
        }
        else {
          unreachable;
        }
        break;
      }
      case ir::ScalarType::Boolean: {
        not_supported_yet;
        // This code is untested, but likely correct
        iassert(op->size % sizeof(bool) == 0)
            << "Literal data size not a multiple of element size";
        iassert(sizeof(bool) == sizeof(uint32_t))
            << "Boolean literal assumes 32-bit data format";
        dataConstant = llvm::ConstantDataArray::get(
            LLVM_CONTEXT, llvm::ArrayRef<uint32_t>(
                reinterpret_cast<uint32_t*>(op->data),
                op->size/sizeof(uint32_t)));
        break;
      }
      default: unreachable;
    }
    llvm::GlobalVariable *globalData =
        new llvm::GlobalVariable(*module, dataConstant->getType(), true,
                                 llvm::GlobalVariable::InternalLinkage,
                                 dataConstant, "global_const", nullptr,
                                 llvm::GlobalVariable::NotThreadLocal,
                                 CUDA_GLOBAL_ADDRSPACE);
    llvm::Type *finalType = createLLVMType(type);
    val = builder->CreateBitCast(globalData, finalType);
  }
  iassert(val);
}
void GPUBackend::visit(const ir::VarExpr *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Load *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::CallStmt *op) {
  std::map<ir::Func, std::string> nvvmIntrinsicByName =
                                  {{ir::Intrinsics::sin,    std::string("__nv_sinf")},
                                   {ir::Intrinsics::cos,    std::string("__nv_cosf")},
                                   {ir::Intrinsics::sqrt,   std::string("__nv_sqrtf")},
                                   {ir::Intrinsics::log,    std::string("__nv_logf")},
                                   {ir::Intrinsics::exp,    std::string("__nv_fast_expf")},
                                   {ir::Intrinsics::pow,    std::string("__nv_fast_powf")},
                                   {ir::Intrinsics::atan2,  std::string("__nv_atan2f")},
                                   {ir::Intrinsics::tan,    std::string("__nv_tanf")},
                                   {ir::Intrinsics::asin,   std::string("__nv_asinf")},
                                   {ir::Intrinsics::acos,   std::string("__nv_acosf")}};
  
  std::vector<llvm::Type*> argTypes;
  std::vector<llvm::Value*> args;
  llvm::Function *fun = nullptr;
  llvm::Value *call = nullptr;

  // compile arguments first
  for (auto a: op->actuals) {
    //FIX: remove once solve() is no longer needed
    //iassert(isScalar(a.type()));
    argTypes.push_back(createLLVMType(a.type().toTensor()->componentType));
    args.push_back(compile(a));
  }

  ir::Func callee = op->callee;

  if (callee.getKind() == ir::Func::Intrinsic) {
    std::string floatTypeName = ir::ScalarType::singleFloat() ? "_f32" : "_f64";

    // first, see if this is an LLVM intrinsic
    auto foundIntrinsic = nvvmIntrinsicByName.find(callee);
    if (foundIntrinsic != nvvmIntrinsicByName.end()) {
      auto ftype = llvm::FunctionType::get(getLLVMFloatType(), argTypes, false);
      fun = llvm::cast<llvm::Function>(module->getOrInsertFunction(
          foundIntrinsic->second, ftype));
      call = builder->CreateCall(fun, args);
    }
    else if (callee == ir::Intrinsics::det) {
      iassert(args.size() == 1);
      std::string fname = callee.getName() + "3" + floatTypeName;
      call = emitCall(fname, args, getLLVMFloatType());
    }
    else if (callee == ir::Intrinsics::norm) {
      iassert(args.size() == 1);
      auto type = op->actuals[0].type().toTensor();
      args.push_back(emitComputeLen(type->dimensions[0]));
      std::string funcName = callee.getName() + floatTypeName;
      call = emitCall(funcName, args, getLLVMFloatType());
    }
    else if (callee == ir::Intrinsics::inv) {
      iassert(args.size() == 1);

      ir::Var result = op->results[0];
      llvm::Value *llvmResult = symtable.get(result);
      args.push_back(llvmResult);

      std::string fname = callee.getName() + "3" + floatTypeName;
      call = emitCall(fname, args);
    }
    else if (op->callee == ir::Intrinsics::loc) {
      call = emitCall("loc", args, LLVM_INT);
    }
    else if (callee == ir::Intrinsics::dot) {
      // we need to add the vector length to the args
      auto type1 = op->actuals[0].type().toTensor();
      auto type2 = op->actuals[1].type().toTensor();
      uassert(type1->dimensions[0] == type2->dimensions[0]) <<
          "dimension mismatch in dot product";
      args.push_back(emitComputeLen(type1->dimensions[0]));
      std::string funcName = callee.getName() + floatTypeName;
      call = emitCall(funcName, args, getLLVMFloatType());
    }
    else {
      ierror << "intrinsic " << op->callee.getName() << " not found";
    }
  
    iassert(call);
    if (!call->getType()->isVoidTy()) {
      iassert(op->results.size() == 1);
      ir::Var var = op->results[0];
      llvm::Value *llvmVar = symtable.get(var);
      builder->CreateStore(call, llvmVar);
    }
  }
  // if not an intrinsic function, try to find it in the module
  else {
    if (module->getFunction(callee.getName())) {
      for (ir::Var r : op->results) {
        argTypes.push_back(createLLVMType(r.getType().toTensor()->componentType));

        llvm::Value *llvmResult = symtable.get(r);
        args.push_back(llvmResult);
        symtable.insert(r, llvmResult);
      }
      fun = module->getFunction(op->callee.getName());
      call = builder->CreateCall(fun, args);
    }
    else {
      ierror << "function " << op->callee.getName() << " not found in module";
    }
  }
}
void GPUBackend::visit(const ir::Call *op) {
  std::map<ir::Func, std::string> nvvmIntrinsicByName =
                                  {{ir::Intrinsics::sin,    std::string("__nv_sinf")},
                                   {ir::Intrinsics::cos,    std::string("__nv_cosf")},
                                   {ir::Intrinsics::sqrt,   std::string("__nv_sqrtf")},
                                   {ir::Intrinsics::log,    std::string("__nv_logf")},
                                   {ir::Intrinsics::exp,    std::string("__nv_fast_expf")},
                                   {ir::Intrinsics::pow,    std::string("__nv_fast_powf")},
                                   {ir::Intrinsics::atan2,  std::string("__nv_atan2f")},
                                   {ir::Intrinsics::tan,    std::string("__nv_tanf")},
                                   {ir::Intrinsics::asin,   std::string("__nv_asinf")},
                                   {ir::Intrinsics::acos,   std::string("__nv_acosf")}};
  
  std::vector<llvm::Type*> argTypes;
  std::vector<llvm::Value*> args;
  llvm::Function *fun = nullptr;

  // compile arguments first
  for (auto a: op->actuals) {
    //FIX: remove once solve() is no longer needed
    //iassert(isScalar(a.type()));
    argTypes.push_back(createLLVMType(a.type().toTensor()->componentType));
    args.push_back(compile(a));
  }

  auto foundIntrinsic = nvvmIntrinsicByName.find(op->func);
  if (foundIntrinsic != nvvmIntrinsicByName.end()) {
    auto ftype = llvm::FunctionType::get(getLLVMFloatType(), argTypes, false);
    fun = llvm::cast<llvm::Function>(module->getOrInsertFunction(
      foundIntrinsic->second, ftype));
  }
  else if (op->func == ir::Intrinsics::norm) {
    iassert(args.size() == 1);
    auto type = op->actuals[0].type().toTensor();

    args.push_back(emitComputeLen(type->dimensions[0]));
    std::string funcName = ir::ScalarType::singleFloat() ?
        "norm_f32" : "norm_f64";
    val = emitCall(funcName, args, getLLVMFloatType());
    return;
  }
  else if (op->func == ir::Intrinsics::loc) {
    val = emitCall("loc", args, LLVM_INT);
    return;
  }
  else if (op->func == ir::Intrinsics::dot) {
    iassert(args.size() == 2);
    // we need to add the vector length to the args
    auto type1 = op->actuals[0].type().toTensor();
    auto type2 = op->actuals[1].type().toTensor();
    uassert(type1->dimensions[0] == type2->dimensions[0]) <<
      "dimension mismatch in dot product";
    args.push_back(emitComputeLen(type1->dimensions[0]));
    std::string funcName = ir::ScalarType::singleFloat() ?
        "dot_f32" : "dot_f64";
    val = emitCall(funcName, args, getLLVMFloatType());
    return;
  }
  // if not an intrinsic function, try to find it in the module
  else if (module->getFunction(op->func.getName())) {
    fun = module->getFunction(op->func.getName());
  }
  else {
    std::cerr << "GPUBackend::visit unsupported node:\n\n" << *op << "\n\n";
    ASSERT(false && "No code generation for this type");
  }
  
  val = builder->CreateCall(fun, args);
}
void GPUBackend::visit(const ir::Neg *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Add *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Sub *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Mul *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Div *op) {
  LLVMBackend::visit(op);
}

void GPUBackend::visit(const ir::VarDecl *op) {
  tassert(op->var.getType().isTensor()) << "Only tensor decls supported";

  if (inKernel) {
    // Allow LLVMBackend to emit a local alloca
    LLVMBackend::visit(op);
  }
  else { // Root scope
    // Always global, to be accessible to all kernels
    makeGlobalTensor(op->var);
  }
}

void GPUBackend::visit(const ir::AssignStmt *op) {
  // Only atomic for a compound scalar-scalar assign
  if (op->cop.kind != ir::CompoundOperator::None &&
      op->var.getType().toTensor()->order() == 0) {
    iassert(symtable.contains(op->var)) << op->var << " has not been declared";
    switch (op->cop.kind) {
      case ir::CompoundOperator::Add: {
        llvm::Value *value = compile(op->value);
        llvm::Value *varPtr = symtable.get(op->var);
        // Guard against non-pointer
        iassert(varPtr->getType()->isPointerTy());
        // Guard against invalid addrspace
        unsigned addrspace = ((llvm::PointerType*)varPtr->getType())
            ->getAddressSpace();
        if (addrspace != CUDA_GLOBAL_ADDRSPACE &&
            addrspace != CUDA_SHARED_ADDRSPACE) {
          LLVMBackend::visit(op);
          return;
        }
        emitAtomicLoadAdd(varPtr, value);
        break;
      }
      default: ierror << "Unknown compound operator type: " << op->cop.kind;
    }
  }
  else {
    LLVMBackend::visit(op);
  }
}
void GPUBackend::visit(const ir::FieldWrite *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Store *op) {
  if (op->cop.kind != ir::CompoundOperator::None) {
    llvm::Value *buffer = compile(op->buffer);
    llvm::Value *index = compile(op->index);
    llvm::Value *value = compile(op->value);
    std::string locName = std::string(buffer->getName()) + PTR_SUFFIX;
    llvm::Value *bufferLoc = builder->CreateInBoundsGEP(buffer, index, locName);
    switch (op->cop.kind) {
      case ir::CompoundOperator::Add: {
        emitAtomicLoadAdd(bufferLoc, value);
        break;
      }
      default: ierror << "Unknown compound operator type";
    }
  }
  else {
    LLVMBackend::visit(op);
  }
}
void GPUBackend::visit(const ir::ForRange *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::For *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::GPUKernel *op) {
  // Stash the symtable
  ScopedMap<simit::ir::Var, llvm::Value*> oldSymtable = symtable;
  symtable.clear();

  // Create LLVM func
  llvm::Function *kernel = emitEmptyFunction(
      irFunc.getName() + "_nested_kernel", irFunc.getArguments(),
      irFunc.getResults(), true, false);
  builder.reset(new LLVMIRBuilder(&kernel->getEntryBlock()));

  // Kernel metadata
  addNVVMAnnotation(kernel, "kernel", llvmInt(1), module);

  // Add global buffers to symtable
  for (auto &buf : buffers) {
    const ir::Var &var = buf.first;
    llvm::Value *val = buf.second;

    llvm::Value *llvmTmp = builder->CreateLoad(val, val->getName());
    symtable.insert(var, llvmTmp);
  }

  GPUSharding kernelSharding = op->sharding;

  // Code generate for the kernel
  if (kernelSharding.xSharded) {
    symtable.insert(kernelSharding.xVar, getTidX());
  }
  if (kernelSharding.ySharded) {
    symtable.insert(kernelSharding.yVar, getTidY());
  }
  if (kernelSharding.zSharded) {
    symtable.insert(kernelSharding.zVar, getTidZ());
  }
  
  inKernel = true;
  LLVMBackend::compile(op->body);
  inKernel = false;

  // NVVM kernel should always return void
  builder->CreateRetVoid();

  // Unstash symtable
  symtable = oldSymtable;

  // Emit a dynamic kernel launch
  builder.reset(new LLVMIRBuilder(&func->getEntryBlock()));
  std::vector<llvm::Value*> args;
  for (auto &irArg : irFunc.getArguments()) {
    args.push_back(symtable.get(irArg));
  }
  for (auto &irRes : irFunc.getResults()) {
    // TODO(gkanwar): Figure out inouts
    args.push_back(symtable.get(irRes));
  }
  emitKernelLaunch(kernel, args, kernelSharding);
}

void GPUBackend::visit(const ir::IfThenElse *op) {
  // ASSERT(false && "No code generation for this type");
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Block *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Pass *op) {
  // ASSERT(false && "No code generation for this type");
  LLVMBackend::visit(op);
}

namespace {

void cleanFuncAttrs(llvm::Function *func) {
  // Clean attributes off of params
  llvm::AttributeSet funcAttrs = func->getAttributes();
  llvm::AttributeSet cleanAttrs;
  for (unsigned slot = 0; slot < funcAttrs.getNumSlots(); ++slot) {
    // Never add func attributes, because attribute groups are
    // disallowed in NVVM. If left on, they trip up the parser
    if (slot == 0) continue;
    // Remove readonly from param attrs
    int index = funcAttrs.getSlotIndex(slot);
    llvm::AttributeSet cleanSlot = funcAttrs.removeAttribute(
        LLVM_CONTEXT, index, llvm::Attribute::ReadOnly);
    cleanAttrs.addAttributes(LLVM_CONTEXT, index, cleanSlot);
  }

  func->setAttributes(cleanAttrs);
}

}

llvm::Value *GPUBackend::emitBarrier() {
  llvm::FunctionType *funcTy = llvm::FunctionType::get(LLVM_VOID, false);
  llvm::Function *func = llvm::cast<llvm::Function>(
      module->getOrInsertFunction("llvm.nvvm.barrier0", funcTy));
  cleanFuncAttrs(func);
  return builder->CreateCall(func);
}

llvm::Value *GPUBackend::emitCheckRoot() {
  not_supported_yet;
  assert(false && "unreachable");
  return NULL;
}

llvm::Value *GPUBackend::getTidX() {
  llvm::FunctionType *funcTy = llvm::FunctionType::get(LLVM_INT, false);
  llvm::Function *func = llvm::cast<llvm::Function>(
      module->getOrInsertFunction("llvm.nvvm.read.ptx.sreg.tid.x", funcTy));
  cleanFuncAttrs(func);
  return builder->CreateCall(func);
}

llvm::Value *GPUBackend::getTidY() {
  llvm::FunctionType *funcTy = llvm::FunctionType::get(LLVM_INT, false);
  llvm::Function *func = llvm::cast<llvm::Function>(
      module->getOrInsertFunction("llvm.nvvm.read.ptx.sreg.tid.y", funcTy));
  cleanFuncAttrs(func);
  return builder->CreateCall(func);
}

llvm::Value *GPUBackend::getTidZ() {
  llvm::FunctionType *funcTy = llvm::FunctionType::get(LLVM_INT, false);
  llvm::Function *func = llvm::cast<llvm::Function>(
      module->getOrInsertFunction("llvm.nvvm.read.ptx.sreg.tid.z", funcTy));
  cleanFuncAttrs(func);
  return builder->CreateCall(func);
}

void GPUBackend::emitThreadBarrier() {
  llvm::FunctionType *funcTy = llvm::FunctionType::get(LLVM_VOID, false);
  llvm::Function *func = llvm::cast<llvm::Function>(
      module->getOrInsertFunction("llvm.nvvm.barrier0", funcTy));
  cleanFuncAttrs(func);
  builder->CreateCall(func);
}

void GPUBackend::emitAtomicLoadAdd(llvm::Value *ptr, llvm::Value *value) {
  if (value->getType()->isIntegerTy()) {
    builder->CreateAtomicRMW(llvm::AtomicRMWInst::Add, ptr, value,
                             llvm::AtomicOrdering::Monotonic);
  }
  else if (value->getType()->isFloatTy()) {
    emitAtomicFLoadAdd(ptr, value);
  }
  else {
    ierror << "Unknown LLVM value type for atomic load add";
  }
}

void GPUBackend::emitAtomicFLoadAdd(llvm::Value *ptr, llvm::Value *value) {
  llvm::Type *ptrGenTy = ptr->getType();
  iassert(ptrGenTy->isPointerTy())
      << "Atomic float load add requires pointer type for ptr";
  llvm::PointerType *ptrTy = reinterpret_cast<llvm::PointerType*>(ptrGenTy);
  unsigned addrspace = ptrTy->getAddressSpace();
  std::vector<llvm::Type*> argTys;
  std::string funcName;
  switch (addrspace) {
    case CUDA_GENERIC_ADDRSPACE: {
      argTys.push_back(LLVM_FLOATPTR);
      argTys.push_back(LLVM_FLOAT);
      funcName = "llvm.nvvm.atomic.load.add.f32.p0f32";
      break;
    }
    case CUDA_GLOBAL_ADDRSPACE: {
      argTys.push_back(LLVM_FLOATPTR_GLOBAL);
      argTys.push_back(LLVM_FLOAT);
      funcName = "llvm.nvvm.atomic.load.add.f32.p1f32";
      break;
    }
    case CUDA_SHARED_ADDRSPACE: {
      argTys.push_back(llvm::Type::getFloatPtrTy(LLVM_CONTEXT, addrspace));
      argTys.push_back(LLVM_FLOAT);
      funcName = "llvm.nvvm.atomic.load.add.f32.p3f32";
      break;
    }
    default:
      ierror << "Unsupported addrspace for float load/add: " << addrspace;
  }
  llvm::FunctionType *funcTy = llvm::FunctionType::get(
      LLVM_FLOAT, argTys, false);
  llvm::Function *func = llvm::cast<llvm::Function>(module->getOrInsertFunction(
      funcName, funcTy));
  cleanFuncAttrs(func);
  builder->CreateCall2(func, ptr, value);
}

void GPUBackend::emitTid0Code(const ir::Stmt& body) {
  // Condition is tid_x == 0 && tid_y == 0 && tid_z == 0
  ierror << "Should not be used";
  // llvm::Value *cond = builder->CreateAnd(
  //     builder->CreateICmpEQ(getTidX(), llvmInt(0)),
  //     builder->CreateAnd(
  //         builder->CreateICmpEQ(getTidY(), llvmInt(0)),
  //         builder->CreateICmpEQ(getTidZ(), llvmInt(0))));
  // llvm::BasicBlock *then = llvm::BasicBlock::Create(LLVM_CONTEXT, "then", func);
  // llvm::BasicBlock *rest = llvm::BasicBlock::Create(LLVM_CONTEXT, "rest", func);
  // builder->CreateCondBr(cond, then, rest);
  // builder.reset(new LLVMIRBuilder(then));
  // LLVMBackend::compile(body);
  // builder->CreateBr(rest);
  // builder.reset(new LLVMIRBuilder(rest));
}

void GPUBackend::emitKernelLaunch(llvm::Function *kernel,
                                  std::vector<llvm::Value*> args,
                                  GPUSharding sharding) {
  std::cout << "emitKernelLaunch: " << std::endl;
  for (llvm::Value *val : args) {
    std::cout << "arg: " << val->getName().str() << std::endl;
  }

  // LLVM types
  // struct dim3
  std::vector<llvm::Type*> dim3Types = { LLVM_INT, LLVM_INT, LLVM_INT };
  llvm::StructType *dim3Ty = llvm::StructType::create(
      llvm::ArrayRef<llvm::Type*>(dim3Types), "dim3");

  // cudaGetParamBufferV2
  std::vector<llvm::Type*> getParamArgTys = {
    LLVM_INT8PTR, dim3Ty, dim3Ty, LLVM_INT
  };
  llvm::FunctionType *getParamFuncTy = llvm::FunctionType::get(
      LLVM_INT8PTR, getParamArgTys, false);

  // CUstream_st
  llvm::StructType *cuStreamTy = llvm::StructType::create(
      LLVM_CONTEXT, "struct.CUstream_st");
  // addrspace 0
  llvm::PointerType *cuStreamPtrTy = llvm::PointerType::get(cuStreamTy, 0);

  // cudaLaunchDeviceV2
  std::vector<llvm::Type*> launchDevArgTys = {
    LLVM_INT8PTR, cuStreamPtrTy
  };
  llvm::FunctionType *launchDevFuncTy = llvm::FunctionType::get(
      LLVM_INT, launchDevArgTys, false);

  // Build dimensions
  std::vector<llvm::Constant*> gridDimsVec = {
    llvmInt(1), llvmInt(1), llvmInt(1)
  };
  llvm::Constant *gridDims =
      llvm::ConstantStruct::get(dim3Ty, gridDimsVec);

  // TODO(gkanwar): Change back
  std::vector<llvm::Constant*> initBlockDims = {
    llvmInt(1), llvmInt(1), llvmInt(1)
  };
  llvm::Value *blockDims =
      llvm::ConstantStruct::get(dim3Ty, initBlockDims);
  blockDims = builder->CreateInsertValue(
      blockDims,
      sharding.xSharded ? emitComputeLen(sharding.xDomain) : llvmInt(1),
      llvm::ArrayRef<unsigned>({0}));
  blockDims = builder->CreateInsertValue(
      blockDims,
      sharding.ySharded ? emitComputeLen(sharding.yDomain) : llvmInt(1),
      llvm::ArrayRef<unsigned>({1}));
  blockDims = builder->CreateInsertValue(
      blockDims,
      sharding.zSharded ? emitComputeLen(sharding.zDomain) : llvmInt(1),
      llvm::ArrayRef<unsigned>({2}));

  // Build param buffer
  llvm::Function *getParamFunc = llvm::cast<llvm::Function>(
      module->getOrInsertFunction("cudaGetParameterBufferV2", getParamFuncTy));
  llvm::Value *kernelBitCast = builder->CreateBitCast(kernel, LLVM_INT8PTR);
  llvm::Value *paramBuf = builder->CreateCall4(
      getParamFunc, kernelBitCast, gridDims, blockDims, llvmInt(0));

  // Insert args into param buffer
  emitFillBuf(paramBuf, args);

  llvm::Function *cudaLaunchFunc = llvm::cast<llvm::Function>(
      module->getOrInsertFunction("cudaLaunchDeviceV2", launchDevFuncTy));
  builder->CreateCall2(cudaLaunchFunc, paramBuf,
                       llvm::ConstantPointerNull::get(cuStreamPtrTy));

  // Synchronize memory after the call
  std::vector<llvm::Type*> argTys;
  llvm::FunctionType *syncFuncTy = llvm::FunctionType::get(
      LLVM_INT, argTys, false);
  llvm::Function *syncFunc = llvm::cast<llvm::Function>(
      module->getOrInsertFunction("cudaDeviceSynchronize", syncFuncTy));
  builder->CreateCall(syncFunc);
}

void GPUBackend::emitPrintf(std::string format,
                            std::vector<llvm::Value*> args) {
  llvm::Value *formatPtr = emitGlobalString(format);

  // Convert any args that need to be extended
  for (size_t i = 0; i < args.size(); ++i) {
    auto &arg = args[i];
    if (arg->getType()->isFloatTy()) {
      args[i] = builder->CreateFPExt(arg, LLVM_DOUBLE);
    }
    else if (arg->getType()->isIntegerTy()) {
      unsigned width = arg->getType()->getIntegerBitWidth();
      if (width < 32) {
        args[i] = builder->CreateSExt(arg, LLVM_INT);
      }
    }
  }

  // Alloc args buf
  size_t size = 0;
  for (auto &arg : args) {
    size += dataLayout->getTypeAllocSize(arg->getType());
  }

  llvm::Value *argBuf = builder->CreateAlloca(LLVM_INT8, llvmInt(size), "buffer");
  emitFillBuf(argBuf, args);

  // Create and call vprintf syscall
  std::vector<llvm::Type*> argTys = {LLVM_INT8PTR, LLVM_INT8PTR};
  llvm::FunctionType *vprintfTy = llvm::FunctionType::get(
      LLVM_INT, argTys, false);
  llvm::Function *vprintf = llvm::cast<llvm::Function>(
      module->getOrInsertFunction("vprintf", vprintfTy));

  builder->CreateCall2(vprintf, formatPtr, argBuf);
}

void GPUBackend::emitMemCpy(llvm::Value *dst, llvm::Value *src,
                            llvm::Value *size, unsigned align) {
  iassert(dst->getType()->isPointerTy());
  iassert(src->getType()->isPointerTy());

  // Emit our own memcpy decl, since the built-in has attributes which
  // are not handled by NVVM
  std::vector<llvm::Type*> argTys = {
    LLVM_INT8PTR_GLOBAL, LLVM_INT8PTR_GLOBAL, LLVM_INT, LLVM_INT, LLVM_BOOL
  };
  llvm::FunctionType *funcTy = llvm::FunctionType::get(
      LLVM_VOID, argTys, false);
  llvm::Function *func = llvm::cast<llvm::Function>(
      module->getOrInsertFunction("llvm.memcpy.p1i8.p1i8.i32", funcTy));
  cleanFuncAttrs(func);

  llvm::Value *llvmAlign = llvmInt(align);
  llvm::Value *castDst = builder->CreateBitCast(dst, LLVM_INT8PTR_GLOBAL);
  llvm::Value *castSrc = builder->CreateBitCast(src, LLVM_INT8PTR_GLOBAL);
  llvm::Constant *isVolatile = llvmBool(true);
  builder->CreateCall5(func, castDst, castSrc, size, llvmAlign, isVolatile);
}

void GPUBackend::emitMemSet(llvm::Value *dst, llvm::Value *val,
                            llvm::Value *size, unsigned align) {
  iassert(dst->getType()->isPointerTy());

  // Emit our own memset decl, since the built-in has attributes which
  // are not handled by NVVM
  std::vector<llvm::Type*> argTys = {
    LLVM_INT8PTR_GLOBAL, LLVM_INT8, LLVM_INT, LLVM_INT, LLVM_BOOL
  };
  llvm::FunctionType *funcTy = llvm::FunctionType::get(
      LLVM_VOID, argTys, false);
  llvm::Function *func = llvm::cast<llvm::Function>(
      module->getOrInsertFunction("llvm.memset.p1i8.i32", funcTy));
  cleanFuncAttrs(func);

  llvm::Value *llvmAlign = llvmInt(align);
  llvm::Value *castDst = builder->CreateBitCast(dst, LLVM_INT8PTR_GLOBAL);
  llvm::Constant *isVolatile = llvmBool(true);
  builder->CreateCall5(func, castDst, val, size, llvmAlign, isVolatile);
}

void GPUBackend::emitFillBuf(llvm::Value *buffer,
                             std::vector<llvm::Value*> vals) {
  uint64_t bufIndex = 0;
  for (auto &val : vals) {
    llvm::Value *bufPtr = builder->CreateGEP(buffer, llvmInt(bufIndex));
    llvm::Value *valPtr = builder->CreateBitCast(
        bufPtr,
        // Pointer to arg type, addrspace 0
        llvm::PointerType::get(val->getType(), 0));
    // Aligned store required to ensure that each parameter remains 8-byte aligned
    builder->CreateAlignedStore(val, valPtr, 8);
    bufIndex += dataLayout->getTypeAllocSize(val->getType());
    if (bufIndex % 8 != 0) {
      iassert(bufIndex % 8 == 4)
          << "Cannot accept non 4-byte aligned params";
      bufIndex += 4;
    }
  }
}

void GPUBackend::makeGlobalTensor(ir::Var var) {
  LLVMBackend::makeGlobalTensor(var);

  // Annotate the global as managed memory to allow us to write its
  // value from the CUDA setup
  llvm::Value *global = buffers[var];
  addNVVMAnnotation(global, "managed", llvmInt(1), module);
}

}
}
