#ifndef SIMIT_ASSIGN_CHECKER_H
#define SIMIT_ASSIGN_CHECKER_H

#include "hir.h"
#include "hir_visitor.h"
#include "error.h"

namespace simit {
namespace hir {

class AssignChecker : public HIRVisitor {
public:
  AssignChecker(std::vector<ParseError> *errors) : errors(errors) {}

  void check(Program::Ptr program) { program->accept(this); }

private:
  virtual void visit(AssignStmt::Ptr);

  void checkTarget(Expr::Ptr);

  void reportError(const std::string msg, HIRNode::Ptr loc) {
    const auto err = ParseError(loc->getLineBegin(), loc->getColBegin(), 
                                loc->getLineEnd(), loc->getColEnd(), msg);
    errors->push_back(err);
  }

  std::vector<ParseError> *errors;
};

}
}

#endif

