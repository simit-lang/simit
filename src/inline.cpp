#include "inline.h"

#include <vector>

#include "temps.h"
#include "flatten.h"

using namespace std;

namespace simit {
namespace ir {

Stmt MapFunctionRewriter::inlineMapFunc(const Map *map, Var targetLoopVar) {
  this->targetLoopVar = targetLoopVar;

  Func kernel = map->function;
  iassert(kernel.getArguments().size() == 1 || kernel.getArguments().size() == 2)
      << "mapped functions must have exactly two arguments";

  iassert(map->vars.size() == kernel.getResults().size());
  for (size_t i=0; i < kernel.getResults().size(); ++i) {
    resultToMapVar[kernel.getResults()[i]] = map->vars[i];
  }

  this->targetSet = map->target;
  this->neighborSet = map->neighbors;

  iassert(kernel.getArguments().size() >= 1)
      << "The function must have a target argument";

  this->target = kernel.getArguments()[0];

  if (kernel.getArguments().size() >= 2) {
    this->neighbors = kernel.getArguments()[1];
  }

  return rewrite(kernel.getBody());
}

bool MapFunctionRewriter::isResult(Var var) {
  return resultToMapVar.find(var) != resultToMapVar.end();
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
  if (isResult(op->var)) {
    expr = resultToMapVar[op->var];
  }
  else {
    expr = op;
  }
}

/// Inlines the mapped function with respect to the given loop variable over
/// the target set, using the given rewriter.
Stmt inlineMapFunction(const Map *map, Var lv, MapFunctionRewriter &rewriter) {
  return rewriter.inlineMapFunc(map, lv);
}

Stmt inlineMap(const Map *map, MapFunctionRewriter &rewriter) {
  Func kernel = map->function;
  kernel = insertTemporaries(kernel);

  // The function must have at least one argument, namely the target. It may
  // also have a neighbor set, as well as other arguments.
  iassert(kernel.getArguments().size() >= 1)
      << "The function must have a target argument";

  Var targetVar = kernel.getArguments()[0];
  Var neighborsVar = kernel.getArguments()[1];

  Var loopVar(targetVar.getName(), Int);
  ForDomain domain(map->target);

  Stmt inlinedMapFunc = inlineMapFunction(map, loopVar, rewriter);
  Stmt inlinedMap = For::make(loopVar, domain, inlinedMapFunc);

  for (auto &var : map->vars) {
    iassert(var.getType().isTensor());
    const TensorType *type = var.getType().toTensor();
    Expr zero = Literal::make(TensorType::make(type->componentType), {0});
    Stmt init = AssignStmt::make(var, zero);
    inlinedMap = Block::make(init, inlinedMap);
  }

  // We flatten the statement after it has been inlined, since inlining may
  // introduce additional nested index expressions
  inlinedMap = flattenIndexExpressions(inlinedMap);

  return inlinedMap;
}

}}
