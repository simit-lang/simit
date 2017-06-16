#include "backend_visitor.h"

#include "ir.h"
#ifdef GPU
#include "backend/gpu/gpu_ir.h"
#endif

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

#ifdef GPU
void BackendVisitorBase::compile(const ir::GPUKernel& kernel) {
  not_supported_yet;
}
#endif

void BackendVisitorBase::compile(const ir::Block& op) {
  for (Stmt stmt : op.stmts) {
    stmt.accept(this);
  }
}

void BackendVisitorBase::compile(const ir::Comment& comment) {
  comment.commentedStmt.accept(this);
}

}}
