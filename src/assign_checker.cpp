#include "assign_checker.h"
#include "hir.h"

namespace simit {
namespace hir {

void AssignChecker::visit(AssignStmt::Ptr stmt) {
  for (auto lhs : stmt->lhs) {
    checkTarget(lhs);
  }
}

void AssignChecker::checkTarget(Expr::Ptr target) {
  if (isa<TensorReadExpr>(target)) {
    checkTarget(to<TensorReadExpr>(target)->tensor);
  } else if (isa<FieldReadExpr>(target)) {
    checkTarget(to<FieldReadExpr>(target)->setOrElem);
  } else if (!isa<VarExpr>(target)) {
    reportError("values can only be assigned to variables, fields, or tensor "
                "elements", target);
  }
}

void AssignChecker::reportError(std::string msg, HIRNode::Ptr loc) {
  const auto err = ParseError(loc->getLineBegin(), loc->getColBegin(), 
                              loc->getLineEnd(), loc->getColEnd(), msg);
  errors->push_back(err);
}

}
}

