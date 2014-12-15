#include "inline.h"

namespace simit {
namespace ir {

InlineMappedFunction::InlineMappedFunction(const Map *map, Var targetLoopVar)
    : targetLoopVar(targetLoopVar) {
  Func func = map->function;
  iassert(func.getArguments().size() == 1 || func.getArguments().size() == 2)
      << "mapped functions must have exactly two arguments";

  iassert(map->vars.size() == func.getResults().size());
  for (size_t i=0; i < func.getResults().size(); ++i) {
    resultToMapVar[func.getResults()[i]] = map->vars[i];
  }

  this->targetSet = map->target;
  this->neighborSet = map->neighbors;

  this->target = func.getArguments()[0];
  this->neighbors = func.getArguments()[1];
}

void InlineMappedFunction::visit(const FieldRead *op) {
  if (isa<VarExpr>(op->elementOrSet) &&
      to<VarExpr>(op->elementOrSet)->var == target) {
    Expr setFieldRead = FieldRead::make(targetSet, op->fieldName);
    expr = TensorRead::make(setFieldRead, {targetLoopVar});
  }
  else if(isa<TupleRead>(op->elementOrSet) &&
          isa<VarExpr>(to<TupleRead>(op->elementOrSet)->tuple) &&
          to<VarExpr>(to<TupleRead>(op->elementOrSet)->tuple)->var==neighbors) {
    Expr setFieldRead = FieldRead::make(neighborSet, op->fieldName);
    expr = setFieldRead;

    Expr index = IRRewriter::rewrite(op->elementOrSet);
    expr = TensorRead::make(setFieldRead, {index});
  }
  else {
    // TODO: Handle the case where the target var was reassigned
    //       tmp = s; ... = tmp.a;
    not_supported_yet;
  }
}

void InlineMappedFunction::visit(const TupleRead *op) {
  iassert(isa<VarExpr>(op->tuple))
      << "This code assumes no expressions return a tuple";

  if (to<VarExpr>(op->tuple)->var == neighbors) {
    const TupleType *tupleType = op->tuple.type().toTuple();
    int cardinality = tupleType->size;

    Expr endpoints = IndexRead::make(targetSet, "endpoints");
    Expr indexExpr = Add::make(Mul::make(targetLoopVar, cardinality),
                               op->index);
    expr = Load::make(endpoints, indexExpr);
  }
  else {
    ierror << "Assumes tuples are only used for neighbor lists";
  }
}

void InlineMappedFunction::visit(const VarExpr *op) {
  if (resultToMapVar.find(op->var) != resultToMapVar.end()) {
    expr = resultToMapVar[op->var];
  }
  else {
    expr = op;
  }
}

}}
