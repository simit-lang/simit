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

      expr = op->tensor;

      // Add tensor reads until the type of expr is a scalar. This can require
      // multiple reads if the indexed tensor was blocked.
      while (!isScalar(expr.type())) {
        expr = tensorRead(expr, op->indexVars);
      }
    }

    /// Removes index expression
    void visit(const IndexExpr *op) {
      expr = rewrite(op->value);
    }

    size_t numBlockLevels(vector<IndexVar> indexVars) {
      size_t blockLevels = 0;
      for (auto &indexVar : indexVars) {
        if (blockLevels < indexVar.getNumBlockLevels()) {
          blockLevels = indexVar.getNumBlockLevels();
        }
      }
      return blockLevels;
    }

    Expr tensorRead(Expr tensor, vector<IndexVar> indexVars, size_t blockLvls) {
      for (size_t i=0; i < blockLvls; ++i) {
        std::vector<Expr> indexExprs;
        for (const IndexVar &indexVar : indexVars) {
          if (i < indexVar.getNumBlockLevels()) {
            indexExprs.push_back(loopVars.getLoopVars(indexVar)[i].getVar());
          }
        }
        tensor = TensorRead::make(tensor, indexExprs);
      }
      return tensor;
    }

    Expr tensorRead(Expr tensor, vector<IndexVar> indexVars) {
      return tensorRead(tensor, indexVars, numBlockLevels(indexVars));
    }

    Stmt tensorWrite(Expr tensor, vector<IndexVar> indexVars, Expr value) {
      size_t blockLevels = numBlockLevels(indexVars);
      if (blockLevels > 1) {
        tensor = tensorRead(tensor, indexVars, blockLevels-1);
      }

      std::vector<Expr> indexExprs;
      for (const IndexVar &iv : indexVars) {
        size_t lastLevel = iv.getNumBlockLevels()-1;
        indexExprs.push_back(loopVars.getLoopVars(iv)[lastLevel].getVar());
      }
      return TensorWrite::make(tensor, indexExprs, value);
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
        tmpVar = op->var;
        stmt = compoundAssign(tmpVar, rop, op->value);
      }
      else {
        stmt = op;
      }
    }

    void visit(const TensorWrite *op) {
      if (op == rstmt) {
        iassert(op->value.type().isTensor());
        ScalarType componentType = op->value.type().toTensor()->componentType;
        string tmpVarName = getReductionTmpName(op);
        tmpVar = Var(tmpVarName, TensorType::make(componentType));
        stmt = compoundAssign(tmpVar, rop, op->value);

        tmpWriteStmt = TensorWrite::make(op->tensor, op->indices, tmpVar);
        if (isa<TensorRead>(op->tensor)) {
          tmpWriteStmt = makeCompound(tmpWriteStmt, CompoundOperator::Add);
        }
      }
      else {
        stmt = op;
      }
    }

    void visit(const FieldWrite *op) {
      if (op == rstmt) {
        iassert(op->value.type().isTensor());
        ScalarType componentType = op->value.type().toTensor()->componentType;
        string tmpVarName = getReductionTmpName(op);
        tmpVar = Var(tmpVarName, TensorType::make(componentType));
        stmt = compoundAssign(tmpVar, rop, op->value);

        tmpWriteStmt = FieldWrite::make(op->elementOrSet,op->fieldName, tmpVar);
      }
      else {
        stmt = op;
      }
    }

    static Stmt compoundAssign(Var var, ReductionOperator op, Expr value) {
      switch (op.getKind()) {
        case ReductionOperator::Sum:
          return AssignStmt::make(var, Add::make(var, value));
        case ReductionOperator::Undefined:
          ierror;
          return Stmt();
      }
    }

    std::string getReductionTmpName(Stmt stmt) {
      class GetReductionTmpNameVisitor : public IRVisitor {
      public:
        string get(Stmt stmt) {
          stmt.accept(this);
          return name;
        }

      private:
        std::string name;
        void visit(const TensorWrite *op) {
          op->tensor.accept(this);
          for (auto &index : op->indices) {
            index.accept(this);
          }
        }
        void visit(const FieldWrite *op) {
          op->elementOrSet.accept(this);
          name += "_" + op->fieldName;
        }
        void visit(const VarExpr *op) {
          name += op->var.getName();
        }
      }; // class GetReductionTmpNameVisitor

      return GetReductionTmpNameVisitor().get(stmt);
    }
  };

  ReduceRewriter reduceRewriter(kernel, reductionOperator);
  loopNest = reduceRewriter.rewrite(loopNest);

  Var tmpVar = reduceRewriter.getTmpVar();
  iassert(tmpVar.defined());

  Stmt alloc = AssignStmt::make(tmpVar, Literal::make(tmpVar.getType(), {0}));
  loopNest = Block::make(alloc, loopNest);

  Stmt tmpWriteStmt = reduceRewriter.getTmpWriteStmt();
  if (tmpWriteStmt.defined()) {
    loopNest = Block::make(loopNest, tmpWriteStmt);
  }

  return loopNest;
}

/// Lowers the given 'stmt' containing an index expression.
Stmt lower(Stmt stmt, const Storage &storage) {
  SIG sig = createSIG(stmt, storage);
  LoopVars loopVars = LoopVars::create(sig);

  // Create compute kernel (loop body)
  Stmt kernel = specialize(stmt, loopVars);

  // Create loops (since we create the loops inside out, we must iterate over
  // the loop vars in reverse order)
  Stmt loopNest = kernel;
  for (auto loopVar=loopVars.rbegin(); loopVar!=loopVars.rend(); ++loopVar){
    if (loopVar->getDomain().kind == ForDomain::Neighbors) {
      Expr set = loopVar->getDomain().set;
      Var i = loopVar->getDomain().var;
      Var j = loopVar->getVar();
      Var ij = loopVars.getCoordVar({i, j});

      Expr jRead = Load::make(IndexRead::make(set, IndexRead::Neighbors), ij);
      loopNest = Block::make(AssignStmt::make(j, jRead), loopNest);

      Expr start = Load::make(IndexRead::make(set, IndexRead::NeighborsStart),
                              i);
      Expr stop  = Load::make(IndexRead::make(set, IndexRead::NeighborsStart),
                              Add::make(i, 1));

      loopNest = ForRange::make(ij, start, stop, loopNest);
    } else {
      loopNest = For::make(loopVar->getVar(), loopVar->getDomain(), loopNest);
    }
    if (loopVar->hasReduction()) {
      loopNest = reduce(loopNest, kernel, loopVar->getReductionOperator());
    }
  }

  // Insert initialization statement for the result. This is only needed when
  // the left-hand side is a sparse iteration
  if (sig.isSparse()) {
    loopNest = Block::make(initializeLhsToZero(stmt), loopNest);
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
