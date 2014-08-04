#include "irvisitors.h"

using namespace simit;

IRVisitor::~IRVisitor() {
}

void IRVisitor::visit(Function *f) {
  handle(f);
}

void IRVisitor::visit(LiteralTensor *t) {
  handle(t);
}

void IRVisitor::visit(Argument *t) {
  handle(t);
}

void IRVisitor::visit(Result *t) {
  handle(t);
}

void IRVisitor::visit(Merge *t) {
  handle(t);
}

void IRVisitor::visit(VariableStore *t) {
  handle(t);
}

void IRVisitor::handle(Function *function) {

}

void IRVisitor::handle(LiteralTensor *t) {

}

void IRVisitor::handle(Argument *t) {

}

void IRVisitor::handle(Result *t) {

}

void IRVisitor::handle(Merge *t) {

}

void IRVisitor::handle(VariableStore *t) {

}

void IRVisitor::visit(const Function &f) {
  handle(f);
}

void IRVisitor::visit(const Argument &t) {
  handle(t);
}

void IRVisitor::visit(const Result &t) {
  handle(t);
}

void IRVisitor::visit(const LiteralTensor &t) {
  handle(t);
}

void IRVisitor::visit(const Merge &t) {
  handle(t);
}

void IRVisitor::visit(const VariableStore &t) {
  handle(t);
}

void IRVisitor::handle(const Function &f) {

}

void IRVisitor::handle(const Argument &t) {

}

void IRVisitor::handle(const Result &t) {

}

void IRVisitor::handle(const LiteralTensor &t) {

}

void IRVisitor::handle(const Merge &t) {

}

void IRVisitor::handle(const VariableStore &t) {

}
