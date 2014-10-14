#ifndef SIMIT_CODEGEN_H
#define SIMIT_CODEGEN_H

#include <vector>
#include <map>

#include "interfaces.h"

namespace simit {
class Diagnostics;
class Function;

namespace ir {
class Func;
}

namespace internal {
class ProgramContext;

/// Code generators are used to turn Simit IR into some other representation.
/// Examples include LLVM IR, compiled machine code and Graphviz .dot files.
class Backend : simit::interfaces::Uncopyable {
public:
  Backend() {}
  virtual ~Backend() {}

  virtual simit::Function *compile(simit::ir::Func *function) = 0;
};

}} // namespace simit::internal
#endif
