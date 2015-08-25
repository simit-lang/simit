#ifndef SIMIT_BACKEND_IMPL_H
#define SIMIT_BACKEND_IMPL_H

#include <vector>
#include "uncopyable.h"

namespace simit {
namespace ir {
class Var;
class Func;
}
namespace backend {
class Function;

class BackendImpl : simit::interfaces::Uncopyable {
public:
  /// Compile the closure consisting of the function and a context.
  virtual backend::Function* compile(const ir::Func& func,
                                     const std::vector<ir::Var>& globals) = 0;
};

}}
#endif
