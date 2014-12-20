#include "lower.h"

#include "ir.h"
#include "ir_rewriter.h"
#include "ir_printer.h"
#include "sig.h"

using namespace std;

namespace simit {
namespace ir {

Stmt lower(Stmt stmt, const Storage &storage) {
  /// Specializes index expressions to compute the value/block for the loop
  /// iteration given by the map from index variables to loop variables.
  class SpecializeIndexExpression : private IRRewriter {
  public:
    SpecializeIndexExpression(const LoopVars &loopVars) : loopVars(loopVars) {}
    Stmt specialize(Stmt stmt) { return this->rewrite(stmt); }

  private:
    /// Loop variables used to compute index (created from the SIG)
    LoopVars const& loopVars;

    void visit(const AssignStmt *op) {
      iassert(isa<IndexExpr>(op->value))<<"Can only specialize IndexExpr stmts";
      const IndexExpr *indexExpr = to<IndexExpr>(op->value);

      Var var = op->var;
      Expr value = rewrite(indexExpr);

      if (isScalar(var.getType())) {
        stmt = AssignStmt::make(var, value);
      }
      else {
        stmt = op;
      }
    }

    void visit(const FieldWrite *op) {
      not_supported_yet;
    }

    void visit(const TensorWrite *op) {
      not_supported_yet;
    }

    /// Replace indexed tensors with tensor reads
    void visit(const IndexedTensor *op) {
      expr = op->tensor;
    }

    /// Removes index expression
    void visit(const IndexExpr *op) {
      expr = rewrite(op->value);
    }
  };

  SIG sig = createSIG(stmt, storage);
  LoopVars loopVars = LoopVars::create(sig);
//  std::cout << loopVars << std::endl;

  // Create compute kernel (loop body)
  Stmt kernel = SpecializeIndexExpression(loopVars).specialize(stmt);

  // Create loops
  Stmt loopNest = kernel;

//  std::cout << loopNest << std::endl;
  return loopNest;
}

/// Lower the index expressions in the fiven function.
Func lowerIndexExpressions(Func func) {
  /// Visits every index expression and calls 'lower' to rewrite them to
  /// equivalent compute statements
  class LowerIndexExpressions : private IRRewriter {
  public:
    Func lower(Func func) {
      storage = &func.getStorage();
      return this->rewrite(func);
    }

  private:
    const Storage *storage;
    void visit(const AssignStmt *op) {
      if (isa<IndexExpr>(op->value))
        stmt = simit::ir::lower(op, *storage);
      else
        IRRewriter::visit(op);
    }

    void visit(const FieldWrite *op) {
      if (isa<IndexExpr>(op->value))
        stmt = simit::ir::lower(op, *storage);
      else
        IRRewriter::visit(op);
    }

    void visit(const TensorWrite *op) {
      if (isa<IndexExpr>(op->value))
        stmt = simit::ir::lower(op, *storage);
      else
        IRRewriter::visit(op);
    }
  };

  return LowerIndexExpressions().lower(func);
}

}}
