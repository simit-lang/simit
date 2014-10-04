#include "ir_visitors.h"

#include "ir.h"

namespace simit {
namespace ir {

#define CHECK_ABORT(func) do { func; if (isAborted()) return; } while(0)

// class IRVisitor
IRVisitor::~IRVisitor() {
}

void IRVisitor::visit(Function *f) {
  CHECK_ABORT(handle(f));
  for (auto &result : f->getResults()) {
    CHECK_ABORT(result->accept(this));
  }
}

void IRVisitor::visit(Literal *t) {
  CHECK_ABORT(handle(t));
}

void IRVisitor::visit(Argument *t) {
  CHECK_ABORT(handle(t));
}

void IRVisitor::visit(Result *t) {
  if (t->getValue() == NULL) {  // TODO: Remove check
    abort();
    return;
  }
  CHECK_ABORT(t->getValue()->accept(this));
  CHECK_ABORT(handle(t));
}

void IRVisitor::visit(IndexExpr *t) {
  for (auto &operand : t->getOperands()) {
    CHECK_ABORT(operand.getTensor()->accept(this));
  }
  CHECK_ABORT(handle(t));
}

void IRVisitor::visit(Call *t) {
  CHECK_ABORT(handle(t));
}

void IRVisitor::visit(FieldRead *t) {
  CHECK_ABORT(t->getSet()->accept(this));
  CHECK_ABORT(handle(t));
}

void IRVisitor::visit(FieldWrite *t) {
  CHECK_ABORT(t->getValue()->accept(this));
  CHECK_ABORT(handle(t));
}

void IRVisitor::visit(TensorRead *t) {
  CHECK_ABORT(t->getTensor()->accept(this));
  for (auto &index : t->getIndices()) {
    CHECK_ABORT(index->accept(this));
  }
  CHECK_ABORT(handle(t));
}

void IRVisitor::visit(TensorWrite *t) {
  CHECK_ABORT(t->getValue()->accept(this));
  for (auto &index : t->getIndices()) {
    CHECK_ABORT(index->accept(this));
  }
  CHECK_ABORT(handle(t));
}

void IRVisitor::handle(Function *f) {
  handleDefault(f);
}

void IRVisitor::handle(Argument *t) {
  handleDefault(t);
}

void IRVisitor::handle(Result *t) {
  handleDefault(t);
}

void IRVisitor::handle(Literal *t) {
  handleDefault(t);
}

void IRVisitor::handle(IndexExpr *t) {
  handleDefault(t);
}

void IRVisitor::handle(Call *t) {
  handleDefault(t);
}

void IRVisitor::handle(FieldRead *t) {
  handleDefault(t);
}

void IRVisitor::handle(FieldWrite *t) {
  handleDefault(t);
}

void IRVisitor::handle(TensorRead *t) {
  handleDefault(t);
}

void IRVisitor::handle(TensorWrite *t) {
  handleDefault(t);
}


// class IRBackwardVisitor
IRBackwardVisitor::~IRBackwardVisitor() {
  
}

void IRBackwardVisitor::visit(Function *f) {
  CHECK_ABORT(handle(f));
  for (auto &result : f->getResults()) {
    CHECK_ABORT(result->accept(this));
  }
}

void IRBackwardVisitor::visit(Literal *t) {
  CHECK_ABORT(handle(t));
}

void IRBackwardVisitor::visit(Argument *t) {
  CHECK_ABORT(handle(t));
}

void IRBackwardVisitor::visit(Result *t) {
  if (t->getValue() == NULL) {  // TODO: Remove check
    abort();
    return;
  }
  CHECK_ABORT(handle(t));
  CHECK_ABORT(t->getValue()->accept(this));
}

void IRBackwardVisitor::visit(IndexExpr *t) {
  CHECK_ABORT(handle(t));
  for (auto &operand : t->getOperands()) {
    CHECK_ABORT(operand.getTensor()->accept(this));
  }
}

void IRBackwardVisitor::visit(Call *t) {
  CHECK_ABORT(handle(t));
}

void IRBackwardVisitor::visit(FieldRead *t) {
  CHECK_ABORT(handle(t));
  CHECK_ABORT(t->getSet()->accept(this));
}

void IRBackwardVisitor::visit(FieldWrite *t) {
  CHECK_ABORT(handle(t));
  CHECK_ABORT(t->getValue()->accept(this));
}

void IRBackwardVisitor::visit(TensorRead *t) {
  CHECK_ABORT(handle(t));
  CHECK_ABORT(t->getTensor()->accept(this));
  for (auto &index : t->getIndices()) {
    CHECK_ABORT(index->accept(this));
  }
}

void IRBackwardVisitor::visit(TensorWrite *t) {
  CHECK_ABORT(handle(t));
  CHECK_ABORT(t->getValue()->accept(this));
  for (auto &index : t->getIndices()) {
    CHECK_ABORT(index->accept(this));
  }
}


}} // namespace simit::ir
