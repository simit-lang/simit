#include "inline.h"

#include <vector>

#include "temps.h"
#include "flatten.h"
#include "intrinsics.h"
#include "ir_codegen.h"

using namespace std;

namespace simit {
namespace ir {

Stmt inlineMapFunction(const Map *map, Var lv, MapFunctionRewriter &rewriter);

Stmt MapFunctionRewriter::inlineMapFunc(const Map *map, Var targetLoopVar,
                                        Var endpoints, Var locs) {
  this->endpoints = endpoints;
  this->locs = locs;
  this->reduction = map->reduction;
  this->targetLoopVar = targetLoopVar;

  Func kernel = map->function;
  // TODO: revise this assert given map functions can have many params
  //iassert(kernel.getArguments().size() == 1 || kernel.getArguments().size() == 2)
  //    << "mapped functions must have exactly two arguments";

  iassert(map->vars.size() == kernel.getResults().size());
  for (size_t i=0; i < kernel.getResults().size(); ++i) {
    resultToMapVar[kernel.getResults()[i]] = map->vars[i];
  }

  this->targetSet = map->target;
  this->neighborSet = map->neighbors;

  iassert(kernel.getArguments().size() >= 1)
      << "The function must have a target argument";

  this->target = kernel.getArguments()[map->partial_actuals.size()];

  if (kernel.getArguments().size() >= (2+map->partial_actuals.size())) {
    this->neighbors = kernel.getArguments()[1+map->partial_actuals.size()];
  }

  return rewrite(kernel.getBody());
}

bool MapFunctionRewriter::isResult(Var var) {
  return resultToMapVar.find(var) != resultToMapVar.end();
}

void MapFunctionRewriter::visit(const FieldWrite *op) {
  // Write a field from the target set
  if (isa<VarExpr>(op->elementOrSet) &&
      to<VarExpr>(op->elementOrSet)->var == target) {
    //iassert(false) << "field from target set";
    Expr setFieldRead = FieldRead::make(targetSet, op->fieldName);
    stmt = TensorWrite::make(setFieldRead, {targetLoopVar}, rewrite(op->value));
  }
  // Write a field from a neighbor set
  else if(isa<TupleRead>(op->elementOrSet) &&
          isa<VarExpr>(to<TupleRead>(op->elementOrSet)->tuple) &&
          to<VarExpr>(to<TupleRead>(op->elementOrSet)->tuple)->var==neighbors) {
    expr = FieldRead::make(neighborSet, op->fieldName);
    Expr setFieldRead = expr;

    Expr index = IRRewriter::rewrite(op->elementOrSet);
    stmt = TensorWrite::make(setFieldRead, {index}, rewrite(op->value));
  }
  else {
    // TODO: Handle the case where the target var was reassigned
    //       tmp = s; ... = tmp.a;
    not_supported_yet;
  }
}


void MapFunctionRewriter::visit(const FieldRead *op) {
  // Read a field from the target set
  if (isa<VarExpr>(op->elementOrSet) &&
      to<VarExpr>(op->elementOrSet)->var == target) {
    Expr setFieldRead = FieldRead::make(targetSet, op->fieldName);
    expr = TensorRead::make(setFieldRead, {targetLoopVar});
  }
  // Read a field from a neighbor set
  else if(isa<TupleRead>(op->elementOrSet) &&
          isa<VarExpr>(to<TupleRead>(op->elementOrSet)->tuple) &&
          to<VarExpr>(to<TupleRead>(op->elementOrSet)->tuple)->var==neighbors) {
    expr = FieldRead::make(neighborSet, op->fieldName);
    Expr setFieldRead = expr;

    Expr index = IRRewriter::rewrite(op->elementOrSet);
    expr = TensorRead::make(setFieldRead, {index});
  }
  else {
    // TODO: Handle the case where the target var was reassigned
    //       tmp = s; ... = tmp.a;
    not_supported_yet;
  }
}

void MapFunctionRewriter::visit(const TupleRead *op) {
  iassert(isa<VarExpr>(op->tuple))
      << "This code assumes no expressions return a tuple";

  if (to<VarExpr>(op->tuple)->var == neighbors) {
    const TupleType *tupleType = op->tuple.type().toTuple();
    int cardinality = tupleType->size;

    Expr endpoints = IndexRead::make(targetSet, IndexRead::Endpoints);
    Expr indexExpr = Add::make(Mul::make(targetLoopVar, cardinality),
                               op->index);
    expr = Load::make(endpoints, indexExpr);
  }
  else {
    ierror << "Assumes tuples are only used for neighbor lists";
  }
}

void MapFunctionRewriter::visit(const VarExpr *op) {
  if (op->var == target) {
    expr = targetLoopVar;
  }
  else if (isResult(op->var)) {
    expr = resultToMapVar[op->var];
  }
  else {
    expr = op;
  }
}

/// Inlines the mapped function with respect to the given loop variable over
/// the target set, using the given rewriter.
Stmt inlineMapFunction(const Map *map, Var lv, MapFunctionRewriter &rewriter) {
  // Compute locations of the mapped edge
  bool returnsMatrix = false;
  for (auto& result : map->function.getResults()) {
    Type type = result.getType();
    if (type.isTensor() && type.toTensor()->order() == 2) {
      returnsMatrix = true;
      break;
    }
  }

  Expr target = map->target;
  iassert(map->target.type().isSet());
  const SetType* setType = map->target.type().toSet();
  int cardinality = setType->endpointSets.size();
  if (returnsMatrix && cardinality > 0) {
    Var i("i", Int);
    Var j("j", Int);

    Var eps("eps", TensorType::make(ScalarType::Int,
                                    {IndexDomain(cardinality)}));
    Expr endpoints = IndexRead::make(target, IndexRead::Endpoints);
    Expr epLoc = Add::make(Mul::make(lv, cardinality), i);
    Expr ep = Load::make(endpoints, epLoc);
    Stmt epsInit = TensorWrite::make(eps, {i}, ep);
    Stmt epsInitLoop = ForRange::make(i, 0, cardinality, epsInit);

    Var locs("locs", TensorType::make(ScalarType::Int,
                                      {IndexDomain(cardinality),
                                       IndexDomain(cardinality)}));

    Expr nbrs_start = IndexRead::make(target, IndexRead::NeighborsStart);
    Expr nbrs = IndexRead::make(target, IndexRead::Neighbors);
    Expr loc = Call::make(intrinsics::loc(), {Load::make(eps,i),
                                              Load::make(eps,j),
                                              nbrs_start, nbrs});
    Stmt locsInit = TensorWrite::make(locs, {i,j}, loc);

    Stmt locsInitLoop = ForRange::make(j, 0, cardinality, locsInit);
    locsInitLoop      = ForRange::make(i, 0, cardinality, locsInitLoop);

    Stmt computeLocs = Block::make({VarDecl::make(eps),
                                    epsInitLoop,
                                    VarDecl::make(locs),
                                    locsInitLoop});

    return Block::make(computeLocs, rewriter.inlineMapFunc(map, lv, eps, locs));
  }
  else {
    return rewriter.inlineMapFunc(map, lv);
  }
}

Stmt inlineMap(const Map *map, MapFunctionRewriter &rewriter) {
  Func kernel = map->function;
  kernel = insertTemporaries(kernel);

  // The function must have at least one argument, namely the target. It may
  // also have a neighbor set, as well as other arguments.
  iassert(kernel.getArguments().size() >= 1)
      << "The function must have a target argument";
  
  Var targetVar = kernel.getArguments()[map->partial_actuals.size()];
  
  Var loopVar(targetVar.getName(), Int);
  ForDomain domain(map->target);

  Stmt inlinedMapFunc = inlineMapFunction(map, loopVar, rewriter);

  Stmt inlinedMap;
  auto initializers = vector<Stmt>();
  for (size_t i=0; i<map->partial_actuals.size(); i++) {
    Var tvar = kernel.getArguments()[i];
    Expr rval = map->partial_actuals[i];
    initializers.push_back(AssignStmt::make(tvar, rval));
  }

  if (initializers.size() > 0) {
    auto initializersBlock = Block::make(initializers);
    inlinedMap = Block::make(initializersBlock,
                             For::make(loopVar, domain, inlinedMapFunc));
  } else {
    inlinedMap = For::make(loopVar, domain, inlinedMapFunc);
  }
  
  if (map->reduction.getKind() != ReductionOperator::Undefined) {
    for (auto &var : map->vars) {
      iassert(var.getType().isTensor());
      Stmt init = AssignStmt::make(var, var);
      init = initializeLhsToZero(init);
      inlinedMap = Block::make(init, inlinedMap);
    }
  }

  // We flatten the statement after it has been inlined, since inlining may
  // introduce additional nested index expressions
  inlinedMap = flattenIndexExpressions(inlinedMap);

  return inlinedMap;
}

}}
