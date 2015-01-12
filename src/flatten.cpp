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

  std::pair<Expr,Expr> materializeInterferringExprs(Expr a, Expr b) {
    std::vector<IndexVar> arvars = getReductionVars(a);
    std::vector<IndexVar> brvars = getReductionVars(b);
    if (arvars.size() > 0 || brvars.size() > 0) {
      std::vector<IndexVar> afvars = getFreeVars(a);
      std::vector<IndexDomain> adims;
      for (auto &afvar : afvars) {
        adims.push_back(afvar.getDomain());
      }
      Type atype = TensorType::make(a.type().toTensor()->componentType, adims);
      Var atmp(tmpNameGen(), atype);
      Expr aiexpr = IndexExpr::make(afvars, a);
      Stmt astmt = AssignStmt::make(atmp, aiexpr);
      stmts.push_back(astmt);

      vector<IndexVar> bfvars = getFreeVars(b);
      vector<IndexDomain> bdims;
      for (auto &bfvar : bfvars) {
        bdims.push_back(bfvar.getDomain());
      }
      Type btype = TensorType::make(a.type().toTensor()->componentType, bdims);
      Var btmp(tmpNameGen(), btype);
      Expr biexpr = IndexExpr::make(bfvars, b);
      Stmt bstmt = AssignStmt::make(btmp, biexpr);
      stmts.push_back(bstmt);

      a = IndexedTensor::make(VarExpr::make(atmp), afvars);
      b = IndexedTensor::make(VarExpr::make(btmp), bfvars);
    }
    return pair<Expr,Expr>(a,b);
  }

  void visit(const Sub *op) {
    iassert(isScalar(op->a.type()) || isa<IndexedTensor>(op->a));
    iassert(isScalar(op->b.type()) || isa<IndexedTensor>(op->b));
    Expr a = rewrite(op->a);
    Expr b = rewrite(op->b);

    pair<Expr,Expr> ab = materializeInterferringExprs(a, b);
    expr = Sub::make(ab.first, ab.second);
  }

  // TODO: Add .* amd ./ too
  void visit(const Add *op) {
    iassert(isScalar(op->a.type()) || isa<IndexedTensor>(op->a));
    iassert(isScalar(op->b.type()) || isa<IndexedTensor>(op->b));

    Expr a = rewrite(op->a);
    Expr b = rewrite(op->b);

    pair<Expr,Expr> ab = materializeInterferringExprs(a, b);
    expr = Add::make(ab.first, ab.second);
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
