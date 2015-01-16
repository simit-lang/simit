#include "flatten.h"

#include <string>
#include <vector>

#include "ir.h"
#include "ir_queries.h"
#include "ir_rewriter.h"
#include "ir_codegen.h"
#include "substitute.h"

using namespace std;

namespace simit {
namespace ir {

std::string tmpNameGen();
bool overlaps(const std::vector<IndexVar> &as, const std::vector<IndexVar> &bs);

/// Static namegen (hacky: fix later)
std::string tmpNameGen() {
  static int i = 0;
  return "tmp" + std::to_string(i++);
}

bool overlaps(const std::vector<IndexVar> &as, const std::vector<IndexVar> &bs){
  set<IndexVar> aset(as.begin(), as.end());
  for (auto &b : bs) {
    if (aset.find(b) != aset.end()) {
      return true;
    }
  }
  return false;
}

/// Flattens nested IndexExprs.
/// E.g. ({i,j} (({i} a{i}){i} * a{j})) -> ({i,j} (a{i} * a{j}))
class FlattenIndexExpressions : private IRRewriter {
public:
  Stmt flatten(Stmt stmt) {
    return rewrite(stmt);
  }

private:
  std::vector<Stmt> stmts;
  
  using IRRewriter::rewrite;
  using IRRewriter::visit;

  Expr rewrite(Expr e) {
    return IRRewriter::rewrite(e);
  }

  Stmt rewrite(Stmt s) {
    if (s.defined()) {
      s.accept(this);
      stmts.push_back(stmt);
      s = (stmts.size() > 0) ? Block::make(stmts) : stmt;
      stmts.clear();
    }
    else {
      s = Stmt();
    }
    expr = Expr();
    stmt = Stmt();
    return s;
  }

  Expr spill(Expr a) {  
    // If it is an index expression
    if (containsIndexedTensor(a)) {
      vector<IndexVar> afvars = getFreeVars(a);
      Expr spill = isa<IndexExpr>(a) ? a : IndexExpr::make(afvars, a);
      Var tmp(tmpNameGen(), spill.type());
      stmts.push_back(AssignStmt::make(tmp, spill));
      return IndexedTensor::make(VarExpr::make(tmp), afvars);
    }
    else {
      Expr spill = a;
      Var tmp(tmpNameGen(), spill.type());
      stmts.push_back(AssignStmt::make(tmp, spill));
      return VarExpr::make(tmp);
    }
  }

  std::pair<Expr,Expr> spillSubExpressions(Expr a, Expr b) {
    if (!isa<IndexedTensor>(a)) {
      a = spill(a);
    }
    if (!isa<IndexedTensor>(b)) {
      b = spill(b);
    }
    return pair<Expr,Expr>(a,b);
  }

  void visit(const Sub *op) {
    iassert(isScalar(op->a.type()));
    iassert(isScalar(op->b.type()));
    Expr a = rewrite(op->a);
    Expr b = rewrite(op->b);

    pair<Expr,Expr> ab = spillSubExpressions(a, b);
    expr = Sub::make(ab.first, ab.second);
  }

  // TODO: Add .* amd ./ too
  void visit(const Add *op) {
    iassert(isScalar(op->a.type()));
    iassert(isScalar(op->b.type()));

    Expr a = rewrite(op->a);
    Expr b = rewrite(op->b);

    pair<Expr,Expr> ab = spillSubExpressions(a, b);
    expr = Add::make(ab.first, ab.second);
  }

  void visit(const CallStmt *op) {
    vector<Expr> actuals;
    bool changed = false;
    for (Expr actual : op->actuals) {
      actual = rewrite(actual);

      // Spill non-var higher-order tensor-typed expressions in function calls
      Type atype = actual.type();
      if ((atype.isTensor() && !isScalar(atype) && !isa<VarExpr>(actual)) ) {
        actual = spill(actual);
        changed = true;
      }
      actuals.push_back(actual);
    }
    stmt = changed ? CallStmt::make(op->results, op->callee, actuals) : op;
  }

  void visit(const IndexedTensor *op) {
    // IndexExprs that are nested inside another IndexExpr must necessarily
    // produce a tensor and therefore be indexed through an IndexedTensor expr.
    Expr tensor = rewrite(op->tensor);

    if (isa<IndexExpr>(tensor)) {
      const IndexExpr *indexExpr = to<IndexExpr>(tensor);
      iassert(indexExpr->resultVars.size() == op->indexVars.size());

      map<IndexVar,IndexVar> substitutions;
      for (size_t i=0; i < indexExpr->resultVars.size(); ++i) {
        pair<IndexVar,IndexVar> sub(indexExpr->resultVars[i], op->indexVars[i]);
        substitutions.insert(sub);
      }
      expr = substitute(substitutions, indexExpr->value);
    }
    else {
      IRRewriter::visit(op);
    }
  }
};

Stmt flattenIndexExpressions(Stmt stmt) {
  return FlattenIndexExpressions().flatten(stmt);
}

Func flattenIndexExpressions(Func func) {
  Stmt body = flattenIndexExpressions(func.getBody());
  func = Func(func, body);
  func = insertVarDecls(func);
  return func;
}

}} // namespace simit::ir
