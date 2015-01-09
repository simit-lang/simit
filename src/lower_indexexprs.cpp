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

Stmt specialize(Stmt stmt, const LoopVars &loopVars);
Stmt reduce(Stmt loopNest, Stmt kernel, ReductionOperator reductionOperator);
Stmt wrapCompoundAssignedValues(Stmt stmt);
Stmt lowerIndexStatement(Stmt stmt, const Storage &storage);

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
    
    using IRRewriter::visit;

    void visit(const AssignStmt *op) {
      iassert(isa<IndexExpr>(op->value))<<"Can only specialize IndexExpr stmts";
      const IndexExpr *indexExpr = to<IndexExpr>(op->value);

      Var var = op->var;
      Expr value = rewrite(op->value);

      if (isScalar(op->value.type())) {
        stmt = AssignStmt::make(var, value, op->cop);
      }
      else {
        stmt = tensorWrite(op->cop, var, indexExpr->resultVars, value);
      }
    }

    void visit(const FieldWrite *op) {
      iassert(isa<IndexExpr>(op->value))<<"Can only specialize IndexExpr stmts";
      const IndexExpr *indexExpr = to<IndexExpr>(op->value);

      Expr elementOrSet = rewrite(op->elementOrSet);
      std::string fieldName = op->fieldName;
      Expr value = rewrite(op->value);

      if (isScalar(op->value.type())) {
        stmt = FieldWrite::make(elementOrSet, fieldName, value, op->cop);
      }
      else {
        Expr field = FieldRead::make(elementOrSet, fieldName);
        stmt = tensorWrite(op->cop, field, indexExpr->resultVars, value);
      }
    }

    void visit(const TensorWrite *op) {
      iassert(isa<IndexExpr>(op->value))<<"Can only specialize IndexExpr stmts";
      const IndexExpr *indexExpr = to<IndexExpr>(op->value);

      Expr value = rewrite(op->value);

      if (isScalar(op->value.type())) {
        stmt = TensorWrite::make(op->tensor, op->indices, value, op->cop);
      }
      else {
        Expr tensor = TensorRead::make(op->tensor, op->indices);
        stmt = tensorWrite(op->cop, tensor, indexExpr->resultVars, value);
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
        std::vector<Var> vars;
        std::vector<Expr> indexExprs;
        for (const IndexVar &indexVar : indexVars) {
          if (i < indexVar.getNumBlockLevels()) {
            Var var = loopVars.getLoopVars(indexVar)[i].getVar();
            vars.push_back(var);
            indexExprs.push_back(var);
          }
        }

        // If loopVars has a variable representing the coordinate formed by vars
        // then we load from the tensor using this variable.
        Var coordVar = loopVars.getCoordVar(vars);
        if (coordVar.defined()) {
          tensor = TensorRead::make(tensor, {coordVar});
        }
        else {
          tensor = TensorRead::make(tensor, indexExprs);
        }
      }
      return tensor;
    }

    Expr tensorRead(Expr tensor, vector<IndexVar> indexVars) {
      return tensorRead(tensor, indexVars, numBlockLevels(indexVars));
    }

    Stmt tensorWrite(CompoundOperator cop, Expr tensor,
                     vector<IndexVar> indexVars, Expr value) {
      size_t blockLevels = numBlockLevels(indexVars);
      if (blockLevels > 1) {
        tensor = tensorRead(tensor, indexVars, blockLevels-1);
      }

      std::vector<Expr> indexExprs;
      for (const IndexVar &iv : indexVars) {
        size_t lastLevel = iv.getNumBlockLevels()-1;
        indexExprs.push_back(loopVars.getLoopVars(iv)[lastLevel].getVar());
      }

      return TensorWrite::make(tensor, indexExprs, value, cop);
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
    Stmt getTmpWritebackStmt() {return tmpWritebackStmt;}

  private:
    Stmt rstmt;
    ReductionOperator rop;

    Var tmpVar;
    Stmt tmpWritebackStmt;
    
    using IRRewriter::visit;

    std::string getReductionTmpName(Stmt stmt) {
      class GetReductionTmpNameVisitor : public IRVisitor {
      public:
        string get(Stmt stmt) {
          stmt.accept(this);
          return name;
        }

      private:
        std::string name;
        using IRVisitor::visit;
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

    static Stmt compoundAssign(Var var, ReductionOperator op, Expr value) {
      switch (op.getKind()) {
        case ReductionOperator::Sum:
          return AssignStmt::make(var, value, CompoundOperator::Add);
        case ReductionOperator::Undefined:
          ierror;
          return Stmt();
      }
      unreachable;
      return Stmt();
    }

    void visit(const AssignStmt *op) {
      if (op == rstmt) {
        ScalarType componentType = op->var.getType().toTensor()->componentType;
        tmpVar = Var(op->var.getName()+"tmp", TensorType::make(componentType));

        stmt = compoundAssign(tmpVar, rop, op->value);
        tmpWritebackStmt = compoundAssign(op->var, rop, tmpVar);
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

        if (isa<TensorRead>(op->tensor)) {
          tmpWritebackStmt = TensorWrite::make(op->tensor, op->indices, tmpVar,
                                               CompoundOperator::Add);
        }
        else {
          tmpWritebackStmt = TensorWrite::make(op->tensor, op->indices, tmpVar);
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

        tmpWritebackStmt = FieldWrite::make(op->elementOrSet,op->fieldName, tmpVar);
      }
      else {
        stmt = op;
      }
    }
  };

  ReduceRewriter reduceRewriter(kernel, reductionOperator);
  loopNest = reduceRewriter.rewrite(loopNest);

  // Insert a temporary scalar to reduce into
  Var tmpVar = reduceRewriter.getTmpVar();
  iassert(tmpVar.defined());

  Stmt alloc = AssignStmt::make(tmpVar, Literal::make(tmpVar.getType(), {0}));
  loopNest = Block::make(alloc, loopNest);

  Stmt tmpWritebackStmt = reduceRewriter.getTmpWritebackStmt();
  if (tmpWritebackStmt.defined()) {
    loopNest = Block::make(loopNest, tmpWritebackStmt);
  }

  return loopNest;
}

/// Wraps tensor values that are compound assigned or written in index
/// expressions to trigger lowering.
Stmt wrapCompoundAssignedValues(Stmt stmt) {
  class CompoundValueWrapper : public IRRewriter {
    using IRRewriter::visit;
    
    void visit(const AssignStmt *op) {
      if (op->cop != CompoundOperator::None && !isa<IndexExpr>(op->value)) {
        // Wrap the written value in an index expression to trigger lowering
        Expr value = IRBuilder().unaryElwiseExpr(IRBuilder::None, op->value);
        stmt = AssignStmt::make(op->var, value, op->cop);
      }
      else {
        stmt = op;
      }
    }

    void visit(const FieldWrite *op) {
      if (op->cop != CompoundOperator::None && !isa<IndexExpr>(op->value)) {
        // Wrap the written value in an index expression to trigger lowering
        Expr value = IRBuilder().unaryElwiseExpr(IRBuilder::None, op->value);
        stmt = FieldWrite::make(op->elementOrSet, op->fieldName, value,
                                op->cop);
      }
      else {
        stmt = op;
      }
    }

    void visit(const TensorWrite *op) {
      if (op->cop != CompoundOperator::None && !isa<IndexExpr>(op->value)) {
        // Wrap the written value in an index expression to trigger lowering
        Expr value = IRBuilder().unaryElwiseExpr(IRBuilder::None, op->value);
        stmt = TensorWrite::make(op->tensor, op->indices, value, op->cop);
      }
      else {
        stmt = op;
      }
    }
  };
  return CompoundValueWrapper().rewrite(stmt);
}

/// Lowers the given 'stmt' containing an index expression.
Stmt lowerIndexStatement(Stmt stmt, const Storage &storage) {
  stmt = wrapCompoundAssignedValues(stmt);

  SIG sig = createSIG(stmt, storage);
  LoopVars loopVars = LoopVars::create(sig, storage);

  // Create initial compute kernel (loop body). The initial kernel does not take
  // into account reductions, so it will be rewritten to reflect these.
  Stmt kernel = specialize(stmt, loopVars);

  // Create loops (since we create the loops inside out, we must iterate over
  // the loop vars in reverse order)
  Stmt loopNest = kernel;
  for (auto loopVar=loopVars.rbegin(); loopVar!=loopVars.rend(); ++loopVar){
    if (loopVar->getDomain().kind == ForDomain::IndexSet) {
      loopNest = For::make(loopVar->getVar(), loopVar->getDomain(), loopNest);
    }
    else if (loopVar->getDomain().kind == ForDomain::Neighbors) {
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
    }
    else if (loopVar->getDomain().kind == ForDomain::Diagonal) {
      Var i = loopVar->getDomain().var;
      Var j = loopVar->getVar();
      
      loopNest = Block::make(AssignStmt::make(j, i), loopNest);
      
    } else {
      not_supported_yet;
    }

    if (loopVar->hasReduction()) {
      loopNest = reduce(loopNest, kernel, loopVar->getReductionOperator());
    }
  }

  // Initialize the result. Only necessary for sparse computations or
  // reductions to scalars.
  bool isResultScalar = containsReductionVar(stmt) && !containsFreeVar(stmt);
  if (isResultScalar || sig.isSparse()) {
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
    
    using IRRewriter::visit;
    
    void visit(const AssignStmt *op) {
      if (isa<IndexExpr>(op->value) || op->cop != CompoundOperator::None)
        stmt = simit::ir::lowerIndexStatement(op, *storage);
      else
        IRRewriter::visit(op);
    }

    void visit(const FieldWrite *op) {
      if (isa<IndexExpr>(op->value) || op->cop != CompoundOperator::None)
        stmt = simit::ir::lowerIndexStatement(op, *storage);
      else
        IRRewriter::visit(op);
    }

    void visit(const TensorWrite *op) {
      if (isa<IndexExpr>(op->value) || op->cop != CompoundOperator::None)
        stmt = simit::ir::lowerIndexStatement(op, *storage);
      else
        IRRewriter::visit(op);
    }

    void visit(const IndexExpr *op) {
      iassert_scalar(Expr(op));
      expr = rewrite(op->value);
    }

    void visit(const IndexedTensor *op) {
      expr = op->tensor;
    }
  };
  return LowerIndexExpressionsRewriter().lower(func);
}

}}
