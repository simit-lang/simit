#include "lower_indexexprs.h"

#include "ir.h"
#include "ir_codegen.h"
#include "ir_queries.h"
#include "ir_rewriter.h"
#include "ir_transforms.h"
#include "ir_printer.h"
#include "sig.h"
#include "path_expressions.h"
#include "tensor_index.h"
#include "stencils.h"

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

      std::vector<Var> vars;
      std::vector<Expr> indexExprs;
      for (const IndexVar &iv : indexVars) {
        size_t lastLevel = iv.getNumBlockLevels()-1;
        Var var = loopVars.getLoopVars(iv)[lastLevel].getVar();
        indexExprs.push_back(var);
        vars.push_back(var);
      }

      Var coordVar = loopVars.getCoordVar(vars);
      if (coordVar.defined()) {
        return TensorWrite::make(tensor, {coordVar}, value, cop);
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
        ScalarType ctype = op->var.getType().toTensor()->getComponentType();
        reductionVar = Var(op->var.getName()+"tmp",
                           TensorType::make(ctype));

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
        ScalarType ctype = op->value.type().toTensor()->getComponentType();
        if (!reductionVar.defined()) {
          string reductionVarName = getReductionTmpName(op);
          reductionVar = Var(reductionVarName, TensorType::make(ctype));
        }
        stmt = compoundAssign(reductionVar, rop, op->value);

        if (isa<TensorRead>(op->tensor)) {
          reductionVarWriteBackStmt = TensorWrite::make(op->tensor, op->indices,
                                                        reductionVar,
                                                        CompoundOperator::Add);
        }
        else {
          reductionVarWriteBackStmt = TensorWrite::make(op->tensor, op->indices,
                                                        reductionVar, op->cop);
        }
      }
      else {
        stmt = op;
      }
    }

    void visit(const FieldWrite *op) {
      if (op == rstmt) {
        iassert(op->value.type().isTensor());
        ScalarType ctype = op->value.type().toTensor()->getComponentType();
        string reductionVarName = getReductionTmpName(op);
        reductionVar = Var(reductionVarName, TensorType::make(ctype));
        stmt = compoundAssign(reductionVar, rop, op->value);

        reductionVarWriteBackStmt = FieldWrite::make(op->elementOrSet,
                                                     op->fieldName,
                                                     reductionVar, op->cop);
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
  Stmt rvarInitZero = initializeLhsToZero(AssignStmt::make(rvar,rvar));
  Stmt rvarInit = Block::make(rvarDecl, rvarInitZero);

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
        Expr value = IRBuilder().unaryElwiseExpr(IRBuilder::Copy, op->value);
        stmt = AssignStmt::make(op->var, value, op->cop);
      }
      else {
        stmt = op;
      }
    }

    void visit(const FieldWrite *op) {
      if (op->cop != CompoundOperator::None && !isa<IndexExpr>(op->value)) {
        // Wrap the written value in an index expression to trigger lowering
        Expr value = IRBuilder().unaryElwiseExpr(IRBuilder::Copy, op->value);
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
        Expr value = IRBuilder().unaryElwiseExpr(IRBuilder::Copy, op->value);
        stmt = TensorWrite::make(op->tensor, op->indices, value, op->cop);
      }
      else {
        stmt = op;
      }
    }
  };
  return CompoundValueWrapper().rewrite(stmt);
}

TensorIndex getTensorIndexOfStatement(Stmt stmt, const Storage& storage,
                                      Environment* environment) {
  TensorIndex tensorIndex;
  match(stmt,
    std::function<void(const VarExpr*)>([&](const VarExpr* op) {
      const Var& var = op->var;
      const Type& type = op->type;
      if (type.isTensor() && type.toTensor()->order() == 2) {
        iassert(storage.hasStorage(var));
        const TensorStorage& tensorStorage = storage.getStorage(var);
        if (tensorStorage.getKind() == TensorStorage::Kind::Indexed) {
          iassert(tensorStorage.hasTensorIndex());
          tensorIndex = tensorStorage.getTensorIndex();
        }
        else if (tensorStorage.getKind() == TensorStorage::Kind::Stencil) {
          iassert(tensorStorage.hasTensorIndex());
          iassert(tensorStorage.getTensorIndex().getKind() ==
                  TensorIndex::Sten);
          tensorIndex = tensorStorage.getTensorIndex();
        }
      }
    })
  );
  return tensorIndex;
}

