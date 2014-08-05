#include "codegen.h"

#include <iostream>
#include "llvm/ExecutionEngine/JIT.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"

#include "ir.h"

using namespace simit;
using namespace std;

#define LLVM_CONTEXT   llvm::getGlobalContext()

#define LLVM_VOID      llvm::Type::getVoidTy(LLVM_CONTEXT)
#define LLVM_INT       llvm::Type::getInt32Ty(LLVM_CONTEXT)
#define LLVM_INTPTR    llvm::Type::getInt32PtrTy(LLVM_CONTEXT)
#define LLVM_DOUBLE    llvm::Type::getDoubleTy(LLVM_CONTEXT)
#define LLVM_DOUBLEPTR llvm::Type::getDoublePtrTy(LLVM_CONTEXT)

LLVMCodeGen::LLVMCodeGen() {
  module = new llvm::Module("Simit jit", llvm::getGlobalContext());
}

LLVMCodeGen::~LLVMCodeGen() {

}

static llvm::Type *toLLVMType(const simit::TensorType *type) {
  llvm::Type *llvmType = NULL;

  if (type->isScalar()) {
    switch (type->getComponentType()) {
      case TensorType::INT:
        llvmType = LLVM_INTPTR;
        break;
      case TensorType::FLOAT:
        llvmType = LLVM_DOUBLEPTR;
        break;
      case TensorType::ELEMENT:
        return NULL;  // TODO: not supported yet
        break;
      default:
        assert(false);
        break;
    }
  }
  else {
    return NULL;  // TODO: not supported yet
  }

  assert(llvmType != NULL);
  return llvmType;
}

void LLVMCodeGen::compileToFunctionPointer(Function *function) {
  cout << *function << endl;
  llvm::Function *f = codegen(function);
  if (f == NULL) return;
  f->dump();
}

void LLVMCodeGen::handle(Function *function) {
  llvm::Function *f = llvmPrototype(function);
  if (f == NULL) {  // TODO: Remove check
    abort();
    return;
  }
  llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", f);
  results.push(f);
}

void LLVMCodeGen::handle(Argument *t) {
  cout << "Argument: " << *t << endl;
}

void LLVMCodeGen::handle(Result *t) {
  cout << "Result:   " << *t << endl;
}

void LLVMCodeGen::handle(LiteralTensor *t) {
  cout << "Literal:  " << *t << endl;
}

void LLVMCodeGen::handle(Merge *t) {
  cout << "Merge   : " << *t << endl;
}

void LLVMCodeGen::handle(VariableStore *t) {
  cout << "Store   : " << *t << endl;
}

llvm::Function *LLVMCodeGen::codegen(Function *function) {
  visit(function);
  if (isAborted()) {
    return NULL;
  }
  llvm::Value *value = results.top();
  results.pop();
  assert(llvm::isa<llvm::Function>(value));
  return llvm::cast<llvm::Function>(value);
}

llvm::Function *LLVMCodeGen::llvmPrototype(Function *function) const {
 vector<llvm::Type*> args;
  for (auto &arg : function->getArguments()) {
    auto llvmType = toLLVMType(arg->getType());
    if (llvmType == NULL) return NULL;  // TODO: Remove check
    args.push_back(llvmType);
  }
  for (auto &result : function->getResults()) {
    auto llvmType = toLLVMType(result->getType());
    if (llvmType == NULL) return NULL;  // TODO: Remove check
    args.push_back(llvmType);
  }

  llvm::FunctionType *ft = llvm::FunctionType::get(LLVM_VOID, args, false);
  llvm::Function *f = llvm::Function::Create(ft,
                                             llvm::Function::ExternalLinkage,
                                             function->getName(),
                                             module);

  auto ai = f->arg_begin();
  for (auto &arg : function->getArguments()) {
    ai->setName(arg->getName());
    ++ai;
  }
  for (auto &result : function->getResults()) {
    ai->setName(result->getName());
    ++ai;
  }
  assert(ai == f->arg_end());

  return f;
}
