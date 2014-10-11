#include "reductions.h"

namespace simit {
namespace ir {

std::string ReductionOperator::getName() {
  switch (kind) {
    case Sum:
      return "sum";
  }
}

std::ostream &operator<<(std::ostream &os, const ReductionOperator &rop) {
  switch (rop.getKind()) {
    case ReductionOperator::Sum:
      os << "+";
      break;
  }
  return os;
}

}} //namespace simit::ir
