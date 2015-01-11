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
  iassert(cuDevMajor >= 2) << "ERROR: Device 0 is not SM 2.0 or greater";

  // Create driver context
  checkCudaErrors(cuCtxCreate(&context, 0, device));
}

GPUBackend::~GPUBackend() {}

simit::Function *GPUBackend::compile(simit::ir::Func irFunc) {
  this->irFunc = irFunc;
  this->module = createNVVMModule("kernels-module");
  this->dataLayout.reset(new llvm::DataLayout(module));
  this->storage = irFunc.getStorage();

  func = createPrototype(irFunc.getName(), irFunc.getArguments(),
                         irFunc.getResults(), module,
                         true, false);

  // Build 'entry' basic block
  llvm::BasicBlock *entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", func);
  builder.reset(new LLVMIRBuilder(entry));

  // Name LLVM arguments, insert into symtable
  auto arg = func->arg_begin();
  for (const ir::Var &irArg : irFunc.getArguments()) {
    arg->setName(irArg.getName());
    symtable.insert(irArg, &(*arg));
    arg++;
  }
  for (const ir::Var &irRes : irFunc.getResults()) {
    arg->setName(irRes.getName());
    symtable.insert(irRes, &(*arg));
    arg++;
  }

  // Compile the body
  iassert(irFunc.getBody().defined()) << "cannot compile an undefined function";
  irFunc.getBody().accept(this);

  // NVVM kernel should always return void
  builder->CreateRetVoid();

  return new GPUFunction(irFunc, func, module, sharding,
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
  std::cerr << "GPUBackend::visit unsupported node:\n\n" << *op << "\n";
  ASSERT(false && "No code generation for this type");
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
  LLVMBackend::visit(op);
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
      symtable.insert(result, llvmResult);

      std::string fname = callee.getName() + "3" + floatTypeName;
      call = emitCall(fname, args);
    }
    else if (callee == ir::Intrinsics::loc) {
      // TODO(gkanwar)
      not_supported_yet;
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
      symtable.insert(op->results[0], call);
    }
    else {
      ierror << "intrinsic " << op->callee.getName() << " not found";
    }
  
    iassert(call);
    if (!call->getType()->isVoidTy()) {
      symtable.insert(op->results[0], call);
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
    return;
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
void GPUBackend::visit(const ir::AssignStmt *op) {
  // Only atomic for a compound scalar-scalar assign
  if (op->cop.kind != ir::CompoundOperator::None &&
      op->var.getType().toTensor()->order() == 0) {
    switch (op->cop.kind) {
      case ir::CompoundOperator::Add: {
        if (!symtable.contains(op->var)) {
          emitFirstAssign(op->var, op->value);
        }
        llvm::Value *value = compile(op->value);
        llvm::Value *varPtr = symtable.get(op->var);
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
  // Create LLVM func
  llvm::Function *kernel = createPrototype(
      irFunc.getName() + "_nested_kernel", irFunc.getArguments(),
      irFunc.getResults(), module,
      true, false);

  // Kernel metadata
  llvm::Value *mdVals[] = {
    kernel, llvm::MDString::get(LLVM_CONTEXT, "kernel"), llvmInt(1)
  };
  llvm::MDNode *kernelMD = llvm::MDNode::get(LLVM_CONTEXT, mdVals);
  llvm::NamedMDNode *nvvmAnnot = module
      ->getOrInsertNamedMetadata("nvvm.annotations");
  nvvmAnnot->addOperand(kernelMD);

  // Name LLVM arguments, insert into symtable
  auto arg = kernel->arg_begin();
  for (const ir::Var &irArg : irFunc.getArguments()) {
    arg->setName(irArg.getName());
    symtable.insert(irArg, &(*arg));
    arg++;
  }
  for (const ir::Var &irRes : irFunc.getResults()) {
    arg->setName(irRes.getName());
    symtable.insert(irRes, &(*arg));
    arg++;
  }

  // TODO(gkanwar): Deal with temps?

  // Build 'entry' basic block
  llvm::BasicBlock *entry = llvm::BasicBlock::Create(
      LLVM_CONTEXT, "entry", kernel);
  builder.reset(new LLVMIRBuilder(entry));

  GPUSharding kernelSharding = op->sharding;

  // Code generate for the kernel
  symtable.scope();
  if (kernelSharding.xSharded) {
    symtable.insert(kernelSharding.xVar, getTidX());
  }
  if (kernelSharding.ySharded) {
    symtable.insert(kernelSharding.yVar, getTidY());
  }
  if (kernelSharding.zSharded) {
    symtable.insert(kernelSharding.zVar, getTidZ());
  }

  LLVMBackend::compile(op->body);
  symtable.unscope();

  // NVVM kernel should always return void
  builder->CreateRetVoid();

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
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::Block *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Pass *op) {
  ASSERT(false && "No code generation for this type");
}

namespace {

void cleanFuncAttrs(llvm::Function *func) {
  // Attribute groups disallowed in NVVM
  func->removeFnAttr(llvm::Attribute::ReadNone);
  func->removeFnAttr(llvm::Attribute::NoUnwind);
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
    case LLVM_GENERIC_ADDRSPACE: {
      argTys.push_back(LLVM_FLOATPTR);
      argTys.push_back(LLVM_FLOAT);
      funcName = "llvm.nvvm.atomic.load.add.f32.p0f32";
      break;
    }
    case LLVM_GLOBAL_ADDRSPACE: {
      argTys.push_back(LLVM_FLOATPTR_GLOBAL);
      argTys.push_back(LLVM_FLOAT);
      funcName = "llvm.nvvm.atomic.load.add.f32.p1f32";
      break;
    }
    case LLVM_SHARED_ADDRSPACE: {
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
  // i8*
  llvm::Type *i8PtrTy = llvm::PointerType::getInt8PtrTy(LLVM_CONTEXT);

  // struct dim3
  std::vector<llvm::Type*> dim3Types = { LLVM_INT, LLVM_INT, LLVM_INT };
  llvm::StructType *dim3Ty = llvm::StructType::create(
      llvm::ArrayRef<llvm::Type*>(dim3Types), "dim3");

  // cudaGetParamBufferV2
  std::vector<llvm::Type*> getParamArgTys = {
    i8PtrTy, dim3Ty, dim3Ty, LLVM_INT
  };
  llvm::FunctionType *getParamFuncTy = llvm::FunctionType::get(
      i8PtrTy, getParamArgTys, false);

  // CUstream_st
  llvm::StructType *cuStreamTy = llvm::StructType::create(
      LLVM_CONTEXT, "struct.CUstream_st");
  // addrspace 0
  llvm::PointerType *cuStreamPtrTy = llvm::PointerType::get(cuStreamTy, 0);

  // cudaLaunchDeviceV2
  std::vector<llvm::Type*> launchDevArgTys = {
    i8PtrTy, cuStreamPtrTy
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
  llvm::Value *kernelBitCast = builder->CreateBitCast(kernel, i8PtrTy);
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
  for (int i = 0; i < args.size(); ++i) {
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
void GPUBackend::emitFirstAssign(const ir::Var& var,
                                 const ir::Expr& value) {
  // TODO(gkanwar): This doesn't handle sharding later in the code
  if (sharding.isSharded()) {
    not_supported_yet;
  }
  else {
    // TODO(gkanwar): This should actually potentially be up to a two
    // dimensional array to allow correct scoping with nested sharding.
    // Potentially should be done as a second pass.
    LLVMBackend::emitFirstAssign(var, value);
  }
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
    builder->CreateStore(val, valPtr);
    bufIndex += dataLayout->getTypeAllocSize(val->getType());
  }
}

}
}
