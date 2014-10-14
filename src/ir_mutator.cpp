#include "ir_mutator.h"

#include <vector>

namespace simit {
namespace ir {

Expr IRMutator::mutate(Expr e) {
  if (e.defined()) {
    e.accept(this);
    e = expr;
  }
  else {
    e = Expr();
  }
  expr = Expr();
  stmt = Stmt();
  return e;
}

Stmt IRMutator::mutate(Stmt s) {
  if (s.defined()) {
    s.accept(this);
    s = stmt;
  }
  else {
    s = Stmt();
  }
  expr = Expr();
  stmt = Stmt();
  return s;
}

Func IRMutator::mutate(Func f) {
  if (f.defined()) {
    f.accept(this);
    f = func;
  }
  else {
    f = Func();
  }
  expr = Expr();
  stmt = Stmt();
  func = Func();
  return f;
}

void IRMutator::visit(const Literal *op) {
  expr = op;
}

void IRMutator::visit(const Variable *op) {
  expr = op;
}

void IRMutator::visit(const Result *op) {
  expr = op;
}

void IRMutator::visit(const FieldRead *op) {
  Expr elementOrSet = mutate(op->elementOrSet);
  if (elementOrSet == op->elementOrSet) {
    expr = op;
  }
  else {
    expr = FieldRead::make(elementOrSet, op->fieldName);
  }
}

void IRMutator::visit(const TensorRead *op) {
  Expr tensor = mutate(op->tensor);
  std::vector<Expr> indices(op->indices.size());
  bool indicesSame = true;
  for (size_t i=0; i < op->indices.size(); ++i) {
    indices[i] = mutate(op->indices[i]);
    if (indices[i] != op->indices[i]) {
      indicesSame = false;
    }
  }
  if (tensor == op->tensor && indicesSame) {
    expr = op;
  }
  else {
    expr = TensorRead::make(tensor, indices);
  }
}

void IRMutator::visit(const TupleRead *op) {
  Expr tuple = mutate(op->tuple);
  Expr index = mutate(op->index);
  if (tuple == op->tuple && index == op->index) {
    expr = op;
  }
  else {
    expr = TupleRead::make(tuple, index);
  }
}

void IRMutator::visit(const Map *op) {
  Expr target = mutate(op->target);
  Expr neighbors = mutate(op->neighbors);
  if (target == op->target && neighbors == op->neighbors) {
    expr = op;
  }
  else {
    expr = Map::make(op->function, target, neighbors, op->reductionOp);
  }
}

void IRMutator::visit(const IndexedTensor *op) {
  Expr tensor = mutate(op->tensor);
  if (tensor == op->tensor) {
    expr = op;
  }
  else {
    expr = IndexedTensor::make(tensor, op->indexVars);
  }
}

void IRMutator::visit(const IndexExpr *op) {
  Expr rhs = mutate(op->rhs);
  if (rhs == op->rhs) {
    expr = op;
  }
  else {
    expr = IndexExpr::make(op->lhsIndexVars, rhs);
  }
}

void IRMutator::visit(const Call *op) {
  std::vector<Expr> actuals(op->actuals.size());
  bool actualsSame = true;
  for (size_t i=0; i < op->actuals.size(); ++i) {
    actuals[i] = mutate(op->actuals[i]);
    if (actuals[i] != op->actuals[i]) {
      actualsSame = false;
    }
  }
  if (actualsSame) {
    expr = op;
  }
  else {
    expr = Call::make(op->function, actuals, op->kind);
  }
}

void IRMutator::visit(const Neg *op) {
  Expr a = mutate(op->a);
  if (a == op->a) {
    expr = op;
  }
  else {
    expr = Neg::make(a);
  }
}

template <class T>
Expr visitBinaryOp(const T *op, IRMutator &mut) {
  Expr a = mut.mutate(op->a);
  Expr b = mut.mutate(op->b);
  if (a == op->a && b == op->b) {
    return op;
  }
  else {
    return T::make(a, b);
  }
}

void IRMutator::visit(const Add *op) {
  expr = visitBinaryOp(op, *this);
}

void IRMutator::visit(const Sub *op) {
  expr = visitBinaryOp(op, *this);
}

void IRMutator::visit(const Mul *op) {
  expr = visitBinaryOp(op, *this);
}

void IRMutator::visit(const Div *op) {
  expr = visitBinaryOp(op, *this);
}


void IRMutator::visit(const AssignStmt *op) {
  Expr rhs = mutate(op->rhs);
  if (rhs == op->rhs) {
    stmt = op;
  }
  else {
    stmt = AssignStmt::make(op->lhs, rhs);
  }
}

void IRMutator::visit(const FieldWrite *op) {
  Expr elementOrSet = mutate(op->elementOrSet);
  Expr value = mutate(op->value);
  if (elementOrSet == op->elementOrSet && value == op->value) {
    stmt = op;
  }
  else {
    stmt = FieldWrite::make(elementOrSet, op->fieldName, value);
  }
}

void IRMutator::visit(const TensorWrite *op) {
  Expr tensor = mutate(op->tensor);
  Expr value = mutate(op->value);
  std::vector<Expr> indices(op->indices.size());
  bool indicesSame = true;
  for (size_t i=0; i < op->indices.size(); ++i) {
    indices[i] = mutate(op->indices[i]);
    if (indices[i] != op->indices[i]) {
      indicesSame = false;
    }
  }
  if (tensor == op->tensor && indicesSame && value == op->value) {
    stmt = op;
  }
  else {
    stmt = TensorWrite::make(tensor, indices, value);
  }
}

void IRMutator::visit(const For *op) {
  Stmt body = mutate(op->body);
  if (body == op->body) {
    stmt = op;
  }
  else {
    stmt = For::make(op->name, op->domain, body);
  }
}

void IRMutator::visit(const IfThenElse *op) {
  Expr condition = mutate(op->condition);
  Stmt thenBody = mutate(op->thenBody);
  Stmt elseBody = mutate(op->elseBody);
  if (condition == op->condition && thenBody == op->thenBody &&
      elseBody == op->elseBody) {
    stmt = op;
  }
  else {
    stmt = IfThenElse::make(condition, thenBody, elseBody);
  }
}

void IRMutator::visit(const Block *op) {
  Stmt first = mutate(op->first);
  Stmt rest = mutate(op->rest);
  if (first == op->first && rest == op->rest) {
    stmt = op;
  }
  else {
    stmt = Block::make(first, rest);
  }
}

void IRMutator::visit(const Pass *op) {
  stmt = op;
}

void IRMutator::visit(const Func *f) {
  std::vector<Expr> arguments(f->getArguments().size());
  std::vector<Expr> results(f->getResults().size());

  bool argumentsSame = true;
  for (size_t i=0; i < f->getArguments().size(); ++i) {
    arguments[i] = mutate(f->getArguments()[i]);
    if (arguments[i] != f->getArguments()[i]) {
      argumentsSame = false;
    }
  }

  for (size_t i=0; i < f->getResults().size(); ++i) {
    results[i] = mutate(f->getResults()[i]);
    if (results[i] != f->getResults()[i]) {
      argumentsSame = false;
    }
  }

  Stmt body = mutate(f->getBody());

  if (body == f->getBody() && argumentsSame) {
    func = *f;
  }
  else {
    func = Func(f->getName(), arguments, results, body);
  }
}

}} // namespace simit::ir