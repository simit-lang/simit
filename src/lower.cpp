#include "lower.h"

#include "ir.h"
#include "ir_mutator.h"
#include "usedef.h"
#include "sig.h"
#include "util.h"

using namespace std;

namespace simit {
namespace ir {

class SIGBuilder : public IRVisitor {
public:
  SIGBuilder(const UseDef *ud) : ud(ud) {}

  SIG create(const IndexExpr *expr) {
    return create(Expr(expr));
  }

private:
  const UseDef *ud;
  SIG sig;

  SIG create(Expr expr) {
    expr.accept(this);
    SIG result = sig;
    sig = SIG();
    return result;
  }

  void visit(const IndexedTensor *op) {
    if (op->indexVars.size() == 1) {
      sig = SIG({op->indexVars[0]});
    }
    else if (op->indexVars.size() >= 2) {
      // TODO: This does not have to be a variable
      Expr edgeSet;
      if (isa<VarExpr>(op->tensor)) {
//        const VarExpr *varExpr = to<VarExpr>(op->tensor);
      }

      sig = SIG(op->indexVars, edgeSet);
    }
  }

  void visit(const Add *op) {
    SIG ig1 = create(op->a);
    SIG ig2 = create(op->b);
    sig = merge(ig1, ig2, SIG::Union);
  }

  void visit(const Sub *op) {
    SIG ig1 = create(op->a);
    SIG ig2 = create(op->b);
    sig = merge(ig1, ig2, SIG::Union);
  }

  void visit(const Mul *op) {
    SIG ig1 = create(op->a);
    SIG ig2 = create(op->b);
    sig = merge(ig1, ig2, SIG::Intersection);
  }

  void visit(const Div *op) {
    SIG ig1 = create(op->a);
    SIG ig2 = create(op->b);
    sig = merge(ig1, ig2, SIG::Intersection);
  }
};

class LoopVars : public SIGVisitor {
public:
  LoopVars(SIG const& sig) {
    apply(sig);
  }

  Var const& getVar(IndexVar const& var) const {
    return vertexLoopvars.at(var);
  }

private:
  map<IndexVar, Var> vertexLoopvars;
  map<Expr, Var> edgeLoopvars;

  void visit(const SIGVertex *v) {
    vertexLoopvars[v->iv] = Var(v->iv.getName(), Int(32));
  }
};

/// Specialize a statement containing an index expression to compute one value.
class SpecializeIndexExprStmt : public IRMutator {
public:
  SpecializeIndexExprStmt(const LoopVars &lvs) : lvs(lvs) {}

private:
  const LoopVars &lvs;

  void visit(const IndexExpr *op) {
    expr = mutate(op->value);
  }

  void visit(const IndexedTensor *op) {
    if (isa<VarExpr>(op->tensor)) {
      std::vector<Expr> indices;
      for (IndexVar const& iv : op->indexVars) {
        indices.push_back(lvs.getVar(iv));
      }
      expr = TensorRead::make(op->tensor, indices);
    }
    else {
      op->tensor.accept(this);
      // Flatten index expression. E.g. ((i) A(i,j) *  ((m) c(m)+b(m))(j)  )
      expr = op;
      NOT_SUPPORTED_YET;
    }
  }
};

class LoopBuilder : public SIGVisitor {
public:
  Stmt create(const IndexExpr *indexExpr, Stmt body, const SIG &sig) {
    stmt = body;
    apply(sig);
    Stmt result = stmt;
    stmt = Stmt();
    return result;
  }

private:
  Stmt stmt;

  void visit(const SIGVertex *v) {
    SIGVisitor::visit(v);

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
    // Emit loops
  }
};

class LowerIndexExpressions : public IRMutator {
public:
  LowerIndexExpressions(const UseDef *ud) : ud(ud) {}

private:
  const UseDef *ud;

  Stmt lower(const IndexExpr *indexExpr, Stmt stmt) {
    SIG sig = SIGBuilder(ud).create(indexExpr);
    LoopVars lvs(sig);
    Stmt body = SpecializeIndexExprStmt(lvs).mutate(stmt);
    return LoopBuilder().create(indexExpr, body, sig);
  }

  void visit(const IndexExpr *op) {
    assert(false &&
           "IndexExprs must be assigned to a var/field/tensor before lowering");
  }

  void visit(const AssignStmt *op) {
    if (isa<IndexExpr>(op->value)) {
      stmt = lower(to<IndexExpr>(op->value), op);
    }
    else {
      IRMutator::visit(op);
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
  UseDef ud(func);
  cout << ud << endl;
  return LowerIndexExpressions(&ud).mutate(func);
}
}}
