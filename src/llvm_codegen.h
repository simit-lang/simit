#ifndef SIMIT_LLVM_CODEGEN_H
#define SIMIT_LLVM_CODEGEN_H

#include <memory>
#include <string>
#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"

#include "ir.h"

#define LLVM_CONTEXT   llvm::getGlobalContext()

#define LLVM_VOID      llvm::Type::getVoidTy(LLVM_CONTEXT)
#define LLVM_INT       llvm::Type::getInt32Ty(LLVM_CONTEXT)
#define LLVM_INTPTR    llvm::Type::getInt32PtrTy(LLVM_CONTEXT)
#define LLVM_DOUBLE    llvm::Type::getDoubleTy(LLVM_CONTEXT)
#define LLVM_DOUBLEPTR llvm::Type::getDoublePtrTy(LLVM_CONTEXT)

#define LLVM_INT8      llvm::Type::getInt8Ty(LLVM_CONTEXT)
#define LLVM_INT32     llvm::Type::getInt32Ty(LLVM_CONTEXT)
#define LLVM_INT64     llvm::Type::getInt64Ty(LLVM_CONTEXT)

namespace simit {
namespace internal {

llvm::Type *toLLVMType(const simit::ComponentType type);

llvm::Type *toLLVMType(const std::shared_ptr<simit::internal::Type> &type);

llvm::Constant *
toLLVMPtr(const std::shared_ptr<simit::internal::Literal> &literal);

simit::ComponentType llvmToSimitType(const llvm::Type *type);

// TODO: Do these need to be template functions?
template <class AT, class RT>
llvm::FunctionType *createFunctionType(const std::vector<std::shared_ptr<AT>> &arguments,
                                       const std::vector<std::shared_ptr<RT>> &results) {
  std::vector<llvm::Type*> args;
  for (auto &arg : arguments) {
    args.push_back(toLLVMType(arg->getType()));
  }
  for (auto &result : results) {
    args.push_back(toLLVMType(result->getType()));
  }
  return llvm::FunctionType::get(LLVM_VOID, args, false);
}

template <class AT, class RT>
llvm::Function *createPrototype(const std::string &name,
                                const std::vector<std::shared_ptr<AT>> &arguments,
                                const std::vector<std::shared_ptr<RT>> &results,
                                llvm::GlobalValue::LinkageTypes linkage,
                                llvm::Module *module) {
  llvm::FunctionType *ft = createFunctionType(arguments, results);
  llvm::Function *f = llvm::Function::Create(ft, linkage, name, module);
  f->setDoesNotThrow();
  for (size_t i=0; i<f->getArgumentList().size(); ++i) {
    f->setDoesNotCapture(i+1);
  }

  auto ai = f->arg_begin();
  for (auto &arg : arguments) {
    ai->setName(arg->getName());
    ++ai;
  }
  for (auto &result : results) {
    ai->setName(result->getName());
    ++ai;
  }
  assert(ai == f->arg_end());
  return f;
}

}} // namespace simit::internal


#endif
