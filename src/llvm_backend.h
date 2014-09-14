#ifndef SIMIT_CODEGEN_LLVM_H
#define SIMIT_CODEGEN_LLVM_H

#include "backend.h"
#include "irvisitors.h"

#include <ostream>
#include <stack>
#include <map>
#include <memory>
#include <vector>

namespace llvm {
class LLVMContext;
class Module;
class ExecutionEngine;
class ConstantFolder;
template<bool> class IRBuilderDefaultInserter;
template<bool, typename, typename> class IRBuilder;
class Value;
class Instruction;
class Function;
}

namespace simit {
namespace internal {
class Function;
template <typename, typename> class ScopedMap;

/// Code generator that uses LLVM to compile Simit IR.
class LLVMBackend : public Backend, IRVisitor {
public:
  LLVMBackend();
  ~LLVMBackend();

  simit::Function *compile(Function *function);

private:
  static bool llvmInitialized;
  llvm::Module *module;
  std::shared_ptr<llvm::ExecutionEngine> executionEngine;
  llvm::IRBuilder<true, llvm::ConstantFolder,
                  llvm::IRBuilderDefaultInserter<true> > *builder;

  ScopedMap<std::string, llvm::Value*> *symtable;
  std::stack<llvm::Value*> resultStack;
  std::map<IRNode*, llvm::Value*> storageLocations;

  void handle(Function  *f);
  void handle(IndexExpr *t);

  llvm::Function *codegen(Function *f, const std::map<IRNode*, void*> &temps);
};

}} // namespace simit::internal
#endif
