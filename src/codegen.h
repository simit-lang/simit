#ifndef SIMIT_CODEGEN_H
#define SIMIT_CODEGEN_H

#include <ostream>
#include <vector>
#include "ir.h"
#include "interfaces.h"

#include <iostream>

namespace simit {
namespace internal {
class Function;


class CompiledFunction : simit::util::Uncopyable {
 public:
  typedef void (*RunPtrType)();

  CompiledFunction(RunPtrType runptr=NULL) : runPtr(runptr) {}
  virtual ~CompiledFunction() {}

  /// Binds the argument and results to the function.  The argument and result
  /// types must match the arguments and results expected by the function.
  /// Note that since this is a performance-sensitive low-level primitive,
  /// there are no safety checks to catch if you bind the wrong number or types.
  virtual void bind(const std::vector<std::shared_ptr<Literal>> &arguments,
                    const std::vector<std::shared_ptr<Literal>> &results) = 0;

  /// Execute the function with the currently bound argument and result tensors.
  inline void run() { runPtr(); }

  virtual void print(std::ostream &os) const {};

 protected:
  inline void setRunPtr(RunPtrType runPtr) { this->runPtr = runPtr; }

 private:
  RunPtrType runPtr;
};

inline std::ostream &operator<<(std::ostream &os, const CompiledFunction &cf) {
  cf.print(os);
  return os;
}


/// Code generators are used to turn Simit IR into some other representation.
/// Examples include LLVM IR, compiled machine code and Graphviz .dot files.
class CodeGen : simit::util::Uncopyable {
 public:
  CodeGen() {}
  virtual ~CodeGen() {}

  virtual CompiledFunction *compile(Function *function) = 0;
};

}} // namespace simit::internal
#endif