/// Lowers the given 'stmt' containing an index expression.
Stmt lowerIndexStatement(Stmt stmt, Environment* environment, Storage storage) {
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
        stmt = TensorWrite::make(op->tensor, op->indices, rewritten);
      }
      else {
        stmt = op;
      }
      currentTensorWrite = nullptr;
    }

    Expr liftDiagonalExpression(Expr exprToCheck, Expr otherExpr, Expr op,
                                vector<Expr> containingToCheck={},
                                vector<Expr> containingOther={}) {
      auto tensor = to<TensorRead>(exprToCheck)->tensor;

      // If tensor is not a VarExpr, that means we have a hierarchical TensorRead
      if (!isa<VarExpr>(tensor)) {
        tassert(isa<TensorRead>(otherExpr))
            << "we only support one level of blocking" << otherExpr;

        auto otherTensor = to<TensorRead>(otherExpr)->tensor;
        containingToCheck.push_back(exprToCheck);
        containingOther.push_back(otherExpr);
        return liftDiagonalExpression(tensor, otherTensor, op,
                                      containingToCheck, containingOther);
      }

      TensorStorage tensorStorage= storage.getStorage(to<VarExpr>(tensor)->var);
      TensorStorage::Kind storageKind = tensorStorage.getKind();
      
      if (storageKind == TensorStorage::Kind::Diagonal &&
          currentTensorWrite != nullptr) {
        std::vector<Expr> newIndices;
        newIndices.push_back(VarExpr::make(outerIndexVar));

        // To store the diagonal input into the non-diagonal matrix we need
        // to use loc to scan until we find the diagonal entry, e.g.:
        // A(loc(n,n)) = I(n);
        // We accomplish this by the access into A(n,n)
        std::vector<Expr> newWriteIndices;
        newWriteIndices.push_back(outerIndexVar);
        newWriteIndices.push_back(outerIndexVar);
        
        auto newRead = TensorRead::make(tensor,newIndices);
        Expr newWriteTensor = currentTensorWrite->tensor;

        if (containingToCheck.size() > 0) {
          // we will make a loop over the innermost indices
          // we find out which ones by looking at the containing expression's
          // indices
          for (int i = containingToCheck.size()-1; i >= 0; --i) {
            auto containingTensorExpr = to<TensorRead>(containingToCheck[i]);
            newRead = TensorRead::make(newRead, containingTensorExpr->indices);
          }

          // we also need to change the write such that the new write indices go
          // outermost
          iassert(isa<TensorRead>(newWriteTensor));

          while (!isa<VarExpr>(newWriteTensor)) {
            newWriteTensor = to<TensorRead>(newWriteTensor)->tensor;
          }
          newWriteTensor = TensorRead::make(newWriteTensor, newWriteIndices);

          // Index by all but the last containing index (which will become the
          // TensorWrite index)
          for (int i = containingToCheck.size()-1; i >= 1; --i) {
            auto containingTensorExpr = to<TensorRead>(containingToCheck[i]);
            newWriteTensor = TensorRead::make(newWriteTensor,
                                              containingTensorExpr->indices);
          }
          newWriteIndices = currentTensorWrite->indices;
        }
        Stmt liftedStmt;
        
        liftedStmt = TensorWrite::make(newWriteTensor, newWriteIndices,
                          newRead);

        if (containingToCheck.size() > 0) {
          size_t totalIndices = 0;
          for (Expr e : containingToCheck) {
            auto containingTensorExpr = to<TensorRead>(e);
            totalIndices += containingTensorExpr->indices.size();
          }
          size_t i = 0;
          for (auto lv=loopVars.rbegin();
               i < totalIndices && lv != loopVars.rend();
               lv++) {
            liftedStmt = For::make(Var(lv->getVar()),
                                   ForDomain(lv->getDomain()), liftedStmt);
            i++;
          }
        }
        liftedStmts.push_back(liftedStmt);

        Expr lhsRead = TensorRead::make(currentTensorWrite->tensor,
                                        currentTensorWrite->indices);

        if (containingOther.size() > 0) {
          for (int i = containingOther.size()-1; i >= 0; --i) {
            auto containingTensorExpr = to<TensorRead>(containingOther[i]);
            otherExpr = TensorRead::make(otherExpr, containingTensorExpr->indices);
          }
        }
        Expr rhs;
        if (isa<Add>(op)) {
          rhs = Add::make(lhsRead, otherExpr);
        }
        else if (isa<Sub>(op)) {
          rhs = Sub::make(lhsRead, otherExpr);
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
      //   C(loc(i,i)) = A(i)
      //   for ij in neighbors.start[i]:neighbors.start[i+1]:
      //     C(ij) += B(ij)

      if (isa<TensorRead>(op->a)) {
        expr = liftDiagonalExpression(op->a, op->b, op);
      }
      else if (isa<TensorRead>(op->b)) {
        // Sketchy on non-commutative op?
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
  
  // HACK: Extract loop vars corresponding to lattice loops
  map<Var, const LoopVar*> latticeLoopVars;
  for (auto loopVar=loopVars.begin(); loopVar!=loopVars.end(); ++loopVar) {
    if (loopVar->getDomain().kind == ForDomain::Lattice) {
      latticeLoopVars[loopVar->getDomain().var] = &(*loopVar);
    }
  }
  
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
    else if (loopVar->getDomain().kind == ForDomain::Lattice) {
      int ndims = loopVar->getDomain().latticeVars.size();
      // Add overall variable advancement
      loopNest = Block::make(loopNest, AssignStmt::make(
          loopVar->getDomain().var, 1, CompoundOperator::Add));
      for (int i = 0; i < ndims; ++i) {
        Expr dimSize = IndexRead::make(loopVar->getDomain().set,
                                       IndexRead::LatticeDim, i);
        loopNest = ForRange::make(loopVar->getDomain().latticeVars[i],
                                  0, dimSize, loopNest);
      }
      // Zero out the overall variable
      Stmt initVar = AssignStmt::make(loopVar->getDomain().var, 0);
      loopNest = Block::make(initVar, loopNest);
    }
    else if (loopVar->getDomain().kind == ForDomain::Neighbors ||
             loopVar->getDomain().kind == ForDomain::NeighborsOf) {
      Expr set = loopVar->getDomain().set;
      Var i = loopVar->getDomain().var;
      Var j = loopVar->getVar();
      Var ij = loopVars.getCoordVar({i, j});

      TensorIndex tensorIndex = getTensorIndexOfStatement(stmt, storage,
                                                          environment);
      if (tensorIndex.getKind() == TensorIndex::PExpr) {
        iassert(tensorIndex.getColidxArray().defined())
            << "Empty tensor index returned from: " << stmt;

        Expr jRead = Load::make(tensorIndex.getColidxArray(), ij);

        // for NeighborsOf, we need to check if this is the j we are looking for
        if (loopVar->getDomain().kind == ForDomain::NeighborsOf) {
          Expr jCond = Eq::make(j, loopVar->getDomain().indexSet.getSet());
          loopNest = IfThenElse::make(jCond, loopNest, Pass::make());
        }

        loopNest = Block::make(AssignStmt::make(j, jRead), loopNest);

        Expr start = Load::make(tensorIndex.getRowptrArray(), i);
        Expr stop = Load::make(tensorIndex.getRowptrArray(), i+1);

        // Rewrite accesses to any SystemDiagonal tensors & lift out ops
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
      else if (tensorIndex.getKind() == TensorIndex::Sten) {
        const StencilLayout &stencil = tensorIndex.getStencilLayout();
        iassert(stencil.defined())
            << "Empty stencil tensor index returned from: " << stmt;

        // Flip stencil layout map
        map<int, vector<int> > flipped;
        for (auto &kv : stencil.getLayout()) {
          flipped[kv.second] = kv.first;
        }

        // Use canonical memory ordering to infer j from stencil offsets
        Expr latticeSet = stencil.getLatticeSet();
        iassert(latticeSet.type().isLatticeLinkSet());
        unsigned dims = latticeSet.type().toLatticeLinkSet()->dimensions;

        // Fetch the full LoopVar corresponding to the i lattice loop
        iassert(latticeLoopVars.count(i));
        const LoopVar *latticeLoopVar = latticeLoopVars[i];
        iassert(latticeLoopVar->getDomain().latticeVars.size() == dims);
        const vector<Var> &latticeVars = latticeLoopVar->getDomain().latticeVars;

        // Use fixed stencil size to do an unrolled DIA-style loop for ij, j
        int stencilSize = stencil.getLayout().size();
        vector<Stmt> ijLoop;
        for (int ijInd = 0; ijInd < stencilSize; ++ijInd) {
          // Assign ij
          ijLoop.push_back(AssignStmt::make(ij, stencilSize*i+ijInd));
          // Compute and assign j
          vector<int> offsets = flipped[ijInd];
          Expr totalInd = Literal::make(0);
          for (int d = dims-1; d >= 0; --d) {
            Expr dimSize = IndexRead::make(latticeSet, IndexRead::LatticeDim, d);
            // Periodic boundary conditions
            Expr ind = ((latticeVars[d]+offsets[d])%dimSize+dimSize)%dimSize;
            totalInd = totalInd * dimSize + ind;
          }
          ijLoop.push_back(AssignStmt::make(j, totalInd));
          // Perform inner loop
          ijLoop.push_back(loopNest);
        }
        loopNest = Block::make(ijLoop);
      }
      else {
        unreachable;
      }
    }
    else if (loopVar->getDomain().kind == ForDomain::Diagonal) {
      Var i = loopVar->getDomain().var;
      Var j = loopVar->getVar();
      
      loopNest = Block::make(AssignStmt::make(j, i), loopNest);
    }
    else {
      not_supported_yet;
    }

    if (loopVar->hasReduction()) {
      loopNest = reduce(loopNest, kernel, loopVar->getReductionOperator());
    }
  }

  // Initialize the result. Only necessary for sparse computations or
  // reductions to scalars. If original statement is compound assignment, 
  // then result is assumed to have already been initialized at some point 
  // and therefore do not initialize again.
  bool isResultScalar = containsReductionVar(stmt) && !containsFreeVar(stmt);
  bool isCompoundAssign = (isa<AssignStmt>(stmt) && 
      (to<AssignStmt>(stmt)->cop != CompoundOperator::None)) ||
      (isa<TensorWrite>(stmt) && 
      (to<TensorWrite>(stmt)->cop != CompoundOperator::None)) ||
      (isa<FieldWrite>(stmt) &&
      (to<FieldWrite>(stmt)->cop != CompoundOperator::None));
  bool isVarSparse = (isa<AssignStmt>(stmt) && 
                      (storage.hasStorage(to<AssignStmt>(stmt)->var) &&
                       storage.getStorage(to<AssignStmt>(stmt)->var).getKind()
                       == TensorStorage::Indexed));
  if (isResultScalar || isVarSparse) {
    loopNest = Block::make(initializeLhsToZero(stmt), loopNest);
  }
  else if (sig.isSparse() && !isCompoundAssign) {
    loopNest = Block::make(initializeTensorToZero(stmt), loopNest);
  }

  return loopNest;
}


/// Lower the index expressions in 'func'.
Func lowerIndexExprs(Func func) {
  class LowerIndexExpressionsRewriter : private IRRewriter {
  public:
    Func lower(Func func) {
      storage = &func.getStorage();
      environment = func.getEnvironment();
      Stmt body = this->rewrite(func.getBody());
      return Func(func.getName(), func.getArguments(), func.getResults(),
                  body, environment);
    }

  private:
    const Storage *storage;
    Environment environment;
    
    using IRRewriter::visit;
    
    void visit(const AssignStmt *op) {
      if (isa<IndexExpr>(op->value) || op->cop != CompoundOperator::None)
        stmt = simit::ir::lowerIndexStatement(op, &environment, *storage);
      else
        IRRewriter::visit(op);
    }

    void visit(const FieldWrite *op) {
      if (isa<IndexExpr>(op->value) || op->cop != CompoundOperator::None)
        stmt = simit::ir::lowerIndexStatement(op, &environment, *storage);
      else
        IRRewriter::visit(op);
    }

    void visit(const TensorWrite *op) {
      if (isa<IndexExpr>(op->value) || op->cop != CompoundOperator::None)
        stmt = simit::ir::lowerIndexStatement(op, &environment, *storage);
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
