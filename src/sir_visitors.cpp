#include "sir_visitors.h"

#include "sir.h"

namespace simit {
namespace ir {

SetIRVisitor::~SetIRVisitor() {
}

void SetIRVisitor::visit(IntLiteral *) {
}

void SetIRVisitor::visit(Variable *) {
}

void SetIRVisitor::visit(Load *op) {
  op->index.accept(this);
}

void SetIRVisitor::visit(Neg *op) {
  op->a.accept(this);
}

void SetIRVisitor::visit(Add *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void SetIRVisitor::visit(Sub *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void SetIRVisitor::visit(Mul *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void SetIRVisitor::visit(Div *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void SetIRVisitor::visit(Block *op) {
  op->first.accept(this);
  if (op->rest.defined()) {
    op->rest.accept(this);
  }
}

void SetIRVisitor::visit(Foreach *op) {
  op->body.accept(this);
}

void SetIRVisitor::visit(Store *op) {
  op->index.accept(this);
  op->value.accept(this);
}

void SetIRVisitor::visit(StoreMatrix *op) {
  op->target.accept(this);
  op->row.accept(this);
  op->col.accept(this);
  op->value.accept(this);
}

void SetIRVisitor::visit(Pass *) {
}

// class SetIRConstVisitor
void SetIRConstVisitor::visit(const IntLiteral *) {
}

void SetIRConstVisitor::visit(const Variable *) {
}

SetIRConstVisitor::~SetIRConstVisitor() {
}

void SetIRConstVisitor::visit(const Load *op) {
  op->index.accept(this);
}

void SetIRConstVisitor::visit(const Neg *op) {
  op->a.accept(this);
}

void SetIRConstVisitor::visit(const Add *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void SetIRConstVisitor::visit(const Sub *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void SetIRConstVisitor::visit(const Mul *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void SetIRConstVisitor::visit(const Div *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void SetIRConstVisitor::visit(const Block *op) {
  op->first.accept(this);
  if (op->rest.defined()) {
    op->rest.accept(this);
  }
}

void SetIRConstVisitor::visit(const Foreach *op) {
  op->body.accept(this);
}

void SetIRConstVisitor::visit(const Store *op) {
  op->index.accept(this);
  op->value.accept(this);
}

void SetIRConstVisitor::visit(const StoreMatrix *op) {
  op->target.accept(this);
  op->row.accept(this);
  op->col.accept(this);
  op->value.accept(this);
}

void SetIRConstVisitor::visit(const Pass *) {
}

}} // namespace simit::ir
