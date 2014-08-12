#include "irvisitors.h"

#include "ir.h"

using namespace simit::internal;

#define CHECK_ABORT(func) do { func; if (isAborted()) return; } while(0)

IRVisitor::~IRVisitor() {
}

void IRVisitor::visit(Function *f) {
  CHECK_ABORT(handle(f));
  for (auto &result : f->getResults()) {
    CHECK_ABORT(result->accept(this));
  }
}

void IRVisitor::visit(LiteralTensor *t) {
  handle(t);
}

void IRVisitor::visit(Argument *t) {
  handle(t);
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

void IRVisitor::visit(VariableStore *t) {
  // TODO: Implement visitation
  CHECK_ABORT(handle(t));
}
