#include "lower_index_expressions.h"

#include "ir.h"
#include "ir_rewriter.h"
#include "ir_codegen.h"

#include "lower_indexexprs.h"
#include "lower_scatter_workspace.h"

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

inline bool isBinaryScale(const IndexExpr* iexpr) {
  if (!isBinaryExpression(iexpr)) {
    return false;
  }

  bool result = false;
  // Binary expression with at least one scalar variable
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
  return false;
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
        stmt = lowerIndexStatement(op, *storage);
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
      else if (isBinaryScale(iexpr)) {
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
        case Unknown:
          break;
        case DenseResult:
        case MatrixScale:
        case MatrixElwiseWithSameStructure:
          stmt = lowerIndexStatement(op, *storage);
          break;
        case MatrixElwise:
          stmt = lowerScatterWorkspace(op->var, iexpr, &environment, storage);
          break;
        case MatrixMultiply:
          break;
      }

      iassert(stmt.defined());
    }

    void visit(const FieldWrite *op) {
      if (!isa<IndexExpr>(op->value) && op->cop == CompoundOperator::None) {
        IRRewriter::visit(op);
        return;
      }
      stmt = lowerIndexStatement(op, *storage);
    }

    void visit(const TensorWrite *op) {
      if (!isa<IndexExpr>(op->value) && op->cop == CompoundOperator::None) {
        IRRewriter::visit(op);
        return;
      }
      stmt = lowerIndexStatement(op, *storage);
    }

    void visit(const IndexExpr *op) {
      iassert_scalar(Expr(op));
      expr = rewrite(op->value);
    }

    void visit(const IndexedTensor *op) {
      expr = op->tensor;
    }
  };
  func = LowerIndexExpressionsRewriter().lower(func);
  func = insertVarDecls(func);

  return func;
}

}}
