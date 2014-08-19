#ifndef SIMIT_CODEGEN_LLVM_H
#define SIMIT_CODEGEN_LLVM_H

#include "codegen.h"
#include "irvisitors.h"

#include <ostream>
#include <stack>
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
};

namespace simit {
namespace internal {
class Function;
template <typename> class SymbolTable;

/// Code generator that uses LLVM to compile Simit IR.
class LLVMCodeGen : public CodeGen, IRVisitor {
 public:
  LLVMCodeGen();
  ~LLVMCodeGen();

  CompiledFunction *compile(Function *function);

 private:
  static bool llvmInitialized;
  llvm::Module *module;
  std::shared_ptr<llvm::ExecutionEngine> executionEngine;
  llvm::IRBuilder<true, llvm::ConstantFolder,
                  llvm::IRBuilderDefaultInserter<true> > *builder;

  SymbolTable<llvm::Value*> *symtable;
  std::stack<llvm::Value*> resultStack;

  void handle(Function *function);
  void handle(IndexExpr     *t);
  void handle(VariableStore *t);

  llvm::Function *codegen(Function *function);

  llvm::Value * createScalarOp(const std::string &name, IndexExpr::Operator op,
                         const std::vector<IndexExpr::IndexedTensor> &operands);
};

}} // namespace simit::internal
#endif
