#include "lower.h"

#include "ir.h"
#include "ir_rewriter.h"

using namespace std;

namespace simit {
namespace ir {

/// Lowers index expressions to (loop) statements
class LowerIndexExpressions : public IRRewriter {
  void visit(const IndexExpr *op) {
    stmt = Pass::make();
  }

  void visit(const AssignStmt *op) {
    op->value.accept(this);
    if (!stmt.defined()) {
      stmt = op;
    }
  }

  void visit(const FieldWrite *op) {
    op->value.accept(this);
    if (!stmt.defined()) {
      stmt = op;
    }
  }

  void visit(const TensorWrite *op) {
    op->value.accept(this);
    if (!stmt.defined()) {
      stmt = op;
    }
  }
};

Func lowerIndexExpressions(Func func) {
  return LowerIndexExpressions().rewrite(func);
}

}}
