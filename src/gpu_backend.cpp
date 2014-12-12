#include "gpu_backend.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

#include "error.h"
#include "gpu_function.h"
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
  llvm::Module *mod = new llvm::Module("nvvm-module", LLVM_CONTEXT);

  // Set appropriate data layout
  if (sizeof(void*) == 8)
    mod->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
                       "i64:64:64-f32:32:32-f64:64:64-v16:16:16-v32:32:32-"
                       "v64:64:64-v128:128:128-n16:32:64");
  else
    mod->setDataLayout("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
                       "i64:64:64-f32:32:32-f64:64:64-v16:16:16-v32:32:32-"
                       "v64:64:64-v128:128:128-n16:32:64");

  std::vector<llvm::Type*> paramTys;
  // Pass 1: Build param type list
  for (const simit::ir::Var &arg : irFunc.getArguments()) {
    if (arg.getType().isSet()) {
      // Set arguments get expanded into their fields, because this is
      // how set data is pushed to the GPU.
      for (auto field : arg.getType().toSet()
               ->elementType.toElement()->fields) {
        paramTys.push_back(createLLVMType(field.type));
      }
    }
    else {
      paramTys.push_back(createLLVMType(arg.getType()));
    }
  }
  for (const simit::ir::Var &res : irFunc.getResults()) {
    paramTys.push_back(createLLVMType(res.getType()));
  }

  llvm::FunctionType *funcTy = llvm::FunctionType::get(
      LLVM_VOID, paramTys, false);
  llvm::Function *func = llvm::Function::Create(
      funcTy, llvm::GlobalValue::ExternalLinkage, "kernel", mod);

  // Pass 2: Name LLVM arguments, insert into symtable
  auto arg = func->arg_begin();
  for (auto &irArg : irFunc.getArguments()) {
    if (irArg.getType().isSet()) {
      for (auto field : irArg.getType().toSet()
               ->elementType.toElement()->fields) {
        // XXX: Is '$' a safe character to use here, in that it cannot
        // be used normally for a symbol name?
        auto name = irArg.getName() + "$" + field.name;
        arg->setName(name);
        symtable.insert(arg->getName(), &(*arg));
        arg++;
      }
    }
    else {
      arg->setName(irArg.getName());
      symtable.insert(arg->getName(), &(*arg));
      arg++;
    }
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

  // Kernel metadata
  llvm::Value *mdVals[] = {
    func, llvm::MDString::get(LLVM_CONTEXT, "kernel"), llvmInt(1)
  };
  llvm::MDNode *kernelMD = llvm::MDNode::get(LLVM_CONTEXT, mdVals);
  llvm::NamedMDNode *nvvmAnnot = mod->getOrInsertNamedMetadata("nvvm.annotations");
  nvvmAnnot->addOperand(kernelMD);

  return new GPUFunction(irFunc, func, mod);
}

llvm::Value *GPUBackend::compile(const ir::Expr &expr) {
  expr.accept(this);
  llvm::Value *tmp = val;
  val = nullptr;
  return tmp;
}

void GPUBackend::visit(const ir::FieldRead *op) {
  ASSERT(false && "No code generation for this type");
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
void GPUBackend::visit(const ir::For *op) {
  // TODO(gkanwar): Some for domains should perhaps be handled differently
  LLVMBackend::visit(op);
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

}
}
