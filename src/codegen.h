#ifndef SIMIT_CODEGEN_H
#define SIMIT_CODEGEN_H

#include "irvisitors.h"
#include <stack>
#include "symboltable.h"

// TODO: pimpl to avoid leaking llvm include files
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Value.h"

namespace llvm {
class Module;
class Value;
class Function;
}

namespace simit {
namespace internal {
class Function;

/** The base class of all classes that perform code generation using LLVM. */
class LLVMCodeGen : public IRVisitor {
 public:
  LLVMCodeGen();
  virtual ~LLVMCodeGen();

  virtual void compileToFunctionPointer(Function *function);

  void handle(Function *function);
  void handle(Argument      *t);
  void handle(Result        *t);
  void handle(LiteralTensor *t);
  void handle(IndexExpr     *t);
  void handle(VariableStore *t);

 private:
  llvm::Module module;
  llvm::IRBuilder<> irBuilder;
  SymbolTable<llvm::Value*> symtable;
  std::stack<llvm::Value*> results;

  llvm::Function *codegen(Function *function);
};

}} // namespace simit::internal
#endif
