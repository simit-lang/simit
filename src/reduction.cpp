#include "reduction.h"
#include "error.h"

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
  simit_unreachable;
  return "";
}

bool operator==(const ReductionOperator &l, const ReductionOperator &r) {
  return l.getKind() == r.getKind();
}

bool operator!=(const ReductionOperator &l, const ReductionOperator &r) {
  return !(l == r);
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

}}
