#include "substitute.h"

using namespace std;

#include "ir.h"
#include "ir_rewriter.h"

namespace simit {
namespace ir {

class Substitute : public IRRewriter {
public:
  Substitute(Expr oldExpr, Expr newExpr) {
    substitutions.insert(pair<Expr,Expr>(oldExpr, newExpr));
  }

  Substitute(map<Expr,Expr> substitutions) : substitutions(substitutions) {}

  Stmt mutate(Stmt stmt) {
    return IRRewriter::mutate(stmt);
  }

  Expr mutate(Expr expr) {
    if (substitutions.find(expr) != substitutions.end()) {
      return substitutions.at(expr);
    }
    else {
      return IRRewriter::mutate(expr);
    }
  }

private:
  map<Expr,Expr> substitutions;

  void visit(const VarExpr *op) {
    expr = op;
  }
};

Expr substitute(std::map<Expr,Expr> substitutions, Expr expr) {
  return Substitute(substitutions).mutate(expr);
}

Stmt substitute(std::map<Expr,Expr> substitutions, Stmt stmt) {
  return Substitute(substitutions).mutate(stmt);
}

Expr substitute(Expr oldExpr, Expr newExpr, Expr expr) {
  return Substitute(oldExpr, newExpr).mutate(expr);
}

Stmt substitute(Expr oldExpr, Expr newExpr, Stmt stmt) {
  return Substitute(oldExpr, newExpr).mutate(stmt);
}


class SubstituteIndexVars : public IRRewriter {
public:
  SubstituteIndexVars(map<IndexVar,IndexVar> subs) : subs(subs) {}

private:
  map<IndexVar,IndexVar> subs;

  void visit(const IndexedTensor *op) {
    vector<IndexVar> indexVars;
    for (auto &iv : op->indexVars) {
      indexVars.push_back((subs.find(iv)!=subs.end()) ? subs.at(iv) : iv);
    }
    expr = IndexedTensor::make(op->tensor, indexVars);
  }
};

Expr substitute(std::map<IndexVar,IndexVar> substitutions, Expr expr) {
  return SubstituteIndexVars(substitutions).mutate(expr);
}

Stmt substitute(std::map<IndexVar,IndexVar> substitutions, Stmt stmt) {
  return SubstituteIndexVars(substitutions).mutate(stmt);
}

}}
