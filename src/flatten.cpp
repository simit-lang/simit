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
#include "macros.h"

using namespace std;

namespace simit {
namespace ir {

std::string tmpNameGen();
bool overlaps(const std::vector<IndexVar> &as, const std::vector<IndexVar> &bs);

/// Static namegen (hacky: fix later)
std::string tmpNameGen() {
  static int i = 0;
  return INTERNAL_PREFIX("spilledTmp") + std::to_string(i++);
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

inline bool isMatrixTransposeElwise(const IndexExpr* iexpr) {
  if (iexpr->resultVars.size() != 2 || isa<IndexedTensor>(iexpr->value)) {
    return false;
  }

  bool result = true;
  match(iexpr->value,
    std::function<void(const IndexedTensor*)>([&](const IndexedTensor* op) {
      if (result == false) {
        return;
      }
      if (op->indexVars.size() != iexpr->resultVars.size() ||
          !std::equal(op->indexVars.begin(), op->indexVars.end(),
                      iexpr->resultVars.rbegin())) {
        result = false;
        return;
      }
    })
  );

  return result;
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
        spill = builder.unaryElwiseExpr(IRBuilder::Copy, spill);
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
      set<IndexVar> ivs;
      auto aivs = getReductionVars(a);
      auto bivs = getReductionVars(b);
      ivs.insert(aivs.begin(), aivs.end());
      ivs.insert(bivs.begin(), bivs.end());
      if (ivs.size() < 2) {
        return pair<Expr,Expr>(a,b);
      }
    }

    if (!isa<IndexedTensor>(a)) {
      a = spill(a);
    }
    if (!isa<IndexedTensor>(b)) {
      b = spill(b);
    }

    // Handle operations on a matrix and its transpose by spilling the 
    // transposed matrix.
    iassert(!isa<Literal>(a) && !isa<Literal>(b));
    const auto aTensor = to<IndexedTensor>(a);
    const auto bTensor = to<IndexedTensor>(b);
    if (aTensor->indexVars.size() == 2 && bTensor->indexVars.size() == 2 && 
        aTensor->indexVars[0] == bTensor->indexVars[1] && 
        aTensor->indexVars[1] == bTensor->indexVars[0]) {
      const auto transposedB = IRBuilder().transposedMatrix(bTensor->tensor);
      const auto spilledB = spill(transposedB);
      
      const auto bTensor = to<IndexedTensor>(spilledB);
      b = IndexedTensor::make(bTensor->tensor, aTensor->indexVars);
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

      // Spill non-var tensor-typed expressions in function calls
      Type atype = actual.type();
      if ((atype.isTensor() && !isa<VarExpr>(actual)) ) {
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

  void visit(const IndexExpr *op) {
    IRRewriter::visit(op);

    // If expression corresponds to transpose of a matrix element-wise 
    // operation, spill result of element-wise operation.
    const auto iexpr = to<IndexExpr>(expr);
    if (isMatrixTransposeElwise(iexpr)) {
      expr = IndexExpr::make(iexpr->resultVars, spill(iexpr->value)); 
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
