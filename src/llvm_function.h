#ifndef SIMIT_LLVM_FUNCTION_H
#define SIMIT_LLVM_FUNCTION_H

#include <string>
#include <memory>

#include "llvm/IR/Module.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"

#include "function.h"
#include "ir.h"

namespace llvm {
class ExecutionEngine;
}

namespace simit {
namespace internal {

/// A Simit function that has been compiled with LLVM.
class LLVMFunction : public simit::Function {
 public:
  LLVMFunction(ir::Func simitFunc, llvm::Function *llvmFunc,
               bool requiresInit, llvm::Module *module,
               std::shared_ptr<llvm::ExecutionEngine> executionEngine);

  ~LLVMFunction();

  void print(std::ostream &os) const;

 private:
  llvm::Function                         *llvmFunc;
  llvm::Module*                          module;
  std::shared_ptr<llvm::ExecutionEngine> executionEngine;

  bool requiresInit;
  FuncType deinit;

  FuncType init(const std::vector<std::string> &formals,
                std::map<std::string, Actual> &actuals);

  FuncType createHarness(const std::string &name,
                         const llvm::SmallVector<llvm::Value*,8> &args);

  llvm::Function *getInitFunc() const;
  llvm::Function *getDeinitFunc() const;
};

}}  // namespace simit::internal

#endif
