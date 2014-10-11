#include "indexvar.h"

namespace simit {
namespace ir {

std::ostream &operator<<(std::ostream &os, const IndexVar &var) {
  return os << var.getOperator() << var.getName();
}

}} // namespace simit::ir
