#ifndef SIMIT_ASSIGN_CHECKER_H
#define SIMIT_ASSIGN_CHECKER_H

#include <vector>

#include "hir.h"
#include "hir_visitor.h"
#include "error.h"

namespace simit {
namespace hir {

// Analysis for verifying that assignment targets are valid.
class AssignChecker : public HIRVisitor {
public:
  AssignChecker(std::vector<ParseError> *errors) : errors(errors) {}

  void check(Program::Ptr program) { program->accept(this); }

private:
  virtual void visit(AssignStmt::Ptr);

  void checkTarget(Expr::Ptr);

  void reportError(std::string, HIRNode::Ptr);

private:
  std::vector<ParseError> *errors;
};

}
}

#endif

