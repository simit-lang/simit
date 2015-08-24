#include "backend_visitor.h"

#include "ir.h"

using namespace simit::ir;

namespace simit {
namespace backend {

void BackendVisitor::visit(const Literal* op) {
  compile(*op);
}

void BackendVisitor::visit(const VarExpr* op) {
  compile(*op);
}

void BackendVisitor::visit(const Load* op) {
  compile(*op);
}

void BackendVisitor::visit(const FieldRead* op) {
  compile(*op);
}

void BackendVisitor::visit(const Call* op) {
  compile(*op);
}

void BackendVisitor::visit(const Length* op) {
  compile(*op);
}

void BackendVisitor::visit(const IndexRead* op) {
  compile(*op);
}

void BackendVisitor::visit(const TensorIndexRead* op) {
  compile(*op);
}

void BackendVisitor::visit(const Neg* op) {
  compile(*op);
}

void BackendVisitor::visit(const Add* op) {
  compile(*op);
}

void BackendVisitor::visit(const Sub* op) {
  compile(*op);
}

void BackendVisitor::visit(const Mul* op) {
  compile(*op);
}

void BackendVisitor::visit(const Div* op) {
  compile(*op);
}

void BackendVisitor::visit(const Not* op) {
  compile(*op);
}

void BackendVisitor::visit(const Eq* op) {
  compile(*op);
}

void BackendVisitor::visit(const Ne* op) {
  compile(*op);
}

void BackendVisitor::visit(const Gt* op) {
  compile(*op);
}

void BackendVisitor::visit(const Lt* op) {
  compile(*op);
}

void BackendVisitor::visit(const Ge* op) {
  compile(*op);
}

void BackendVisitor::visit(const Le* op) {
  compile(*op);
}

void BackendVisitor::visit(const And* op) {
  compile(*op);
}

void BackendVisitor::visit(const Or* op) {
  compile(*op);
}

void BackendVisitor::visit(const Xor* op) {
  compile(*op);
}

void BackendVisitor::visit(const VarDecl* op) {
  compile(*op);
}

void BackendVisitor::visit(const AssignStmt* op) {
  compile(*op);
}

void BackendVisitor::visit(const CallStmt* op) {
  compile(*op);
}

void BackendVisitor::visit(const Store* op) {
  compile(*op);
}

void BackendVisitor::visit(const FieldWrite* op) {
  compile(*op);
}

void BackendVisitor::visit(const Block* op) {
  compile(*op);
}

void BackendVisitor::visit(const IfThenElse* op) {
  compile(*op);
}

void BackendVisitor::visit(const ForRange* op) {
  compile(*op);
}

void BackendVisitor::visit(const For* op) {
  compile(*op);
}

void BackendVisitor::visit(const While* op) {
  compile(*op);
}

void BackendVisitor::visit(const Print* op) {
  compile(*op);
}

void BackendVisitor::visit(const Pass* op) {
  compile(*op);
}

///// High-level IRNodes that should be lowered and never reach the backend
void BackendVisitor::visit(const IndexExpr* op) {
  ierror << "IndexedTensor expressions should never reach the backend: "
         << util::toString(*op);
}

void BackendVisitor::visit(const TensorRead* op) {
  ierror << "TensorRead expressions should never reach a backend: "
         << util::toString(*op);
}

void BackendVisitor::visit(const TupleRead* op) {
  ierror << "TupleRead expressions should never reach the backend: "
         << util::toString(*op);
}

void BackendVisitor::visit(const IndexedTensor* op) {
  ierror << "IndexedTensor expressions should never reach the backend: "
         << util::toString(*op);
}

void BackendVisitor::visit(const Map* op) {
  ierror << "Map statements should never reach the backend: "
         << util::toString(*op);
}

void BackendVisitor::visit(const TensorWrite* op) {
  ierror << "TensorWrite statements should never reach the backend: "
         << util::toString(*op);
}

}}
