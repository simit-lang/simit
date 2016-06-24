#include "lower_matrix_multiply.h"

#include <vector>

#include "loops.h"
#include "lower_indexexprs.h"
#include "lower_tensor_utils.h"
#include "path_expressions.h"

using namespace std;

namespace simit {
namespace ir {

Stmt lowerMatrixMultiply(Var target, const IndexExpr* indexExpression,
                         Environment* env, Storage* storage) {
  iassert(target.getType().isTensor());
  const TensorType* type = target.getType().toTensor();
  tassert(type->order() == 2)
    << "lowerMatrixMultiply does not support non-matrix tensors";
  Type targetBlockType = type->getBlockType();
  // Check no blocking or single blocking
  tassert(targetBlockType.isTensor() &&
          (targetBlockType.toTensor()->order() == 0 ||
           targetBlockType.toTensor()->getBlockType().toTensor()->order() == 0))
    << "lowerMatrixMultiply does not support multiply levels of blocking";

  // Build a dense row workspace for per-row reductions
  Var workspace;
  IndexDomain workspaceDomain = type->getDimensions()[1]; // Row domain
  IndexSet outerRowSet = workspaceDomain.getIndexSets()[0];
  // IndexSet outerColSet(1);
  vector<IndexDomain> workspaceDomains =
    targetBlockType.toTensor()->getDimensions();
  if (workspaceDomains.size() >= 2) {
    workspaceDomains[1] = IndexDomain(outerRowSet)*workspaceDomains[1];
  }
  else {
    workspaceDomains = {workspaceDomain};
  }

  ScalarType tensorCType = type->getComponentType();
  Type workspaceType = TensorType::make(tensorCType, workspaceDomains);

  workspace = env->createTemporary(workspaceType, INTERNAL_PREFIX("workspace"));
  env->addTemporary(workspace);
  storage->add(workspace, TensorStorage::Kind::Dense);

  // Identify row and column index variables
  // The index expression is assumed to be exactly a matrix-matrix multiply
  // (i,j B(i,+k)*C(+k,j)) OR (i,j B(+k,j)*C(i,+k))
  // Thus we can assume that the first result var is the row index, and the
  // second is a column index
  iassert(indexExpression->resultVars.size() == 2)
    << "lowerMatrixMuliply does not support non-matrix output";
  const IndexVar& rowVar = indexExpression->resultVars[0];

  // Identify first and second tensors
  const IndexedTensor* firstTensor = nullptr;
  const IndexedTensor* secondTensor = nullptr;
  match(indexExpression->value,
    std::function<void(const IndexedTensor*)>([&](const IndexedTensor* op) {
      iassert(op->indexVars.size() == 2)
        << "lowerMatrixMultiple requires two-index tensors, but got: "
        << Expr(op);
      if (op->indexVars[0].isFreeVar()) {
        firstTensor = op;
      }
      else if (op->indexVars[1].isFreeVar()) {
        secondTensor = op;
      }
      else {
        ierror << "lowerMatrixMultiply requires one free index on tensors, "
               << "but found none: " << Expr(op);
      }
    })
  );
  iassert(firstTensor != nullptr && secondTensor != nullptr);
  tassert(isa<VarExpr>(firstTensor->tensor) &&
          isa<VarExpr>(secondTensor->tensor));
  const Var& firstTensorVar = to<VarExpr>(firstTensor->tensor)->var;
  const Var& secondTensorVar = to<VarExpr>(secondTensor->tensor)->var;
  const TensorType* firstType = firstTensorVar.getType().toTensor();
  const TensorType* secondType = secondTensorVar.getType().toTensor();
  Type firstBlockType = firstType->getBlockType();
  Type secondBlockType = secondType->getBlockType();
  const TensorType* firstBlockTType = firstBlockType.toTensor();
  const TensorType* secondBlockTType = secondBlockType.toTensor();

  // Get outer loop, over the row index variable
  IndexVariableLoop rowLoop(rowVar);
  Var inductionVar = rowLoop.getInductionVar();
  vector<Stmt> headerStmts;
  headerStmts.push_back(VarDecl::make(inductionVar));
  Stmt header = Block::make(headerStmts);

  vector<Stmt> loopStatements;
  // First clear the workspace dense vector
  loopStatements.push_back(AssignStmt::make(workspace, Literal::make(0)));


  // Loop over the indices in this row in the first matrix
  TensorStorage& firstTs = storage->getStorage(firstTensorVar);
  TensorIndex firstTi;
  if (!firstTs.hasTensorIndex(0,1)) { // Tensor index from rows to cols
    if (env->hasExtern(firstTensorVar.getName())) {
      terror << "Extern matrix multiply currently not supported";
    }
    else {
      const pe::PathExpression pexpr = firstTs.getPathExpression();
      if (!env->hasTensorIndex(pexpr)) {
        env->addTensorIndex(pexpr, firstTensorVar);
      }
      firstTi = env->getTensorIndex(pexpr);
    }
  }
  else {
    firstTi = firstTs.getTensorIndex(0,1);
  }
  TensorIndexVar firstIndex(inductionVar.getName(), firstTensorVar.getName(),
                            inductionVar, firstTi);
  vector<Stmt> firstBodyStmts;
  firstBodyStmts.push_back(firstIndex.initSinkVar());

  // Make a variable for the current value in this row of the first matrix
  Var firstVal(firstTensorVar.getName() +
               firstIndex.getCoordVar().getName(),
               firstBlockType);
  firstBodyStmts.push_back(VarDecl::make(firstVal));
  Expr load = Load::make(firstTensor->tensor, firstIndex.getCoordVar());
  if (firstBlockTType->order() != 0) {
    Var dummyIndex("dummyIndex", firstIndex.getCoordVar().getType());
    Stmt dummyInit = AssignStmt::make(dummyIndex, Literal::make(0));
    Stmt firstValStore = Store::make(firstVal, VarExpr::make(dummyIndex), load);
    Stmt block = Block::make(dummyInit, firstValStore);
    block = rewriteToBlocked(block, {firstIndex.getCoordVar(), dummyIndex},
                             (int)firstBlockTType->size());
    firstBodyStmts.push_back(block);
  }
  else {
    firstBodyStmts.push_back(AssignStmt::make(firstVal, load));
  }

  // Loop over the second matrix row, multiplying values with firstVal
  // and reducing into the workspace vectori
  TensorStorage& secondTs = storage->getStorage(secondTensorVar);
  TensorIndex secondTi;
  if (!secondTs.hasTensorIndex(0,1)) { // Tensor index from rows to cols
    if (env->hasExtern(secondTensorVar.getName())) {
      terror << "Extern matrix multiply currently not supported";
    }
    else {
      const pe::PathExpression pexpr = secondTs.getPathExpression();
      if (!env->hasTensorIndex(pexpr)) {
        env->addTensorIndex(pexpr, secondTensorVar);
      }
      secondTi = env->getTensorIndex(pexpr);
    }
  }
  else {
    secondTi = secondTs.getTensorIndex(0,1);
  }
  TensorIndexVar secondIndex(firstIndex.getSinkVar().getName(),
                             secondTensorVar.getName(),
                             firstIndex.getSinkVar(), secondTi);
  vector<Stmt> secondBodyStmts;
  secondBodyStmts.push_back(secondIndex.initSinkVar());

  // Compute and store the inner value into the workspace
  if (firstBlockTType->order() == 0 &&
      secondBlockTType->order() == 0) {
    Expr innerVal = Load::make(secondTensor->tensor, secondIndex.getCoordVar());
    innerVal = Mul::make(VarExpr::make(firstVal), innerVal);
    secondBodyStmts.push_back(
      Store::make(workspace, secondIndex.getSinkVar(),
                  innerVal, CompoundOperator::Add));
  }
  else {
    // TODO: Remove this constraint. We can use this recursion more generally.
    tassert(targetBlockType.toTensor()
            ->getBlockType().toTensor()->order() == 0);
    Expr first, second;
    const TensorType* blockTensorType = targetBlockType.toTensor();
    IndexVar u("u", blockTensorType->getDimensions()[0]);
    IndexVar v("v", blockTensorType->getDimensions()[1]);
    IndexVar w;
    if (firstBlockTType->order() == 0) {
      first = Expr(firstVal);
    }
    else {
      iassert(firstBlockTType->getDimensions()[0] ==
              blockTensorType->getDimensions()[0]);
      w = IndexVar("w", firstBlockTType->getDimensions()[1],
                   ReductionOperator::Sum);
      first = Expr(firstVal)(u,w);
      // HACK: Need to explicitly add storage now, instead of letting this be
      // handled by a backend on VarDecl
      storage->add(firstVal, TensorStorage::Kind::Dense);
    }
    if (secondBlockTType->order() == 0) {
      second = Load::make(secondTensor->tensor, secondIndex.getCoordVar());
    }
    else {
      // Copy block into second value temporary
      Expr innerVal = Load::make(secondTensor->tensor, secondIndex.getCoordVar());
      Var innerValVar(secondTensorVar.getName() +
                      secondIndex.getCoordVar().getName(),
                      secondBlockType);
      secondBodyStmts.push_back(VarDecl::make(innerValVar));
      Var innerDummy("dummyIndexInner", secondIndex.getCoordVar().getType());
      Stmt innerDummyInit = AssignStmt::make(innerDummy, Literal::make(0));
      Stmt innerValStore = Store::make(innerValVar,
                                       Expr(innerDummy), innerVal);
      Stmt block = Block::make(innerDummyInit, innerValStore);
      block = rewriteToBlocked(block, {secondIndex.getCoordVar(), innerDummy},
                               (int)targetBlockType.toTensor()->size());
      secondBodyStmts.push_back(block);
      
      iassert(w.defined());
      second = Expr(innerValVar)(w,v);
      // HACK: Need to explicitly add storage now, instead of letting this be
      // handled by a backend on VarDecl
      storage->add(innerValVar, TensorStorage::Kind::Dense);
    }
    // Evaluate inner-most multiply
    Expr innerVal = IndexExpr::make({u,v}, first*second);
    Var blockOut("blockOut", targetBlockType);
    // Block out is written to as a tensor, so needs an explicit VarDecl
    secondBodyStmts.push_back(VarDecl::make(blockOut));
    // HACK: Need to explicitly add storage now, instead of letting this be
    // handled by a backend on VarDecl
    storage->add(blockOut, TensorStorage::Kind::Dense);
    // TODO: General recursion is probably usable here
    Stmt blockOutWrite = AssignStmt::make(blockOut, innerVal);
    blockOutWrite = lowerIndexStatement(blockOutWrite, env, *storage);
    secondBodyStmts.push_back(blockOutWrite);
    // Copy block into workspace
    Var copyDummy("dummyIndexCopy", secondIndex.getCoordVar().getType());
    Stmt copyDummyInit = AssignStmt::make(copyDummy, Literal::make(0));
    Expr copyLoad = Load::make(blockOut, Expr(copyDummy));
    Stmt copyStore = Store::make(workspace, secondIndex.getSinkVar(),
                                 copyLoad, CompoundOperator::Add);
    Stmt copyBlock = Block::make(copyDummyInit, copyStore);
    copyBlock = rewriteToBlocked(copyBlock, {secondIndex.getSinkVar(), copyDummy},
                                 (int)targetBlockType.toTensor()->size());
    copyBlock = Comment::make("Copy block into workspace", copyBlock, true);
    secondBodyStmts.push_back(copyBlock);
  }

  Stmt secondCoordLoop = ForRange::make(secondIndex.getCoordVar(),
                                        secondIndex.loadCoord(),
                                        secondIndex.loadCoord(1),
                                        Block::make(secondBodyStmts));
  firstBodyStmts.push_back(secondCoordLoop);

  Stmt firstCoordLoop = ForRange::make(firstIndex.getCoordVar(),
                                       firstIndex.loadCoord(),
                                       firstIndex.loadCoord(1),
                                       Block::make(firstBodyStmts));
  loopStatements.push_back(
    Comment::make("Build workspace vector", firstCoordLoop, true));

  // Copy workspace into appropriate output row
  TensorStorage& outTs = storage->getStorage(target);
  TensorIndex outTi;
  if (!outTs.hasTensorIndex(0,1)) { // Tensor index from rows to cols
    if (env->hasExtern(target.getName())) {
      terror << "Extern matrix multiply currently not supported";
    }
    else {
      const pe::PathExpression pexpr = outTs.getPathExpression();
      if (!env->hasTensorIndex(pexpr)) {
        env->addTensorIndex(pexpr, target);
      }
      outTi = env->getTensorIndex(pexpr);
    }
  }
  else {
    outTi = firstTs.getTensorIndex(0,1);
  }
  TensorIndexVar outIndex(indexExpression->resultVars[1].getName(),
                          target.getName(), inductionVar, outTi);
  vector<Stmt> copyBody;
  copyBody.push_back(outIndex.initSinkVar());
  load = Load::make(workspace, outIndex.getSinkVar());
  copyBody.push_back(Store::make(target, outIndex.getCoordVar(), load));
  Stmt copyLoop = ForRange::make(outIndex.getCoordVar(),
                                 outIndex.loadCoord(),
                                 outIndex.loadCoord(1),
                                 Block::make(copyBody));
  loopStatements.push_back(
    Comment::make("Copy workspace into target matrix", copyLoop, true));

  Stmt rowLoopBody = Block::make(loopStatements);
  // Dense loop over the row indices
  const IndexSet &indexSet = rowVar.getDomain().getIndexSets()[0];
  Stmt rowLoopFinal = ForRange::make(inductionVar, 0,
                                     Length::make(indexSet), rowLoopBody);
  Stmt out = Block::make(header, rowLoopFinal);
  return out;
}

}}

