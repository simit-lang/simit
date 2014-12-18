#include "gpu_backend.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

#include "error.h"
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

GPUBackend::GPUBackend() {}
GPUBackend::~GPUBackend() {}

simit::Function *GPUBackend::compile(simit::ir::Func irFunc) {
  this->module = new llvm::Module("nvvm-module", LLVM_CONTEXT);

  // Set appropriate data layout
  if (sizeof(void*) == 8)
    module->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
                          "i64:64:64-f32:32:32-f64:64:64-v16:16:16-v32:32:32-"
                          "v64:64:64-v128:128:128-n16:32:64");
  else
    module->setDataLayout("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
                          "i64:64:64-f32:32:32-f64:64:64-v16:16:16-v32:32:32-"
                          "v64:64:64-v128:128:128-n16:32:64");

  llvm::Function *func = createFunction("kernel.main", irFunc.getArguments(),
                                        irFunc.getResults(), module,
                                        false, false);

  // Name LLVM arguments, insert into symtable
  auto arg = func->arg_begin();
  for (auto &irArg : irFunc.getArguments()) {
    arg->setName(irArg.getName());
    symtable.insert(arg->getName(), &(*arg));
    arg++;
  }
  for (auto &irRes : irFunc.getResults()) {
    arg->setName(irRes.getName());
    symtable.insert(arg->getName(), &(*arg));
    arg++;
  }

  // TODO(gkanwar): Deal with temps?

  // Build 'entry' basic block
  llvm::BasicBlock *entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", func);
  builder.reset(new llvm::IRBuilder<>(entry));

  irFunc.getBody().accept(this);

  // NVVM kernel should always return void
  builder->CreateRetVoid();

  return new GPUFunction(irFunc, func, module);
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
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::TupleRead *op) {
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::IndexRead *op) {
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::Length *op) {
  not_supported_yet;
}
void GPUBackend::visit(const ir::Map *op) {
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::IndexedTensor *op) {
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::IndexExpr *op) {
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::TensorWrite *op) {
  ASSERT(false && "No code generation for this type");
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
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::FieldWrite *op) {
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::Store *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::ForRange *op) {
  // TODO(gkanwar): Some for domains should perhaps be handled differently
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::For *op) {
  std::string iName = op->var.getName();
  ir::ForDomain domain = op->domain;
  
  // Only supports sharding over index set
  bool sharded = true;
  llvm::Value *index;
  if (iName == xVar) {
    index = getTidX();
  }
  else if (iName == yVar) {
    index = getTidY();
  }
  else if (iName == zVar) {
    index = getTidZ();
  }
  else {
    if (xVar.empty()) {
      xVar = iName;
      index = getTidX();
    }
    else if (yVar.empty()) {
      yVar = iName;
      index = getTidY();
    }
    else if (zVar.empty()) {
      zVar = iName;
      index = getTidZ();
    }
    else {
      sharded = false;
    }
  }

  if (sharded) {
    symtable.scope();
    symtable.insert(iName, index);
    LLVMBackend::compile(op->body);
    symtable.unscope();
  }
  else {
    LLVMBackend::visit(op);
  }
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

llvm::Value *GPUBackend::getTidX() {
  llvm::FunctionType *funcTy = llvm::FunctionType::get(LLVM_INT, false);
  llvm::Function *func = llvm::cast<llvm::Function>(
      module->getOrInsertFunction("llvm.nvvm.read.ptx.sreg.tid.x", funcTy));
  // Attribute groups disallowed in NVVM
  func->removeFnAttr(llvm::Attribute::ReadNone);
  func->removeFnAttr(llvm::Attribute::NoUnwind);
  return builder->CreateCall(func);
}

llvm::Value *GPUBackend::getTidY() {
  llvm::FunctionType *funcTy = llvm::FunctionType::get(LLVM_INT, false);
  llvm::Function *func = llvm::cast<llvm::Function>(
      module->getOrInsertFunction("llvm.nvvm.read.ptx.sreg.tid.y", funcTy));
  // Attribute groups disallowed in NVVM
  func->removeFnAttr(llvm::Attribute::ReadNone);
  func->removeFnAttr(llvm::Attribute::NoUnwind);
  return builder->CreateCall(func);
}

llvm::Value *GPUBackend::getTidZ() {
  llvm::FunctionType *funcTy = llvm::FunctionType::get(LLVM_INT, false);
  llvm::Function *func = llvm::cast<llvm::Function>(
      module->getOrInsertFunction("llvm.nvvm.read.ptx.sreg.tid.z", funcTy));
  // Attribute groups disallowed in NVVM
  func->removeFnAttr(llvm::Attribute::ReadNone);
  func->removeFnAttr(llvm::Attribute::NoUnwind);
  return builder->CreateCall(func);
}

}
}
