#include "ir_visitor.h"

#include "ir.h"

namespace simit {
namespace ir {

// class IRVisitor
IRVisitor::~IRVisitor() {
}

void IRVisitor::visit(Function *op) {
  for (auto argument : op->getArguments()) {
    argument.accept(this);
  }
  for (auto result : op->getResults()) {
    result.accept(this);
  }
  op->getBody().accept(this);
}

void IRVisitor::visit(Literal *op) {
}

void IRVisitor::visit(Variable *op) {
}

void IRVisitor::visit(Result *op) {
  op->producer.accept(this);
}

void IRVisitor::visit(FieldRead *op) {
  op->elementOrSet.accept(this);
}

void IRVisitor::visit(TensorRead *op) {
  op->tensor.accept(this);
  for (auto &index : op->indices) {
    index.accept(this);
  }
}

void IRVisitor::visit(TupleRead *op) {
  op->tuple.accept(this);
  op->index.accept(this);
}

void IRVisitor::visit(Map *op) {
  op->target.accept(this);
  op->neighbors.accept(this);
}

void IRVisitor::visit(IndexedTensor *op) {
  op->tensor.accept(this);
}

void IRVisitor::visit(IndexExpr *op) {
  op->expr.accept(this);
}

void IRVisitor::visit(Call *op) {
  // TODO
}

void IRVisitor::visit(Neg *op) {
  op->a.accept(this);
}

void IRVisitor::visit(Add *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(Sub *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(Mul *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(Div *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(AssignStmt *op) {
  op->rhs.accept(this);
}

void IRVisitor::visit(FieldWrite *op) {
  op->elementOrSet.accept(this);
  op->value.accept(this);
}

void IRVisitor::visit(TensorWrite *op) {
  op->tensor.accept(this);
  for (auto &index : op->indices) {
    index.accept(this);
  }
  op->value.accept(this);
}

void IRVisitor::visit(For *op) {
  op->body.accept(this);
}

void IRVisitor::visit(IfThenElse *op) {
  op->condition.accept(this);
  op->thenBody.accept(this);
  op->elseBody.accept(this);
}

void IRVisitor::visit(Block *op) {
  op->first.accept(this);
  op->rest.accept(this);
}

void IRVisitor::visit(Pass *op) {
}


// class IRConstVisitor
IRConstVisitor::~IRConstVisitor() {
}

void IRConstVisitor::visit(const Function *op) {
  for (auto &argument : op->getArguments()) {
    argument.accept(this);
  }
  for (auto &result : op->getResults()) {
    result.accept(this);
  }
  op->getBody().accept(this);
}

void IRConstVisitor::visit(const Literal *op) {
}

void IRConstVisitor::visit(const Variable *op) {
}

void IRConstVisitor::visit(const Result *op) {
  op->producer.accept(this);
}

void IRConstVisitor::visit(const FieldRead *op) {
  op->elementOrSet.accept(this);
}

void IRConstVisitor::visit(const TensorRead *op) {
  op->tensor.accept(this);
  for (auto &index : op->indices) {
    index.accept(this);
  }
}

void IRConstVisitor::visit(const TupleRead *op) {
  op->tuple.accept(this);
  op->index.accept(this);
}

void IRConstVisitor::visit(const Map *op) {
  op->target.accept(this);
  op->neighbors.accept(this);
}

void IRConstVisitor::visit(const IndexedTensor *op) {
  op->tensor.accept(this);
}

void IRConstVisitor::visit(const IndexExpr *op) {
  op->expr.accept(this);
}

void IRConstVisitor::visit(const Call *op) {
  // TODO
}

void IRConstVisitor::visit(const Neg *op) {
  op->a.accept(this);
}

void IRConstVisitor::visit(const Add *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRConstVisitor::visit(const Sub *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRConstVisitor::visit(const Mul *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRConstVisitor::visit(const Div *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRConstVisitor::visit(const AssignStmt *op) {
  op->rhs.accept(this);
}

void IRConstVisitor::visit(const FieldWrite *op) {
  op->elementOrSet.accept(this);
  op->value.accept(this);
}

void IRConstVisitor::visit(const TensorWrite *op) {
  op->tensor.accept(this);
  for (auto &index : op->indices) {
    index.accept(this);
  }
  op->value.accept(this);
}

void IRConstVisitor::visit(const For *op) {
  op->body.accept(this);
}

void IRConstVisitor::visit(const IfThenElse *op) {
  op->condition.accept(this);
  op->thenBody.accept(this);
  op->elseBody.accept(this);
}

void IRConstVisitor::visit(const Block *op) {
  op->first.accept(this);
  op->rest.accept(this);
}

void IRConstVisitor::visit(const Pass *op) {
}

}} // namespace simit::ir
