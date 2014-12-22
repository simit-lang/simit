#include "lower.h"

#include "ir.h"
#include "ir_queries.h"
#include "ir_rewriter.h"
#include "ir_codegen.h"
#include "ir_printer.h"
#include "sig.h"

using namespace std;

namespace simit {
namespace ir {


/// Specializes a statement containing an index expression to compute the
/// value/block for the loop iteration given by the map from index variables
/// to loop variables.
Stmt specialize(Stmt stmt, const LoopVars &loopVars) {
  class SpecializeIndexExpression : private IRRewriter {
  public:
    SpecializeIndexExpression(const LoopVars &loopVars) : loopVars(loopVars) {}

    /// Specialize 'stmt'.
    Stmt specialize(Stmt stmt) {
      iassert(isFlattened(stmt))
          << "Index expressions must be flattened before specializing";
      return this->rewrite(stmt);
    }

  private:
    /// Loop variables used to compute index (created from the SIG)
    LoopVars const& loopVars;

    Stmt tensorWrite(Expr tensor, vector<IndexVar> indexVars, Expr value){
      std::vector<Expr> indexExprs;
      for (const IndexVar &iv : indexVars) {
        indexExprs.push_back(loopVars.getLoopVar(iv).getVar());
      }
      return TensorWrite::make(tensor, indexExprs, value);
    }

    void visit(const AssignStmt *op) {
      iassert(isa<IndexExpr>(op->value))<<"Can only specialize IndexExpr stmts";
      const IndexExpr *indexExpr = to<IndexExpr>(op->value);

      Var var = op->var;
      Expr value = rewrite(indexExpr);

      if (isScalar(op->value.type())) {
        stmt = AssignStmt::make(var, value);
      }
      else {
        stmt = tensorWrite(var, indexExpr->resultVars, value);
      }
    }

    void visit(const FieldWrite *op) {
      iassert(isa<IndexExpr>(op->value))<<"Can only specialize IndexExpr stmts";
      const IndexExpr *indexExpr = to<IndexExpr>(op->value);

      Expr elementOrSet = rewrite(op->elementOrSet);
      std::string fieldName = op->fieldName;
      Expr value = rewrite(indexExpr);

      if (isScalar(op->value.type())) {
        stmt = FieldWrite::make(elementOrSet, fieldName, value);
      }
      else {
        Expr field = FieldRead::make(elementOrSet, fieldName);
        stmt = tensorWrite(field, indexExpr->resultVars, value);
      }
    }

    void visit(const TensorWrite *op) {
      iassert(isa<IndexExpr>(op->value))<<"Can only specialize IndexExpr stmts";
      const IndexExpr *indexExpr = to<IndexExpr>(op->value);

      Expr value = rewrite(op->value);

      if (isScalar(op->value.type())) {
        stmt = TensorWrite::make(op->tensor, op->indices, value);
      }
      else {
        Expr tensor = TensorRead::make(op->tensor, op->indices);
        stmt = tensorWrite(tensor, indexExpr->resultVars, value);
      }
    }

    /// Replace indexed tensors with tensor reads
    void visit(const IndexedTensor *op) {
      iassert(!isa<IndexExpr>(op->tensor))
          << "index expressions should have been flattened by now";

      if (op->indexVars.size() == 0) {
        expr = op->tensor;
      }
      else {
        std::vector<Expr> indices;
        for (const IndexVar &indexVar : op->indexVars) {
          Expr varExpr = loopVars.getLoopVar(indexVar).getVar();
          indices.push_back(varExpr);
        }
        expr = TensorRead::make(op->tensor, indices);

        // If the tensor expression was a non-scalar tensor, then ...
        if (expr.type().toTensor()->order() > 0) {
          not_supported_yet; // See in old code
        }
      }
    }

    /// Removes index expression
    void visit(const IndexExpr *op) {
      expr = rewrite(op->value);
    }
  };

  return SpecializeIndexExpression(loopVars).specialize(stmt);
}

/// Rewrite 'loopNest' to reduce the result of the 'kernel' into a temporary
/// variable using the 'reductionOperator'.
Stmt reduce(Stmt loopNest, Stmt kernel, ReductionOperator reductionOperator) {
  class ReduceRewriter : public IRRewriter {
  public:
    ReduceRewriter(Stmt rstmt, ReductionOperator rop) : rstmt(rstmt), rop(rop){}

    Var getTmpVar() {return tmpVar;}

    /// Retrieve a statement that writes the tmp variable to the original
    /// location of the rewritten statement.  If result is !defined then the
    /// reduction variable does not ned to be written back.
    Stmt getTmpWriteStmt() {return tmpWriteStmt;}

  private:
    Stmt rstmt;
    ReductionOperator rop;

    Var tmpVar;
    Stmt tmpWriteStmt;

    void visit(const AssignStmt *op) {
      if (op == rstmt) {
        iassert(isScalar(op->value.type()))
        << "assignment non-scalars should have been lowered by now";
        switch (rop.getKind()) {
          case ReductionOperator::Sum: {
            Expr varExpr = VarExpr::make(op->var);
            tmpVar = op->var;
            stmt = AssignStmt::make(op->var, Add::make(varExpr, op->value));
            break;
          }
          case ReductionOperator::Undefined:
            ierror;
            break;
        }
      }
      else {
        stmt = op;
      }
    }

    void visit(const TensorWrite *op) {
      if (op == rstmt) {
        Expr tensor = op->tensor;
        std::vector<Expr> indices = op->indices;

        iassert(tensor.type().isTensor());
        switch (rop.getKind()) {
          case ReductionOperator::Sum: {
            ScalarType componentType = tensor.type().toTensor()->componentType;
            string tmpVarName = getReductionTmpName(op);

            tmpVar = Var(tmpVarName, TensorType::make(componentType));
            stmt = AssignStmt::make(tmpVar, Add::make(tmpVar, op->value));
            break;
          }
          case ReductionOperator::Undefined:
            ierror;
            break;
        }

        tmpWriteStmt = TensorWrite::make(tensor, indices, VarExpr::make(tmpVar));
        if (isa<TensorRead>(tensor)) {
          tmpWriteStmt = makeCompound(tmpWriteStmt, CompoundOperator::Add);
        }
      }
    }

    std::string getReductionTmpName(const TensorWrite *tensorWrite) {
      class GetReductionTmpName : public IRVisitor {
      public:
        string get(const TensorWrite *op) {
          op->tensor.accept(this);
          for (auto &index : op->indices) {
            index.accept(this);
          }
          return name;
        }

      private:
        std::string name;
        void visit(const VarExpr *op) {
          IRVisitor::visit(op);
          name += op->var.getName();
        }
      };

      return GetReductionTmpName().get(tensorWrite);
    }
  };

  ReduceRewriter reduceRewriter(kernel, reductionOperator);
  loopNest = reduceRewriter.rewrite(loopNest);

  Var tmpVar = reduceRewriter.getTmpVar();
  Stmt alloc = AssignStmt::make(tmpVar, Literal::make(tmpVar.getType(), {0}));
  Stmt tmpWriteStmt = reduceRewriter.getTmpWriteStmt();
  if (tmpWriteStmt.defined()) {
    loopNest = Block::make(alloc, Block::make(loopNest, tmpWriteStmt));
  }
  else {
    loopNest = Block::make(alloc, loopNest);
  }

  return loopNest;
}

/// Lowers the given 'stmt' containing an index expression.
Stmt lower(Stmt stmt, const Storage &storage) {
  SIG sig = createSIG(stmt, storage);
  LoopVars loopVars = LoopVars::create(sig);

  // Create compute kernel (loop body)
  Stmt kernel = specialize(stmt, loopVars);

  // Create loops
  Stmt loopNest = kernel;
  for (auto &loopVar : loopVars) {
    loopNest = For::make(loopVar.getVar(), loopVar.getDomain(), loopNest);

    if (loopVar.hasReduction()) {
      loopNest = reduce(loopNest, kernel, loopVar.getReductionOperator());
    }
  }

  return loopNest;
}

/// Lower the index expressions in 'func'.
Func lowerIndexExpressions(Func func) {
  class LowerIndexExpressionsRewriter : private IRRewriter {
  public:
    Func lower(Func func) {
      storage = &func.getStorage();
      return this->rewrite(func);
    }

  private:
    const Storage *storage;
    void visit(const AssignStmt *op) {
      if (isa<IndexExpr>(op->value))
        stmt = simit::ir::lower(op, *storage);
      else
        IRRewriter::visit(op);
    }

    void visit(const FieldWrite *op) {
      if (isa<IndexExpr>(op->value))
        stmt = simit::ir::lower(op, *storage);
      else
        IRRewriter::visit(op);
    }

    void visit(const TensorWrite *op) {
      if (isa<IndexExpr>(op->value))
        stmt = simit::ir::lower(op, *storage);
      else
        IRRewriter::visit(op);
    }
  };

  return LowerIndexExpressionsRewriter().lower(func);
}

}}
