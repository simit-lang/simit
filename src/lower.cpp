#include "lower.h"

#include "ir_mutator.h"
#include "sig.h"
#include "util.h"

using namespace std;

namespace simit {
namespace ir {

class SIGBuilder : public IRVisitor {
public:
  SIG create(const IndexExpr *expr) {
    return create(Expr(expr));
  }

private:
  SIG igraph;

  SIG create(Expr expr) {
    expr.accept(this);
    SIG result = igraph;
    igraph = SIG();
    return result;
  }

  void visit(const IndexedTensor *op) {
    if (op->indexVars.size() == 1) {
      igraph = SIG(op->indexVars[0]);
    }
    else if (op->indexVars.size() >= 2) {
      const Variable *var = to<Variable>(op->tensor);
      igraph = SIG(var->name, op->indexVars);
    }
  }

  void visit(const Add *op) {
    SIG ig1 = create(op->a);
    SIG ig2 = create(op->b);
    igraph = merge(ig1, ig2, SIG::Union);
  }

  void visit(const Sub *op) {
    SIG ig1 = create(op->a);
    SIG ig2 = create(op->b);
    igraph = merge(ig1, ig2, SIG::Union);
  }

  void visit(const Mul *op) {
    SIG ig1 = create(op->a);
    SIG ig2 = create(op->b);
    igraph = merge(ig1, ig2, SIG::Intersection);
  }

  void visit(const Div *op) {
    SIG ig1 = create(op->a);
    SIG ig2 = create(op->b);
    igraph = merge(ig1, ig2, SIG::Intersection);
  }
};

class IndexedTensorsToLoads : public IRMutator {
  void visit(const IndexedTensor *op) {
//    expr = TensorLoad()
  }
};

class LoopBuilder : public SIGVisitor {
public:
  Stmt create(const SIG &g, const IndexExpr *indexExpr) {
    apply(g);
    Stmt result = stmt;
    stmt = Stmt();
    return result;
  }

private:
  Stmt stmt;

  void visit(const SIGVertex *v) {
    SIGVisitor::visit(v);

    if (!stmt.defined()) {
      stmt = Pass::make();
    }

    const SIGEdge *previous = getPreviousEdge();
    if (previous == nullptr) {
      const IndexSet &domain = v->iv.getDomain().getIndexSets()[0];
      stmt = For::make(v->iv.getName(), domain, stmt);
    }
    else {
      const IndexSet &domain = v->iv.getDomain().getIndexSets()[0];
      stmt = For::make("derived", domain, stmt);
    }
  }

  void visit(const SIGEdge *e) {
    SIGVisitor::visit(e);
    if (!stmt.defined()) {
      stmt = Pass::make();
    }
//    stmt = Block::make(Pass::make(), stmt);
  }
};

class LowerIndexExpressions : public IRMutator {
  void visit(const IndexExpr *op) {
    SIG igraph = SIGBuilder().create(op);
    Stmt loops = LoopBuilder().create(igraph, op);
    cout << loops << endl;
    stmt = loops;
  }

  void visit(const AssignStmt *op) {
    if (!op->value.type().isTensor()) {
      IRMutator::visit(op);
    }
    else {
      // By calling accept on op->value directly we bypass the mutate method
      // and allow the visit method of the tensor type to return an stmt.
      op->value.accept(this);
    }
  }

  void visit(const FieldWrite *op) {
    if (!op->value.type().isTensor()) {
      IRMutator::visit(op);
    }
    else {
      // By calling accept on op->value directly we bypass the mutate method
      // and allow the visit method of the tensor type to return an stmt.
      op->value.accept(this);
    }
  }

  void visit(const TensorWrite *op) {
    if (!op->value.type().isTensor()) {
      IRMutator::visit(op);
    }
    else {
      // By calling accept on op->value directly we bypass the mutate method
      // and allow the visit method of the tensor type to return an stmt.
      op->value.accept(this);
    }
  }
};

Func lowerIndexExpressions(Func func) {
  return LowerIndexExpressions().mutate(func);
}
}}
