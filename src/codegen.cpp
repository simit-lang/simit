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

static llvm::Type *llvmType(const simit::TensorType *type) {
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
        assert(false && "TODO: not supported yet");
        break;
      default:
        assert(false);
        break;
    }
  }
  else {
    assert(false && "TODO: not supported yet");
  }

  assert(llvmType != NULL);
  return llvmType;
}

void LLVMCodeGen::compileToFunctionPointer(const Function &function) {
  cout << function << endl;
  llvm::Function *f = codegen(function);
  f->dump();
}

void LLVMCodeGen::handle(const Function &function) {
  llvm::Function *f = llvmPrototype(function);
  llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", f);
  results.push(f);
}

llvm::Function *LLVMCodeGen::codegen(const Function &function) {
  visit(function);
  llvm::Value *value = results.top();
  results.pop();
  assert(llvm::isa<llvm::Function>(value));
  return llvm::cast<llvm::Function>(value);
}

llvm::Function *LLVMCodeGen::llvmPrototype(const Function &function) const {
 vector<llvm::Type*> args;
  for (auto &arg : function.getArguments()) {
    args.push_back(llvmType(arg->getType()));
  }
  for (auto &result : function.getResults()) {
    args.push_back(llvmType(result->getType()));
  }

  llvm::FunctionType *ft = llvm::FunctionType::get(LLVM_VOID, args, false);
  llvm::Function *f = llvm::Function::Create(ft,
                                             llvm::Function::ExternalLinkage,
                                             function.getName(),
                                             module);

  auto ai = f->arg_begin();
  for (auto &arg : function.getArguments()) {
    ai->setName(arg->getName());
    ++ai;
  }
  for (auto &result : function.getResults()) {
    ai->setName(result->getName());
    ++ai;
  }
  assert(ai == f->arg_end());

  return f;
}
