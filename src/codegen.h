#ifndef SIMIT_CODEGEN_H
#define SIMIT_CODEGEN_H

namespace simit {
namespace internal {
class Function;
class LLVMCodeGenImpl;

class BinaryFunction {
 public:
  BinaryFunction() {}
  virtual ~BinaryFunction() {}

  virtual void run() = 0;

 private:
  // Not implemented
  BinaryFunction (const BinaryFunction& other);
  BinaryFunction& operator= (BinaryFunction &other);
};

class CodeGen {
 public:
  CodeGen() {}
  virtual ~CodeGen() {}

  virtual BinaryFunction *compileToFunctionPointer(Function *function) = 0;

 private:
  // Not implemented
  CodeGen (const CodeGen& other);
  CodeGen& operator= (CodeGen &other);
};

/** The base class of all classes that perform code generation using LLVM. */
class LLVMCodeGen : public CodeGen {
 public:
  LLVMCodeGen();
  ~LLVMCodeGen();

  BinaryFunction *compileToFunctionPointer(Function *function);

 private:
  /** Implementation class to avoid bleeding LLVM to the rest of the project.
    * It is easier to pimpl here than to forward declare the LLVM classes. */
  LLVMCodeGenImpl *impl;
};

}} // namespace simit::internal
#endif
