#include "indexvar.h"

namespace simit {
namespace ir {

// class ReductionOperator
std::string ReductionOperator::getName() {
  switch (kind) {
    case Sum:
      return "sum";
    case Undefined:
      return "";
  }
}

std::ostream &operator<<(std::ostream &os, const ReductionOperator &rop) {
  switch (rop.getKind()) {
    case ReductionOperator::Sum:
      os << "+";
      break;
    case ReductionOperator::Undefined:
      break;
  }
  return os;
}

// class IndexVar
std::ostream &operator<<(std::ostream &os, const IndexVar &var) {
  if (var.isReductionVar()) {
    os << var.getOperator();
  }
  return os << var.getName();
}

}} // namespace simit::ir
