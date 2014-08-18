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




}} // namespace simit::internal
#endif
