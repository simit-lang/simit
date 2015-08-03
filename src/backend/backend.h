#ifndef SIMIT_BACKEND_H
#define SIMIT_BACKEND_H

#include <vector>
#include <map>

#include "function.h"
#include "uncopyable.h"
#include "backend/backend_function.h"

namespace simit {
namespace ir {
class Func;
}
namespace internal {
class ProgramContext;
}

namespace backend {

/// Code generators are used to turn Simit IR into some other representation.
/// Examples include LLVM IR, compiled machine code and Graphviz .dot files.
class Backend : simit::interfaces::Uncopyable {
public:
  Backend() {}
  virtual ~Backend() {}

  virtual simit::Function compile(const ir::Func &func) = 0;
};

}}
#endif
