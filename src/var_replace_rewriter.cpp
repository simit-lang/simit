#include "var_replace_rewriter.h"

#include "ir_rewriter.h"

namespace simit {
namespace ir {

class VarReplaceRewriter : public IRRewriter {
public:
  VarReplaceRewriter(Var init, Var final) : init(init), final(final) {}

  using IRRewriter::visit;

  void visit(const VarExpr *op) {
    if (op->var == init) {
      expr = VarExpr::make(final);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const VarDecl *op) {
    if (op->var == init) {
      stmt = VarDecl::make(final);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const IndexedTensor *op) {
    Expr tensor = rewrite(op->tensor);
  
    std::vector<IndexVar> indexVars;
    for (const auto iv : op->indexVars) {
      if (iv.getFixedExpr() != nullptr) {
        Expr *fixedExpr = new Expr(rewrite(*iv.getFixedExpr()));
        indexVars.push_back(IndexVar(iv.getName(), iv.getDomain(), fixedExpr));
      } else {
        indexVars.push_back(iv);
      }
    }

    expr = IndexedTensor::make(tensor, indexVars);
  }

  void visit(const AssignStmt *op) {
    Expr value = rewrite(op->value);
    if (op->var == init) {
      stmt = AssignStmt::make(final, value, op->cop);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const CallStmt *op) {
    std::vector<Var> finalResults;
    for (Var res : op->results) {
      if (res == init)
        finalResults.push_back(final);
      else
        finalResults.push_back(res);
    }
    std::vector<Expr> actuals(op->actuals.size());
    for (size_t i=0; i < op->actuals.size(); ++i) {
      actuals[i] = rewrite(op->actuals[i]);
    }
    stmt = CallStmt::make(finalResults, op->callee, actuals);
  }

  void visit(const ForRange *op) {
    Expr start = rewrite(op->start);
    Expr end = rewrite(op->end);
    Stmt body = rewrite(op->body);
    if (op->var == init) {
      stmt = ForRange::make(final, start, end, body);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const For *op) {
    Stmt body = rewrite(op->body);
    if (op->domain.var == init) {

      ForDomain domain = ForDomain(op->domain.set, final,
                                   op->domain.kind, op->domain.indexSet);
      stmt = For::make(op->var, domain, body);
    }
    else if (op->var == init) {
      stmt = For::make(final, op->domain, body);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const Map *op) {
    std::vector<Var> vars(op->vars.size());
    for (size_t i = 0; i < op->vars.size(); ++i) {
      vars[i] = (op->vars[i] == init) ? final : op->vars[i];
    }

    Expr target = rewrite(op->target);
  
    std::vector<Expr> neighbors(op->neighbors.size());
    for (size_t i = 0; i < op->neighbors.size(); ++i) {
      neighbors[i] = rewrite(op->neighbors[i]);
    }
  
    Expr through;
    if (op->through.defined()) {
      through = rewrite(op->through);
    }
    
    std::vector<Expr> partial_actuals(op->partial_actuals.size());
    for (size_t i = 0; i < op->partial_actuals.size(); ++i) {
      partial_actuals[i] = rewrite(op->partial_actuals[i]);
    }
  
    stmt = Map::make(vars, op->function, partial_actuals, target,
                     neighbors, through, op->reduction);
  }

private:
  Var init, final;
};

class ReplaceVarByExprRewriter : public IRRewriter {
public:
  ReplaceVarByExprRewriter(Var init, Expr final) : init(init), final(final) {}

  using IRRewriter::visit;

  void visit(const VarExpr *op) {
    if (op->var == init) {
      expr = final;
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const AssignStmt *op) {
    Expr value = rewrite(op->value);
    if (op->var == init) {
      if (isa<VarExpr>(final)) {
        stmt = AssignStmt::make(to<VarExpr>(final)->var, value, op->cop);
      }
      else if (isa<FieldRead>(final)) {
        const auto fieldRead = to<FieldRead>(final);
        stmt = FieldWrite::make(fieldRead->elementOrSet, fieldRead->fieldName,
                                value, op->cop);
      }
      else if (isa<TensorRead>(final)) {
        const auto tensorRead = to<TensorRead>(final);
        stmt = TensorWrite::make(tensorRead->tensor, tensorRead->indices,
                                 value, op->cop);
      }
      else {
        not_supported_yet;
      }
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const CallStmt *op) {
    std::vector<Var> finalResults(op->results.size());
    for (size_t i = 0; i < op->results.size(); ++i) {
      finalResults[i] = (op->results[i] == init) ? to<VarExpr>(final)->var : 
                        op->results[i];
    }

    std::vector<Expr> actuals(op->actuals.size());
    for (size_t i = 0; i < op->actuals.size(); ++i) {
      actuals[i] = rewrite(op->actuals[i]);
    }

    stmt = CallStmt::make(finalResults, op->callee, actuals);
  }

  void visit(const Map *op) {
    std::vector<Var> vars(op->vars.size());
    for (size_t i = 0; i < op->vars.size(); ++i) {
      vars[i] = (op->vars[i] == init) ? to<VarExpr>(final)->var : op->vars[i];
    }

    Expr target = rewrite(op->target);
  
    std::vector<Expr> neighbors(op->neighbors.size());
    for (size_t i = 0; i < op->neighbors.size(); ++i) {
      neighbors[i] = rewrite(op->neighbors[i]);
    }
  
    Expr through;
    if (op->through.defined()) {
      through = rewrite(op->through);
    }
    
    std::vector<Expr> partial_actuals(op->partial_actuals.size());
    for (size_t i = 0; i < op->partial_actuals.size(); ++i) {
      partial_actuals[i] = rewrite(op->partial_actuals[i]);
    }
  
    stmt = Map::make(vars, op->function, partial_actuals, target,
                     neighbors, through, op->reduction);
  }

private:
  Var  init;
  Expr final;
};

Stmt replaceVar(Stmt stmt, Var init, Var final) {
  return VarReplaceRewriter(init, final).rewrite(stmt);
}

Func replaceVar(Func func, Var init, Var final) {
  return VarReplaceRewriter(init, final).rewrite(func);
}

Stmt replaceVarByExpr(Stmt stmt, Var init, Expr final) {
  return ReplaceVarByExprRewriter(init, final).rewrite(stmt);
}

}}  // namespace simit::ir
