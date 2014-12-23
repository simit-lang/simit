#include "ir_codegen.h"

#include <vector>

#include "ir_rewriter.h"
#include "ir_queries.h"
#include "indexvar.h"

using namespace std;

namespace simit {
namespace ir {

/// Turns tensor writes into compound assignments (e.g. +=, *=)
/// \todo Generalize to include Assignments, FieldWrite, TupleWrite
class MakeCompound : public IRRewriter {
public:
  MakeCompound(CompoundOperator compoundOperator)
      : compoundOperator(compoundOperator) {}

  Stmt rewrite(Stmt stmt) {
    return IRRewriter::rewrite(stmt);
  }

private:
  CompoundOperator compoundOperator;
  Expr lhsExpr;

  Expr rewrite(Expr e) {
    iassert(lhsExpr.defined());
    if (e.defined()) {
      if (!isScalar(e.type())) {
        e = IRRewriter::rewrite(e);
      }
      else {
        switch (compoundOperator.kind) {
          case CompoundOperator::Add: {
            e = Add::make(lhsExpr, e);
            break;
          }
        }
      }
    }
    else {
      e = Expr();
    }
    expr = Expr();
    stmt = Stmt();
    return e;
  }

  void visit(const TensorWrite *op) {
    lhsExpr = TensorRead::make(op->tensor, op->indices);
    vector<IndexVar> indexVars = getFreeVars(op->value);
    if (indexVars.size()) {
      lhsExpr = IndexedTensor::make(lhsExpr, indexVars);
    }
    Expr value = rewrite(op->value);
    stmt = TensorWrite::make(op->tensor, op->indices, value);
  }
};


Stmt makeCompound(Stmt stmt, CompoundOperator cop) {
  return MakeCompound(cop).rewrite(stmt);
}

}}
