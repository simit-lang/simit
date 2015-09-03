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
  LLVMFunction(ir::Func func, llvm::Function* llvmFunc, llvm::Module* module,
               std::shared_ptr<llvm::EngineBuilder> engineBuilder);
  virtual ~LLVMFunction();

  virtual void bind(const std::string& name, simit::Set* set);
  virtual void bind(const std::string& name, void* data);
  virtual void bind(const std::string& name, const int* rowPtr,
                    const int* colInd, void* data);

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
  std::map<std::string, std::unique_ptr<Actual>> actuals;

  /// Globals storage
  std::map<std::string, void**> externPtrs;

  bool initialized;
  FuncType deinit;

  FuncType createHarness(const std::string& name,
                         const llvm::SmallVector<llvm::Value*,8>& args);

  llvm::Function* getInitFunc() const;
  llvm::Function* getDeinitFunc() const;
};

}}
#endif
