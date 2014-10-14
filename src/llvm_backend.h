#ifndef SIMIT_CODEGEN_LLVM_H
#define SIMIT_CODEGEN_LLVM_H

#include "backend.h"
#include "ir_visitor.h"

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

template <typename, typename> class ScopedMap;
class Function;

/// Code generator that uses LLVM to compile Simit IR.
class LLVMBackend : public Backend, ir::IRVisitor {
public:
  LLVMBackend();
  ~LLVMBackend();

  simit::Function *compile(simit::ir::Func *function);

private:
  static bool llvmInitialized;
  llvm::Module *module;
  std::shared_ptr<llvm::ExecutionEngine> executionEngine;
  llvm::IRBuilder<true, llvm::ConstantFolder,
                  llvm::IRBuilderDefaultInserter<true> > *builder;

  ScopedMap<std::string, llvm::Value*> *symtable;
};

}} // namespace simit::internal
#endif
