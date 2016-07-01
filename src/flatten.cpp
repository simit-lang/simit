#include "flatten.h"

#include <string>
#include <vector>

#include "ir.h"
#include "intrinsics.h"
#include "ir_queries.h"
#include "ir_rewriter.h"
#include "ir_transforms.h"
#include "substitute.h"
#include "ir_builder.h"

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
  IRBuilder builder;
  
  using IRRewriter::visit;

  Expr spill(Expr a) {  
    // If it is an index expression
    if (containsIndexedTensor(a)) {
      vector<IndexVar> afvars = getFreeVars(a);
      Expr spill = isa<IndexExpr>(a) ? a : IndexExpr::make(afvars, a);
      Var tmp(tmpNameGen(), spill.type());
      IRRewriter::spill(AssignStmt::make(tmp, spill));
      return IndexedTensor::make(VarExpr::make(tmp), afvars);
    }
    else if (!isScalar(a.type())) {
      Expr spill = a;
      Var tmp(tmpNameGen(), spill.type());
      // if we're spilling a tensor, we have to spill the whole thing.
      // TODO: We should let these actually go into the backend as
      // assign statements and deal with them there
      if (spill.type().isTensor()) {
        spill = builder.unaryElwiseExpr(IRBuilder::None, spill);
      }
      IRRewriter::spill(AssignStmt::make(tmp, spill));
      return VarExpr::make(tmp);
    }
    else {
      // We don't need to spill scalars
      return a;
    }
  }

  std::pair<Expr,Expr> spillAsNeeded(Expr a, Expr b) {
    class IsAnyInputSparseVisitor : public IRQuery {
      using IRQuery::visit;
      void visit(const IndexedTensor *op) {
        if (op->tensor.type().toTensor()->isSparse()) {
          result = true;
        }
      }
    };

    if (countIndexVars(a) == countIndexVars(b) &&
        !IsAnyInputSparseVisitor().query(a) &&
        !IsAnyInputSparseVisitor().query(b)) {
      return pair<Expr,Expr>(a,b);
    }

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

    pair<Expr,Expr> ab = spillAsNeeded(a, b);
    expr = Sub::make(ab.first, ab.second);
  }

  void visit(const Add *op) {
    iassert(isScalar(op->a.type()));
    iassert(isScalar(op->b.type()));

    Expr a = rewrite(op->a);
    Expr b = rewrite(op->b);

    pair<Expr,Expr> ab = spillAsNeeded(a, b);
    expr = Add::make(ab.first, ab.second);
  }
  
  void visit(const Mul *op) {
    iassert(isScalar(op->a.type()));
    iassert(isScalar(op->b.type()));

    Expr a = rewrite(op->a);
    Expr b = rewrite(op->b);

    pair<Expr,Expr> ab = spillAsNeeded(a, b);
    expr = Mul::make(ab.first, ab.second);
  }

  void visit(const Div *op) {
    iassert(isScalar(op->a.type()));
    iassert(isScalar(op->b.type()));

    Expr a = rewrite(op->a);
    Expr b = rewrite(op->b);

    pair<Expr,Expr> ab = spillAsNeeded(a, b);
    expr = Div::make(ab.first, ab.second);
  }

  void visit(const CallStmt *op) {
    vector<Expr> actuals;
    bool changed = false;
    for (Expr actual : op->actuals) {
      Expr newActual = rewrite(actual);
      if (newActual != actual) {
        actual = newActual;
        changed = true;
      }

      // Spill non-var higher-order tensor-typed expressions in function calls
      Type atype = actual.type();
      if ((atype.isTensor() && !isScalar(atype) && !isa<VarExpr>(actual)) ) {
        actual = spill(actual);
        if (isa<IndexedTensor>(actual)) {
          actual = to<IndexedTensor>(actual)->tensor;
        }
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

      bool containsReduction = false;
      map<IndexVar,IndexVar> substitutions;
      for (size_t i=0; i < indexExpr->resultVars.size(); ++i) {
        pair<IndexVar,IndexVar> sub(indexExpr->resultVars[i], op->indexVars[i]);
        substitutions.insert(sub);

        if (op->indexVars[i].isReductionVar()) {
          containsReduction = true;
        }
      }

      if (containsReduction) {
        Var tmp(tmpNameGen(), tensor.type());
        IRRewriter::spill(AssignStmt::make(tmp, tensor));
        expr = IndexedTensor::make(VarExpr::make(tmp), op->indexVars);
      } else {
        expr = substitute(substitutions, indexExpr->value);
      }
    }
    else {
      IRRewriter::visit(op);
    }
  }
};

class NormAndDotRewriter : public ir::IRRewriter {
  using IRRewriter::visit;
  IRBuilder builder;
  void visit(const ir::CallStmt *op) {
    if (op->callee.getName() == "norm") {
      iassert(op->actuals.size() == 1);
      iassert(op->results.size() == 1);
      uassert(op->actuals[0].type().isTensor());
      
      auto dot = builder.innerProduct(op->actuals[0], op->actuals[0]);
      auto tmpvar = builder.temporary(op->results[0].getType(), "normrewrite");
      stmt = AssignStmt::make(tmpvar, dot);
      stmt = Block::make(stmt, CallStmt::make(op->results, intrinsics::sqrt(),
                                              {VarExpr::make(tmpvar)}));
    }
    else if (op->callee.getName() == "dot") {
      iassert(op->actuals.size() == 2);
      iassert(op->results.size() == 1);

      auto dot = builder.innerProduct(op->actuals[0], op->actuals[1]);
      stmt = AssignStmt::make(op->results[0], dot);
    }
    else {
      stmt = op;
    }
  }
};


Stmt flattenIndexExpressions(Stmt stmt) {
  return FlattenIndexExpressions().flatten(stmt);
}

Func flattenIndexExpressions(Func func) {
  Stmt body = flattenIndexExpressions(NormAndDotRewriter().rewrite(func.getBody()));
  func = Func(func, body);
  func = insertVarDecls(func);
  return func;
}

}} // namespace simit::ir
