#ifndef SIMIT_LLVM_FUNCTION_H
#define SIMIT_LLVM_FUNCTION_H

#include <string>

#include "llvm/IR/Module.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"

#include "function.h"
#include "ir.h"
#include "llvm_codegen.h"
#include "storage.h"

namespace llvm {
class ExecutionEngine;
}

namespace simit {
namespace internal {

/// A Simit function that has been compiled with LLVM.
class LLVMFunction : public simit::Function {
 public:
  LLVMFunction(simit::internal::Function *simitFunc,
               llvm::Function *llvmFunc,
               const std::shared_ptr<llvm::ExecutionEngine> &llvmFuncEE,
               const std::vector<std::shared_ptr<Storage>> &storage);

  ~LLVMFunction();

  void bind(const std::vector<std::shared_ptr<Literal>> &arguments,
            const std::vector<std::shared_ptr<Literal>> &results);

  void print(std::ostream &os) const;

 private:
  simit::internal::Function *simitFunc;
  llvm::Function *llvmFunc;
  std::shared_ptr<llvm::ExecutionEngine> llvmFuncEE;
  std::vector<std::shared_ptr<Storage>> storage;
  llvm::Module module;
};


}}  // namespace simit::internal

#endif
