#ifndef SIMIT_CODEGEN_H
#define SIMIT_CODEGEN_H

#include <vector>
#include <map>

#include "interfaces.h"

namespace simit {
class Diagnostics;
class Function;

namespace internal {
class Function;
class ProgramContext;

/// Code generators are used to turn Simit IR into some other representation.
/// Examples include LLVM IR, compiled machine code and Graphviz .dot files.
class Backend : simit::interfaces::Uncopyable {
public:
  Backend() {}
  virtual ~Backend() {}

  virtual simit::Function *compile(Function *function) = 0;

  int verify(ProgramContext &ctx, Diagnostics *diags);
};

}} // namespace simit::internal
#endif
