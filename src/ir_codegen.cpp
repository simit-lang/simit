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
    MakeCompound(CompoundOperator compound) : compound(compound) {}

  private:
    CompoundOperator compound;

    void visit(const TensorWrite *op) {
      Expr lhsRead = TensorRead::make(op->tensor, op->indices);
      Expr value = rewrite(op->value);

      iassert_types_equal(lhsRead, value);

      if (isScalar(lhsRead.type())) {
        switch (compound.kind) {
          case CompoundOperator::Add: {
            value = Add::make(lhsRead, value);
            break;
          }
        }
      }
      else {
        IRBuilder::BinaryOperator binop;
        switch (compound.kind) {
          case CompoundOperator::Add: {
            binop = IRBuilder::Add;
            break;
          }
        }
        IRBuilder builder;
        value = builder.binaryElwiseExpr(lhsRead, binop, value);
      }


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
