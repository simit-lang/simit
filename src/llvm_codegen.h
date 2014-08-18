#ifndef SIMIT_CODEGEN_LLVM_H
#define SIMIT_CODEGEN_LLVM_H

#include "codegen.h"
#include <ostream>

namespace simit {
namespace internal {
class Function;

/// Code generator that uses LLVM to compile Simit IR.
class LLVMCodeGen : public CodeGen {
 public:
  LLVMCodeGen();
  ~LLVMCodeGen();

  BinaryFunction *compileToFunctionPointer(Function *function);

 private:
  /// Implementation class to avoid bleeding LLVM to the rest of the project.
  LLVMCodeGenImpl *impl;
};

}} // namespace simit::internal
#endif
