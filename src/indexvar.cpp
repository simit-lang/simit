#include "indexvar.h"

namespace simit {
namespace ir {

std::ostream &operator<<(std::ostream &os, const IndexVar &var) {
  if (var.isReductionVar()) {
    os << var.getOperator();
  }
  return os << var.getName();
}

}} // namespace simit::ir
