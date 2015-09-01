#include "backend_visitor.h"

#include "ir.h"

using namespace simit::ir;

namespace simit {
namespace backend {

void BackendVisitorBase::accept(const Expr& expr) {
  expr.accept(this);
}

void BackendVisitorBase::accept(const Stmt& stmt) {
  stmt.accept(this);
}

void BackendVisitorBase::visitError(std::string type, const void* op) {
  const IRNode* ir = static_cast<const IRNode*>(op);
  ierror << type << " should never reach the backend: " << util::toString(*ir);
}

void BackendVisitorBase::compile(const ir::Kernel& kernel) {
  not_supported_yet;
}

void BackendVisitorBase::compile(const ir::Block& op) {
  op.first.accept(this);
  if (op.rest.defined()) {
    op.rest.accept(this);
  }
}

void BackendVisitorBase::compile(const ir::Comment& comment) {
  comment.commentedStmt.accept(this);
}

}}
