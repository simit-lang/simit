#include "gpu_backend.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/Support/raw_ostream.h"

#include "error.h"
#include "gpu_codegen.h"
#include "gpu_function.h"
#include "ir.h"
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
  // TODO(gkanwar): Set storage?

  irFunc.getBody().accept(this);

  // Compile LLVM kernels to PTX string
  std::string moduleStr;
  llvm::raw_string_ostream raw_str(moduleStr);
  raw_str << *module;

  std::string ptxStr = generatePtx(moduleStr, cuDevMajor, cuDevMinor,
                                   module->getModuleIdentifier().c_str());

  return new GPUFunction(irFunc, module, kernels, sharding,
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
  
  auto foundIntrinsic = nvvmIntrinsicByName.find(op->func);
  if (foundIntrinsic != nvvmIntrinsicByName.end()) {
    std::vector<llvm::Type*> argTypes;
    std::vector<llvm::Value*> args;
    llvm::Function *fun = nullptr;
    // compile arguments first
    for (auto a : op->actuals) {
      argTypes.push_back(createLLVMType(a.type().toTensor()->componentType));
      args.push_back(compile(a));
    }
  
    auto ftype = llvm::FunctionType::get(LLVM_DOUBLE, argTypes, false);
    fun = llvm::cast<llvm::Function>(module->getOrInsertFunction(
      foundIntrinsic->second, ftype));
    
    val = builder->CreateCall(fun, args);
    return;
  }
  
  std::cerr << "GPUBackend::visit unsupported node:\n\n" << *op << "\n\n";
  ASSERT(false && "No code generation for this type");
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
      }
      default: ierror << "Unknown compound operator type";
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
  llvm::Function *func = createPrototype("kernel", irFunc.getArguments(),
                                         irFunc.getResults(), module,
                                         false, false);

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

  // TODO(gkanwar): Deal with temps?

  // Build 'entry' basic block
  llvm::BasicBlock *entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", func);
  builder.reset(new LLVMIRBuilder(entry));

  GPUSharding kernelSharding = op->sharding;
  kernels.emplace_back(func, kernelSharding);

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
  std::vector<llvm::Type*> argTys = {LLVM_FLOATPTR, LLVM_FLOAT};
  llvm::FunctionType *funcTy = llvm::FunctionType::get(
      LLVM_FLOAT, argTys, false);
  llvm::Function *func = llvm::cast<llvm::Function>(module->getOrInsertFunction(
      "llvm.nvvm.atomic.load.add.f32.p0f32", funcTy));
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

}
}
