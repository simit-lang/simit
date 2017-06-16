#include "indexvar.h"

#include "ir.h"

namespace simit {
namespace ir {

// class IndexVarContent
IndexVarContent::~IndexVarContent() {
  delete fixedExpr;
}

// class IndexVar
std::ostream &operator<<(std::ostream &os, const IndexVar &var) {
  simit_iassert(var.defined()) << "Undefined IndexVar";
  if (var.isReductionVar()) {
    os << var.getOperator();
  }
  return os << var.getName();
}

}} // namespace simit::ir
