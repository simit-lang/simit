#include "sir_visitor.h"

#include "sir.h"

namespace simit {
namespace ir {

SetIRVisitor::~SetIRVisitor() {
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
  for (auto &stmt : op->stmts) {
    stmt.accept(this);
  }
}

void SetIRVisitor::visit(Foreach *op) {
  op->body.accept(this);
}

void SetIRVisitor::visit(Store *op) {
  op->index.accept(this);
  op->value.accept(this);
}

void SetIRConstVisitor::visit(const Variable *) {

}


// class SetIRConstVisitor
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
  for (auto &stmt : op->stmts) {
    stmt.accept(this);
  }
}

void SetIRConstVisitor::visit(const Foreach *op) {
  op->body.accept(this);
}

void SetIRConstVisitor::visit(const Store *op) {
  op->index.accept(this);
  op->value.accept(this);
}

}} // namespace simit::ir
