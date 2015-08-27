#ifndef SIMIT_LLVM_FUNCTION_H
#define SIMIT_LLVM_FUNCTION_H

#include <string>
#include <memory>

#include "llvm/IR/Module.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"

#include "backend/backend_function.h"
#include "ir.h"

namespace llvm {
class ExecutionEngine;
}

namespace simit {
namespace backend {
class Actual;

/// A Simit function that has been compiled with LLVM.
class LLVMFunction : public backend::Function {
 public:
  LLVMFunction(ir::Func func, const std::vector<ir::Var>& globals,
               llvm::Function* llvmFunc, llvm::Module* module,
               std::shared_ptr<llvm::EngineBuilder> engineBuilder);
  virtual ~LLVMFunction();

  virtual void bindTensor(const std::string& bindable, void* data);
  virtual void bindSet(const std::string& bindable, simit::Set* set);

  virtual FuncType init();

  virtual bool isInitialized() {
    return initialized;
  }

  virtual void print(std::ostream &os) const;
  virtual void printMachine(std::ostream &os) const;

 private:
  llvm::Function*                        llvmFunc;
  llvm::Module*                          module;
  std::shared_ptr<llvm::EngineBuilder>   engineBuilder;
  std::shared_ptr<llvm::ExecutionEngine> executionEngine;

  /// Function actual storage
  std::map<std::string,Actual*> actuals;

  /// Globals storage
  std::map<std::string,void**> globals;

  bool initialized;
  FuncType deinit;

  FuncType createHarness(const std::string& name,
                         const llvm::SmallVector<llvm::Value*,8>& args);

  llvm::Function* getInitFunc() const;
  llvm::Function* getDeinitFunc() const;
};

}}
#endif
