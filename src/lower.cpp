#include "lower.h"

#include "ir_mutator.h"
#include "sig.h"
#include "util.h"

using namespace std;

namespace simit {
namespace ir {

class SIGBuilder : public IRVisitor {
public:
  SIG create(const IndexStmt *indexStmt) {
    return create(indexStmt->value);
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
  Stmt create(const SIG &g, const IndexStmt *indexStmt) {
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
  void visit(const IndexStmt *op) {
    SIG igraph = SIGBuilder().create(op);
    Stmt loops = LoopBuilder().create(igraph, op);
    stmt = loops;
  }
};

Func lowerIndexExpressions(Func func) {
  return LowerIndexExpressions().mutate(func);
}
}}
