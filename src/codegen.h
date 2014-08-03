#ifndef SIMIT_CODEGEN_H
#define SIMIT_CODEGEN_H

#include "irvisitors.h"

namespace simit {

/** The base class of all classes that perform code generation using LLVM. */
class LLVMCodeGen {// : public IRVisitor {
 public:
  LLVMCodeGen() {}
  virtual ~LLVMCodeGen() {}

  virtual void compileToFunctionPointer();

 private:
};

}
#endif
