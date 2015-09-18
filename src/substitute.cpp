#include "substitute.h"

using namespace std;

#include "ir.h"
#include "ir_rewriter.h"
#include "util/collections.h"

namespace simit {
namespace ir {

class Substitute : public IRRewriter {
public:
  Substitute(Expr oldExpr, Expr newExpr) {
    substitutions.insert(pair<Expr,Expr>(oldExpr, newExpr));
  }

  Substitute(map<Expr,Expr> substitutions) : substitutions(substitutions) {
    for (auto& sub : substitutions) {
      match(sub.first,
        std::function<void(const VarExpr*)>([&](const VarExpr* op) {
          varSubstitutions.insert({op->var, sub.second});
        })
      );
    }
  }
  
  using IRRewriter::rewrite;

  Stmt rewrite(Stmt stmt) {
    return IRRewriter::rewrite(stmt);
  }

  Expr rewrite(Expr expr) {
    if (util::contains(substitutions, expr)) {
      return substitutions.at(expr);
    }
    else {
      return IRRewriter::rewrite(expr);
    }
  }

private:
  map<Expr,Expr> substitutions;
  map<Var,Expr> varSubstitutions;

  using IRRewriter::visit;

  void visit(const VarExpr *op) {
    if (util::contains(varSubstitutions, op->var)) {
      expr = varSubstitutions.at(op->var);
    }
    else {
      expr = op;
    }
  }
};

Expr substitute(std::map<Expr,Expr> substitutions, Expr expr) {
  return Substitute(substitutions).rewrite(expr);
}

Stmt substitute(std::map<Expr,Expr> substitutions, Stmt stmt) {
  return Substitute(substitutions).rewrite(stmt);
}

Expr substitute(Expr oldExpr, Expr newExpr, Expr expr) {
  return Substitute(oldExpr, newExpr).rewrite(expr);
}

Stmt substitute(Expr oldExpr, Expr newExpr, Stmt stmt) {
  return Substitute(oldExpr, newExpr).rewrite(stmt);
}


class SubstituteIndexVars : public IRRewriter {
public:
  SubstituteIndexVars(map<IndexVar,IndexVar> subs) : subs(subs) {}

private:
  map<IndexVar,IndexVar> subs;

  using IRRewriter::visit;

  void visit(const IndexedTensor *op) {
    vector<IndexVar> indexVars;
    for (auto &iv : op->indexVars) {
      indexVars.push_back((subs.find(iv)!=subs.end()) ? subs.at(iv) : iv);
    }
    expr = IndexedTensor::make(op->tensor, indexVars);
  }
};

Expr substitute(std::map<IndexVar,IndexVar> substitutions, Expr expr) {
  return SubstituteIndexVars(substitutions).rewrite(expr);
}

Stmt substitute(std::map<IndexVar,IndexVar> substitutions, Stmt stmt) {
  return SubstituteIndexVars(substitutions).rewrite(stmt);
}

}}
