#include "ir_rewriter.h"

#include <vector>

namespace simit {
namespace ir {

Expr IRRewriter::mutate(Expr e) {
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

Stmt IRRewriter::mutate(Stmt s) {
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

Func IRRewriter::mutate(Func f) {
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

void IRRewriter::visit(const Literal *op) {
  expr = op;
}

void IRRewriter::visit(const VarExpr *op) {
  expr = op;
}

void IRRewriter::visit(const Result *op) {
  expr = op;
}

void IRRewriter::visit(const FieldRead *op) {
  Expr elementOrSet = mutate(op->elementOrSet);
  if (elementOrSet == op->elementOrSet) {
    expr = op;
  }
  else {
    expr = FieldRead::make(elementOrSet, op->fieldName);
  }
}

void IRRewriter::visit(const TensorRead *op) {
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

void IRRewriter::visit(const TupleRead *op) {
  Expr tuple = mutate(op->tuple);
  Expr index = mutate(op->index);
  if (tuple == op->tuple && index == op->index) {
    expr = op;
  }
  else {
    expr = TupleRead::make(tuple, index);
  }
}

void IRRewriter::visit(const IndexRead *op) {
  Expr edgeSet = mutate(op->edgeSet);
  if (edgeSet == op->edgeSet) {
    expr = op;
  }
  else {
    expr = IndexRead::make(edgeSet, op->indexName);
  }
}

void IRRewriter::visit(const Length *op) {
  expr = op;
}

void IRRewriter::visit(const Load *op) {
  Expr buffer = mutate(op->buffer);
  Expr index = mutate(op->index);
  if (buffer == op->buffer && index == op->index) {
    expr = op;
  }
  else {
    expr = Load::make(buffer, index);
  }
}

void IRRewriter::visit(const IndexedTensor *op) {
  Expr tensor = mutate(op->tensor);
  if (tensor == op->tensor) {
    expr = op;
  }
  else {
    expr = IndexedTensor::make(tensor, op->indexVars);
  }
}

void IRRewriter::visit(const IndexExpr *op) {
  Expr value = mutate(op->value);
  if (value == op->value) {
    expr = op;
  }
  else {
    expr = IndexExpr::make(op->resultVars, value);
  }
}

void IRRewriter::visit(const Call *op) {
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
    expr = Call::make(op->func, actuals);
  }
}

void IRRewriter::visit(const Neg *op) {
  Expr a = mutate(op->a);
  if (a == op->a) {
    expr = op;
  }
  else {
    expr = Neg::make(a);
  }
}

template <class T>
Expr visitBinaryOp(const T *op, IRRewriter &mut) {
  Expr a = mut.mutate(op->a);
  Expr b = mut.mutate(op->b);
  if (a == op->a && b == op->b) {
    return op;
  }
  else {
    return T::make(a, b);
  }
}

void IRRewriter::visit(const Add *op) {
  expr = visitBinaryOp(op, *this);
}

void IRRewriter::visit(const Sub *op) {
  expr = visitBinaryOp(op, *this);
}

void IRRewriter::visit(const Mul *op) {
  expr = visitBinaryOp(op, *this);
}

void IRRewriter::visit(const Div *op) {
  expr = visitBinaryOp(op, *this);
}


void IRRewriter::visit(const AssignStmt *op) {
  Expr value = mutate(op->value);
  if (value == op->value) {
    stmt = op;
  }
  else {
    stmt = AssignStmt::make(op->var, value);
  }
}

void IRRewriter::visit(const Map *op) {
  Expr target = mutate(op->target);
  Expr neighbors = mutate(op->neighbors);
  if (target == op->target && neighbors == op->neighbors) {
    stmt = op;
  }
  else {
    stmt = Map::make(op->vars, op->function, target, neighbors, op->reduction);
  }
}


void IRRewriter::visit(const FieldWrite *op) {
  Expr elementOrSet = mutate(op->elementOrSet);
  Expr value = mutate(op->value);
  if (elementOrSet == op->elementOrSet && value == op->value) {
    stmt = op;
  }
  else {
    stmt = FieldWrite::make(elementOrSet, op->fieldName, value);
  }
}

void IRRewriter::visit(const TensorWrite *op) {
  Expr tensor = mutate(op->tensor);
  std::vector<Expr> indices(op->indices.size());
  bool indicesSame = true;
  for (size_t i=0; i < op->indices.size(); ++i) {
    indices[i] = mutate(op->indices[i]);
    if (indices[i] != op->indices[i]) {
      indicesSame = false;
    }
  }
  Expr value = mutate(op->value);
  if (tensor == op->tensor && indicesSame && value == op->value) {
    stmt = op;
  }
  else {
    stmt = TensorWrite::make(tensor, indices, value);
  }
}

void IRRewriter::visit(const Store *op) {
  Expr buffer = mutate(op->buffer);
  Expr index = mutate(op->index);
  Expr value = mutate(op->value);
  if (buffer == op->buffer && op->index == index && value == op->value) {
    stmt = op;
  }
  else {
    stmt = Store::make(buffer, index, value);
  }
}

void IRRewriter::visit(const For *op) {
  Stmt body = mutate(op->body);
  if (body == op->body) {
    stmt = op;
  }
  else {
    stmt = For::make(op->var, op->domain, body);
  }
}

void IRRewriter::visit(const IfThenElse *op) {
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

void IRRewriter::visit(const Block *op) {
  Stmt first = mutate(op->first);
  Stmt rest = mutate(op->rest);
  if (first == op->first && rest == op->rest) {
    stmt = op;
  }
  else {
    if (first.defined() && rest.defined()) {
      stmt = Block::make(first, rest);
    }
    else if (first.defined() && !rest.defined()) {
      stmt = first;
    }
    else if (!first.defined() && rest.defined()) {
      stmt = rest;
    }
    else {
      stmt = Stmt();
    }
  }
}

void IRRewriter::visit(const Pass *op) {
  stmt = op;
}

void IRRewriter::visit(const Func *f) {
  Stmt body = mutate(f->getBody());

  if (body == f->getBody()) {
    func = *f;
  }
  else {
    if (!body.defined()) {
      body = Pass::make();
    }

    func = Func(f->getName(), f->getArguments(), f->getResults(), body,
                f->getKind(), f->getTemporaries());
  }
}

}} // namespace simit::ir