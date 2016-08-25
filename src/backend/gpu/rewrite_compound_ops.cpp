#include "rewrite_compound_ops.h"

#include "ir_rewriter.h"

namespace simit {
namespace ir {

/// Quick pattern-matching rewriter to convert the simplest compound
/// statements for wider applicability of the GPU backend.
class CompoundOpsRewriter : public IRRewriter {
private:
  void visit(const AssignStmt *op) {
    stmt = op;
    if (op->cop != CompoundOperator::None) {
      return;
    }

    if (!isa<Add>(op->value)) {
      return;
    }

    const Add *add = to<Add>(op->value);
    if (isa<VarExpr>(add->a) &&
        to<VarExpr>(add->a)->var == op->var) {
      stmt = AssignStmt::make(op->var, add->b, CompoundOperator::Add);
    }
    else if (isa<VarExpr>(add->b) &&
             to<VarExpr>(add->b)->var == op->var) {
      stmt = AssignStmt::make(op->var, add->a, CompoundOperator::Add);
    }
  }
};

Func rewriteCompoundOps(Func func) {
  return CompoundOpsRewriter().rewrite(func);
}

}} // simit::ir
