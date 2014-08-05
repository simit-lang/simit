#ifndef SIMIT_CODEGEN_H
#define SIMIT_CODEGEN_H

#include <stack>

#include "irvisitors.h"

namespace llvm {
class Module;
class Value;
class Function;
}

namespace simit {
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
  void handle(Merge         *t);
  void handle(VariableStore *t);

 private:
  llvm::Module *module;
  std::stack<llvm::Value*> results;

  llvm::Function *codegen(Function *function);
  llvm::Function *llvmPrototype(Function *function) const;
};

}
#endif
