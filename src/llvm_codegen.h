#ifndef SIMIT_CODEGEN_LLVM_H
#define SIMIT_CODEGEN_LLVM_H

#include "codegen.h"
#include <ostream>

namespace simit {
namespace internal {
class Function;
class LLVMCodeGenImpl;

/// Code generator that uses LLVM to compile Simit IR.
class LLVMCodeGen : public CodeGen {
 public:
  LLVMCodeGen();
  ~LLVMCodeGen();

  CompiledFunction *compile(Function *function);

 private:
  /// Implementation class to avoid bleeding LLVM to the rest of the project.
  LLVMCodeGenImpl *impl;
};

}} // namespace simit::internal
#endif
