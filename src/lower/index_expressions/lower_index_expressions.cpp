#include "lower_index_expressions.h"

#include "ir.h"
#include "ir_rewriter.h"
#include "ir_transforms.h"

#include "lower_indexexprs.h"
#include "lower_scatter_workspace.h"
#include "lower_matrix_multiply.h"

namespace simit {
namespace ir {

inline unsigned getExperssionArity(const IndexExpr* iexpr) {
  // The expression arity is the number of binary expressions plus one
  unsigned expressionArity = 1;
  match(iexpr->value,
    std::function<void(const BinaryExpr*)>([&](const BinaryExpr* op) {
      ++expressionArity;
    })
  );
  return expressionArity;
}

inline bool isBinaryExpression(const IndexExpr* iexpr) {
  return getExperssionArity(iexpr) == 2;
}

inline bool isScale(const IndexExpr* iexpr) {

  bool result = false;
  // Expression with at least one scalar variable
  match(iexpr->value,
    std::function<void(const VarExpr*)>([&](const VarExpr* op) {
      if (isScalar(op->type)) {
        result = true;
      }
    }),
    std::function<void(const Literal*)>([&](const Literal* op) {
      if (isScalar(op->type)) {
        result = true;
      }
    }),
    std::function<void(const Load*)>([&](const Load* op) {
      if (isScalar(op->type)) {
        result = true;
      }
    }),
    std::function<void(const FieldRead*)>([&](const FieldRead* op) {
      if (isScalar(op->type)) {
        result = true;
      }
    })
  );

  return result;
}

inline bool doesOperandsHaveSameStructure(const IndexExpr* iexpr,
                                          const Storage& storage) {
  bool result = true;
  pe::PathExpression pexpr;
  match(iexpr->value,
    std::function<void(const VarExpr*)>([&](const VarExpr* op) {
      iassert(storage.hasStorage(op->var));
      if (!pexpr.defined()) {
        pexpr = storage.getStorage(op->var).getPathExpression();
      }
      else if (pexpr != storage.getStorage(op->var).getPathExpression()) {
        result = false;
      }
    })
  );

  return result;
}

inline bool isElwise(const IndexExpr* iexpr) {
  bool result = true;
  match(iexpr->value,
    std::function<void(const IndexedTensor*)>([&](const IndexedTensor* op) {
      if (result == false) {
        return;
      }
      if (op->indexVars.size() != iexpr->resultVars.size() ||
          !std::equal(op->indexVars.begin(), op->indexVars.end(),
                      iexpr->resultVars.begin())) {
        result = false;
        return;
      }
    })
  );

  return result;
}

inline bool isGemm(const IndexExpr* iexpr) {
  // Very specific index expression form: (i,j B(i,+k)*C(+k,j))
  // "First" matrix is defined as the one with its first index var
  // as a free variable. The "second" matrix is defined as the reverse.
  // The remaining variable should be summed.
  if (iexpr->resultVars.size() != 2) {
    return false;
  }
  bool result = true;
  bool foundFirst = false;
  bool foundSecond = false;
  match(iexpr->value,
    std::function<void(const IndexedTensor*)>([&](const IndexedTensor* op) {
      if (result == false) {
        return;
      }
      if (op->indexVars.size() != 2) {
        result = false;
        return;
      }
      // Check for first matrix
      // TODO: We only allow non-transposed multiplication
      if (op->indexVars[0].isFreeVar() &&
          op->indexVars[0] == iexpr->resultVars[0] &&
          op->indexVars[1].isReductionVar() &&
          op->indexVars[1].getOperator() == ReductionOperator::Sum) {
        // We're doing a transpose multiply, unhandled right now
        if (foundFirst) {
          result = false;
          return;
        }
        foundFirst = true;
      }
      // Check for second matrix
      else if (op->indexVars[1].isFreeVar() &&
               op->indexVars[1] == iexpr->resultVars[1] &&
               op->indexVars[0].isReductionVar() &&
               op->indexVars[0].getOperator() == ReductionOperator::Sum) {
        // Transpose multiply is unhandled right now
        if (foundSecond) {
          result = false;
          return;
        }
        foundSecond = true;
      }
      // Tensor term is not of matrix multiply form
      else {
        result = false;
        return;
      }
    })
  );
  result = result && foundFirst && foundSecond;

  return result;
}

Func lowerIndexExpressions(Func func) {
  class LowerIndexExpressionsRewriter : private IRRewriter {
  public:
    Func lower(Func func) {
      storage = &func.getStorage();
      environment = func.getEnvironment();
      return this->rewrite(func);
    }

  private:
    Storage *storage;
    Environment environment;
    
    using IRRewriter::visit;

    void visit(const Func* f) {
      Stmt body = rewrite(f->getBody());
      if (body != f->getBody()) {
        func = Func(f->getName(), f->getArguments(), f->getResults(),
                    body, environment, f->getKind());
        func.setStorage(*storage);
      }
      else {
        func = *f;
      }
    }

    void visit(const AssignStmt *op) {
      if (!isa<IndexExpr>(op->value) && op->cop == CompoundOperator::None) {
        IRRewriter::visit(op);
        return;
      }
      if (!isa<IndexExpr>(op->value)) {
        stmt = lowerIndexStatement(op, &environment, *storage);
        return;
      }

      const IndexExpr* iexpr = to<IndexExpr>(op->value);

      // Dispatch the index expression lowering to the correct lowering pass.
      enum Kind {Unknown, DenseResult, MatrixScale,
                 MatrixElwiseWithSameStructure, MatrixElwise, MatrixMultiply};
      Kind kind = Unknown;

      iassert(iexpr->type.isTensor());
      const Var& var = op->var;
      const TensorType* type = iexpr->type.toTensor();

      if (type->order()==0 || type->order()==1 ||
          storage->getStorage(var).isDense()) {
        kind = DenseResult;
      }
      else if (isScale(iexpr)) {
        kind = MatrixScale;
      }
      else if (isElwise(iexpr)) {
        kind = doesOperandsHaveSameStructure(iexpr, *storage)
               ? MatrixElwiseWithSameStructure
               : MatrixElwise;
      }
      else if (isGemm(iexpr)) {
        kind = MatrixMultiply;
      }

      iassert(kind != Unknown)
          << "Index expression lowering does not know how to lower: "
          << Stmt(op);

      switch (kind) {
        case DenseResult:
        case MatrixScale:
        case MatrixElwiseWithSameStructure:
          stmt = lowerIndexStatement(op, &environment, *storage);
          break;
        case MatrixElwise:
          stmt = lowerScatterWorkspace(op->var, iexpr, &environment, storage);
          break;
        case MatrixMultiply:
          stmt = lowerMatrixMultiply(op->var, iexpr, &environment, storage);
          break;
        case Unknown:
          ierror << "unknown matrix expression";
          break;
      }
      iassert(stmt.defined());
    }

    void visit(const FieldWrite *op) {
      if (!isa<IndexExpr>(op->value) && op->cop == CompoundOperator::None) {
        IRRewriter::visit(op);
        return;
      }
      stmt = lowerIndexStatement(op, &environment, *storage);
    }

    void visit(const TensorWrite *op) {
      if (!isa<IndexExpr>(op->value) && op->cop == CompoundOperator::None) {
        IRRewriter::visit(op);
        return;
      }
      stmt = lowerIndexStatement(op, &environment, *storage);
    }

    void visit(const IndexExpr *op) {
      iassert_scalar(Expr(op));
      expr = rewrite(op->value);
    }
  };
  func = LowerIndexExpressionsRewriter().lower(func);
  func = insertVarDecls(func);

  return func;
}

}}
