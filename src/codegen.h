#ifndef SIMIT_CODEGEN_H
#define SIMIT_CODEGEN_H

#include "irvisitors.h"

namespace llvm {
class Module;
class Function;
}

namespace simit {
class Function;

/** The base class of all classes that perform code generation using LLVM. */
class LLVMCodeGen : public IRVisitor {
 public:
  LLVMCodeGen();
  virtual ~LLVMCodeGen();

  virtual void compileToFunctionPointer(const Function *function);

 private:
  llvm::Module *module;

  llvm::Function *llvmFunctionPrototype(const Function *function);
};

}
#endif
