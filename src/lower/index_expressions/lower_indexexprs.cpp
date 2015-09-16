#include "lower_indexexprs.h"

#include "ir.h"
#include "ir_codegen.h"
#include "ir_queries.h"
#include "ir_rewriter.h"
#include "ir_transforms.h"
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

      Expr tensor = rewrite(op->tensor);

      vector<Expr> indices;
      for (auto &index : op->indices) {
        indices.push_back(rewrite(index));
      }

      Expr value = rewrite(op->value);

      if (isScalar(op->value.type())) {
        stmt = TensorWrite::make(tensor, indices, value, op->cop);
      }
      else {
        tensor = TensorRead::make(tensor, indices);
        stmt = tensorWrite(op->cop, tensor,
                           to<IndexExpr>(op->value)->resultVars, value);
      }
    }

    /// Replace indexed tensors with tensor reads
    void visit(const IndexedTensor *op) {
      iassert(!isa<IndexExpr>(op->tensor))
          << "index expressions should have been flattened by now";

      expr = rewrite(op->tensor);

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

    Var getReductionVar() {return reductionVar;}

    /// Retrieve a statement that writes the tmp variable to the original
    /// location of the rewritten statement.  If result is !defined then the
    /// reduction variable does not ned to be written back.
    Stmt getReductionVarWritebackStmt() {return reductionVarWriteBackStmt;}

  private:
    Stmt rstmt;
    ReductionOperator rop;

    Var reductionVar;
    Stmt reductionVarWriteBackStmt;
    
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
        reductionVar = Var(op->var.getName()+"tmp",
                           TensorType::make(componentType));

        stmt = compoundAssign(reductionVar, rop, op->value);
        reductionVarWriteBackStmt = compoundAssign(op->var, rop, reductionVar);
      }
      else {
        stmt = op;
      }
    }

    void visit(const TensorWrite *op) {
      if (op == rstmt) {
        iassert(op->value.type().isTensor());
        ScalarType componentType = op->value.type().toTensor()->componentType;
        string reductionVarName = getReductionTmpName(op);
        reductionVar = Var(reductionVarName, TensorType::make(componentType));
        stmt = compoundAssign(reductionVar, rop, op->value);

        if (isa<TensorRead>(op->tensor)) {
          reductionVarWriteBackStmt = TensorWrite::make(op->tensor, op->indices,
                                               reductionVar,
                                               CompoundOperator::Add);
        }
        else {
          reductionVarWriteBackStmt = TensorWrite::make(op->tensor, op->indices,
                                               reductionVar);
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
        string reductionVarName = getReductionTmpName(op);
        reductionVar = Var(reductionVarName, TensorType::make(componentType));
        stmt = compoundAssign(reductionVar, rop, op->value);

        reductionVarWriteBackStmt = FieldWrite::make(op->elementOrSet,
                                                     op->fieldName,
                                                     reductionVar);
      }
      else {
        stmt = op;
      }
    }
  };

  ReduceRewriter reduceRewriter(kernel, reductionOperator);
  loopNest = reduceRewriter.rewrite(loopNest);

  // Insert a temporary scalar to reduce into
  Var rvar = reduceRewriter.getReductionVar();
  iassert(rvar.defined());

  Type rvarType = rvar.getType();
  Stmt rvarDecl = VarDecl::make(rvar);
  Stmt rvarInit =
      Block::make(rvarDecl, AssignStmt::make(rvar,Literal::make(rvarType,{0})));

  loopNest = Block::make(rvarInit, loopNest);

  Stmt tmpWritebackStmt = reduceRewriter.getReductionVarWritebackStmt();
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
  class DiagonalReadsRewriter : private IRRewriter {
  public:
    std::vector<Stmt> liftedStmts;

    DiagonalReadsRewriter(const Storage& storage, Var outerIndexVar,
                          LoopVars loopVars)
        :storage(storage), loopVars(loopVars), outerIndexVar(outerIndexVar) {
      currentTensorWrite = nullptr;
    }

    Stmt doRewrite(Stmt stmt) {
      return this->rewrite(stmt);
    }
  private:
    using IRRewriter::visit;
    const Storage &storage;
    LoopVars loopVars;
    Var outerIndexVar;
    const TensorWrite *currentTensorWrite;

    void visit(const TensorWrite *op) {
      currentTensorWrite = op;
      auto rewritten = rewrite(op->value);

      if (rewritten != op->value) {
        stmt = TensorWrite::make(op->tensor, op->indices, rewrite(op->value));
      }
      else {
        stmt = op;
      }
      currentTensorWrite = nullptr;
    }

    Expr liftDiagonalExpression(Expr exprToCheck, Expr otherExpr, Expr op,
        Expr containingToCheck=Expr(), Expr containingOther=Expr()) {
      
      auto tensor = to<TensorRead>(exprToCheck)->tensor;

      // If tensor is not a VarExpr, that means we have a hierarchical TensorRead
      if (!isa<VarExpr>(tensor)) {
        tassert(isa<TensorRead>(otherExpr))
            << "we only support one level of blocking" << otherExpr;
        
        auto otherTensor = to<TensorRead>(otherExpr)->tensor;
        return liftDiagonalExpression(tensor, otherTensor, op,
                                      exprToCheck, otherExpr);
      }
      
      TensorStorage::Kind storageKind =
          storage.getStorage(to<VarExpr>(tensor)->var).getKind();
      
      if (storageKind == TensorStorage::Kind::Diagonal &&
          currentTensorWrite != nullptr) {
        std::vector<Expr> newIndices;
        newIndices.push_back(VarExpr::make(outerIndexVar));

        std::vector<Expr> newWriteIndices;
        for (auto x: currentTensorWrite->indices)
          newWriteIndices.push_back(outerIndexVar);

        auto newRead = TensorRead::make(tensor,newIndices);
        Expr newWriteTensor = currentTensorWrite->tensor;
        
        
        if (containingToCheck.defined()) {
          // we will make a loop over the innermost indices
          // we find out which ones by looking at the containing expression's
          // indices
          auto containingTensorExpr = to<TensorRead>(containingToCheck);
          newRead = TensorRead::make(newRead, containingTensorExpr->indices);

          // we also need to change the write such that the new write indices go
          // outermost
          iassert(isa<TensorRead>(newWriteTensor));
          
          newWriteTensor = TensorRead::make(to<TensorRead>(newWriteTensor)->tensor, newWriteIndices);
          newWriteIndices = currentTensorWrite->indices;
        }
        Stmt liftedStmt;
        
        liftedStmt = TensorWrite::make(newWriteTensor, newWriteIndices,
                          newRead);
        
        if (containingToCheck.defined()) {
          auto containingTensorExpr = to<TensorRead>(containingToCheck);
          size_t i = 0;
          for (auto lv=loopVars.rbegin();
               i<containingTensorExpr->indices.size() &&
               lv != loopVars.rend();
               lv++) {
            liftedStmt = For::make(Var(lv->getVar()), ForDomain(lv->getDomain()), liftedStmt);
            i++;
          }
          
        }
        liftedStmts.push_back(liftedStmt);

        Expr lhsRead = TensorRead::make(currentTensorWrite->tensor,
                                        currentTensorWrite->indices);
        
        if (containingOther.defined()) {
          auto containingTensorExpr = to<TensorRead>(containingOther);
          otherExpr = TensorRead::make(otherExpr, containingTensorExpr->indices);
          
        }
        Expr rhs;
        if (isa<Add>(op)) {
          return Add::make(lhsRead, otherExpr);
        }
        else if (isa<Sub>(op)) {
          return Sub::make(lhsRead, otherExpr);
        }
        iassert(rhs.defined());
        return rhs;
      }
      else {
        return op;
      }
    }

    void visit(const Add *op) {
      // the idea here is to lift out the system diagonal tensor reads (that
      // take place during a tensor write) and place them in a higher level
      // loop.  In addition, change A(i,j) to A(i) if A is diagonal.
      //
      // E.g. `C(i,j) = A(i,j) + B(i,j)`, where B is system reduced, becomes:
      // for i in points:
      //   C(loc(i,j)) = A(i)
      //   for ij in neighbors.start[i]:neighbors.start[i+1]:
      //     C(ij) += B(ij)

      if (isa<TensorRead>(op->a)) {
        expr = liftDiagonalExpression(op->a, op->b, op);
      }
      else if (isa<TensorRead>(op->b)) {
        expr = liftDiagonalExpression(op->b, op->a, op);
      }
      else {
        expr = op;
      }
    }

    void visit(const Sub *op) {
      if (isa<TensorRead>(op->a)) {
        expr = liftDiagonalExpression(op->a, op->b, op);
      }
      else if (isa<TensorRead>(op->b)) {
        expr = liftDiagonalExpression(op->b, op->a, op);
      }
      else {
        expr = op;
      }
    }
  };

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
      // if this is a Single domain, don't generate a loop, just use an
      // assignment statement.
      if (loopVar->getDomain().indexSet.getKind() == IndexSet::Single) {
        auto assign = AssignStmt::make(loopVar->getVar(),
            loopVar->getDomain().indexSet.getSet());
        loopNest = Block::make(assign, loopNest);
      }
      else {
      loopNest = For::make(loopVar->getVar(), loopVar->getDomain(), loopNest);
      }
    }
    else if (loopVar->getDomain().kind == ForDomain::Neighbors ||
             loopVar->getDomain().kind == ForDomain::NeighborsOf) {
      Expr set = loopVar->getDomain().set;
      Var i = loopVar->getDomain().var;
      Var j = loopVar->getVar();
      Var ij = loopVars.getCoordVar({i, j});

      Expr jRead = Load::make(IndexRead::make(set, IndexRead::Neighbors), ij);
      
      // for NeighborsOf, we need to check if this is the j we are looking
      // for
      if (loopVar->getDomain().kind == ForDomain::NeighborsOf) {
        Expr jCond = Eq::make(j, loopVar->getDomain().indexSet.getSet());
        loopNest = IfThenElse::make(jCond, loopNest, Pass::make());
      }

      loopNest = Block::make(AssignStmt::make(j, jRead), loopNest);

      
      Expr start = Load::make(IndexRead::make(set, IndexRead::NeighborsStart),
                              i);
      Expr stop  = Load::make(IndexRead::make(set, IndexRead::NeighborsStart),
                              Add::make(i, 1));

      // Rewrite accesses to SystemDiagonal tensors & lift out ops
      DiagonalReadsRewriter drRewriter(storage, i, loopVars);
      loopNest = drRewriter.doRewrite(loopNest);
      if (drRewriter.liftedStmts.size() > 0) {
        auto newBlock = Block::make(drRewriter.liftedStmts);
        loopNest = Block::make(newBlock,
                               ForRange::make(ij, start, stop, loopNest));
      }
      else {
        loopNest = ForRange::make(ij, start, stop, loopNest);
      }
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
Func lowerIndexExprs(Func func) {
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
  func = LowerIndexExpressionsRewriter().lower(func);
  func = insertVarDecls(func);

  return func;
}

}}
