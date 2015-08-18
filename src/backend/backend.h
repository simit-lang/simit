#ifndef SIMIT_BACKEND_H
#define SIMIT_BACKEND_H

#include <vector>
#include <map>

#include "uncopyable.h"

namespace simit {

namespace ir {
class Func;
class Stmt;
struct Environment;
}

namespace backend {
class Function;

/// Code generators are used to turn Simit IR into some other representation.
/// Examples include LLVM IR, compiled machine code and Graphviz .dot files.
class Backend : simit::interfaces::Uncopyable {
public:
  Backend() {}
  virtual ~Backend() {}

  /// Compiles a statement with an environment to a runable function with
  /// no parameters.
  backend::Function* compile(const ir::Stmt &stmt, const ir::Environment &env);

  /// Compiles an IR function to a runable function with the same parameters.
  virtual backend::Function* compile(const ir::Func &func) = 0;
};

}}
#endif
