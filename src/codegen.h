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

  virtual void compileToFunctionPointer(const Function &function);

  void handle(const Function &function);

 private:
  llvm::Module *module;
  std::stack<llvm::Value*> results;

  llvm::Function *codegen(const Function &function);
  llvm::Function *llvmPrototype(const Function &function) const;
};

}
#endif
