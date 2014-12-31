#include "ir_codegen.h"

#include <vector>

#include "ir_rewriter.h"
#include "ir_queries.h"
#include "ir_builder.h"
#include "indexvar.h"

using namespace std;

namespace simit {
namespace ir {

Stmt makeCompound(Stmt stmt, CompoundOperator cop) {
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
        if (!e.type().isTensor()) {
          e = IRRewriter::rewrite(e);
        }
        else {
          iassert_types_equal(lhsExpr, e);

          if (isScalar(lhsExpr.type())) {
            switch (compoundOperator.kind) {
              case CompoundOperator::Add: {
                e = Add::make(lhsExpr,e);
                break;
              }
            }
          }
          else {
            IRBuilder::BinaryOperator binop;
            switch (compoundOperator.kind) {
              case CompoundOperator::Add: {
                binop = IRBuilder::Add;
                break;
              }
            }
            IRBuilder builder;
            e = builder.binaryElwiseExpr(lhsExpr, binop, e);
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

  return MakeCompound(cop).rewrite(stmt);
}


Stmt initializeLhsToZero(Stmt stmt) {
  class ReplaceRhsWithZero : public IRRewriter {
    void visit(const AssignStmt *op) {
      stmt = AssignStmt::make(op->var, 0.0);
    }

    void visit(const FieldWrite *op) {
      stmt = FieldWrite::make(op->elementOrSet, op->fieldName, 0.0);
    }

    void visit(const TensorWrite *op) {
      stmt = TensorWrite::make(op->tensor, op->indices, 0.0);
    }
  };

  return ReplaceRhsWithZero().rewrite(stmt);
}

}}
