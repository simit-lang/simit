#ifndef SIMIT_CODEGEN_H
#define SIMIT_CODEGEN_H

namespace simit {
namespace internal {
class Function;
class LLVMCodeGenImpl;

/** The base class of all classes that perform code generation using LLVM. */
class LLVMCodeGen {
 public:
  LLVMCodeGen();
  ~LLVMCodeGen();

  void compileToFunctionPointer(Function *function);

 private:
  // Not implemented
  LLVMCodeGen (const LLVMCodeGen& other);
  LLVMCodeGen& operator= (LLVMCodeGen other);

  /** Implementation class to avoid bleeding LLVM to the rest of the project. */
  LLVMCodeGenImpl *impl;
};

}} // namespace simit::internal
#endif
