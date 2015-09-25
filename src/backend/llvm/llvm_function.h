#ifndef SIMIT_LLVM_FUNCTION_H
#define SIMIT_LLVM_FUNCTION_H

#include <string>
#include <vector>
#include <map>
#include <memory>

#include "llvm/IR/Module.h"
#include "llvm/ExecutionEngine/ExecutionEngine.h"

#include "backend/backend_function.h"
#include "ir.h"
#include "storage.h"

namespace llvm {
class ExecutionEngine;
}

namespace simit {

namespace pe {
class PathExpression;
class PathIndex;
class PathIndexBuilder;
}
namespace backend {
class Actual;

/// A Simit function that has been compiled with LLVM.
class LLVMFunction : public backend::Function {
 public:
  LLVMFunction(ir::Func func, const ir::Storage &storage,
               llvm::Function* llvmFunc, llvm::Module* module,
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

 protected:
  /// Get the number of elements in the index domains.
  size_t size(const ir::IndexDomain &dimension);

  void initIndices(pe::PathIndexBuilder& piBuilder,
                   const ir::Environment& environment);

  bool initialized;

  llvm::Function*                        llvmFunc;
  llvm::Module*                          module;
  ir::Storage storage;

  /// Function actual storage
  std::map<std::string, std::unique_ptr<Actual>> arguments;
  std::map<std::string, std::unique_ptr<Actual>> globals;

  /// Externs
  std::map<std::string, std::vector<void**>> externPtrs;

  /// TensorIndices
  std::map<pe::PathExpression,
           std::pair<const uint32_t**,const uint32_t**>> tensorIndexPtrs;
  std::map<pe::PathExpression, pe::PathIndex>            pathIndices;

 private:
  std::shared_ptr<llvm::EngineBuilder>   engineBuilder;
  std::shared_ptr<llvm::ExecutionEngine> executionEngine;

  /// Temporaries
  std::map<std::string, void**> temporaryPtrs;

  FuncType deinit;

  FuncType createHarness(const std::string& name,
                         const llvm::SmallVector<llvm::Value*,8>& args);

  llvm::Function* getInitFunc() const;
  llvm::Function* getDeinitFunc() const;
};

}}
#endif
