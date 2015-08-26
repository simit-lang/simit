#ifndef SIMIT_BACKEND_H
#define SIMIT_BACKEND_H

#include <memory>
#include "uncopyable.h"

namespace simit {
namespace ir {
class Func;
class Stmt;
}
namespace backend {
class Function;
class BackendImpl;

/// Code generators are used to turn Simit IR into some other representation.
/// Examples include LLVM IR, compiled machine code and Graphviz .dot files.
class Backend : simit::interfaces::Uncopyable {
public:
  Backend(const std::string &type);
  ~Backend();

  /// Compiles an IR function to a runable function.
  backend::Function* compile(const ir::Func& func);

  /// Compiles an IR statement to a runable function. Any undefined variable
  /// becomes part of the runable function's environment and must be bound
  /// before the function is run.
  backend::Function* compile(const ir::Stmt& stmt);

protected:
  BackendImpl* pimpl;
};

}}
#endif
