#include "ir_visitor.h"

#include "ir.h"

#ifdef GPU
#include "backend/gpu/gpu_ir.h"
#endif

namespace simit {
namespace ir {

// class IRVisitor
IRVisitor::~IRVisitor() {
}

void IRVisitor::visit(const Literal *op) {
}

void IRVisitor::visit(const VarExpr *op) {
}

void IRVisitor::visit(const FieldRead *op) {
  op->elementOrSet.accept(this);
}

void IRVisitor::visit(const IndexRead *op) {
  op->edgeSet.accept(this);
}

void IRVisitor::visit(const Length *op) {
  if (op->indexSet.getKind() == IndexSet::Set) {
    op->indexSet.getSet().accept(this);
  }
}

void IRVisitor::visit(const Load *op) {
  op->buffer.accept(this);
  op->index.accept(this);
}

void IRVisitor::visit(const UnaryExpr* op) {
  op->a.accept(this);
}

void IRVisitor::visit(const BinaryExpr* op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(const Neg *op) {
  visit(static_cast<const UnaryExpr*>(op));
}

void IRVisitor::visit(const Add *op) {
  visit(static_cast<const BinaryExpr*>(op));
}

void IRVisitor::visit(const Sub *op) {
  visit(static_cast<const BinaryExpr*>(op));
}

void IRVisitor::visit(const Mul *op) {
  visit(static_cast<const BinaryExpr*>(op));
}

void IRVisitor::visit(const Div *op) {
  visit(static_cast<const BinaryExpr*>(op));
}

void IRVisitor::visit(const Rem *op) {
  visit(static_cast<const BinaryExpr*>(op));
}

void IRVisitor::visit(const Not *op) {
  visit(static_cast<const UnaryExpr*>(op));
}

void IRVisitor::visit(const Eq *op) {
  visit(static_cast<const BinaryExpr*>(op));
}

void IRVisitor::visit(const Ne *op) {
  visit(static_cast<const BinaryExpr*>(op));
}

void IRVisitor::visit(const Gt *op) {
  visit(static_cast<const BinaryExpr*>(op));
}

void IRVisitor::visit(const Lt *op) {
  visit(static_cast<const BinaryExpr*>(op));
}

void IRVisitor::visit(const Ge *op) {
  visit(static_cast<const BinaryExpr*>(op));
}

void IRVisitor::visit(const Le *op) {
  visit(static_cast<const BinaryExpr*>(op));
}

void IRVisitor::visit(const And *op) {
  visit(static_cast<const BinaryExpr*>(op));
}

void IRVisitor::visit(const Or *op) {
  visit(static_cast<const BinaryExpr*>(op));
}

void IRVisitor::visit(const Xor *op) {
  visit(static_cast<const BinaryExpr*>(op));
}

void IRVisitor::visit(const VarDecl *op) {
}

void IRVisitor::visit(const AssignStmt *op) {
  op->value.accept(this);
}

void IRVisitor::visit(const CallStmt *op) {
  for (auto &actual : op->actuals) {
    actual.accept(this);
  }
}

void IRVisitor::visit(const Store *op) {
  op->buffer.accept(this);
  op->index.accept(this);
  op->value.accept(this);
}


void IRVisitor::visit(const FieldWrite *op) {
  op->elementOrSet.accept(this);
  op->value.accept(this);
}

void IRVisitor::visit(const Scope* op) {
  op->scopedStmt.accept(this);
}

void IRVisitor::visit(const IfThenElse *op) {
  op->condition.accept(this);
  op->thenBody.accept(this);
  if (op->elseBody.defined()) {
    op->elseBody.accept(this);
  }
}

void IRVisitor::visit(const ForRange *op) {
  op->start.accept(this);
  op->end.accept(this);
  op->body.accept(this);
}

void IRVisitor::visit(const For *op) {
  op->body.accept(this);
}

void IRVisitor::visit(const While *op) {
  op->condition.accept(this);
  op->body.accept(this);
}

void IRVisitor::visit(const Kernel *op) {
  op->body.accept(this);
}

void IRVisitor::visit(const Block *op) {
  for (Stmt stmt : op->stmts) {
    stmt.accept(this);
  }
}

void IRVisitor::visit(const Print *op) {
  op->expr.accept(this);
}

void IRVisitor::visit(const Comment *op) {
  if (op->commentedStmt.defined()) {
    op->commentedStmt.accept(this);
  }
}

void IRVisitor::visit(const Pass *op) {
}

void IRVisitor::visit(const UnnamedTupleRead *op) {
  op->tuple.accept(this);
  op->index.accept(this);
}

void IRVisitor::visit(const NamedTupleRead *op) {
  op->tuple.accept(this);
}

void IRVisitor::visit(const SetRead *op) {
  op->set.accept(this);
  for (auto &index : op->indices) {
    index.accept(this);
  }
}

void IRVisitor::visit(const TensorRead *op) {
  op->tensor.accept(this);
  for (auto &index : op->indices) {
    index.accept(this);
  }
}

void IRVisitor::visit(const TensorWrite *op) {
  op->tensor.accept(this);
  for (auto &index : op->indices) {
    index.accept(this);
  }
  op->value.accept(this);
}

void IRVisitor::visit(const IndexedTensor *op) {
  op->tensor.accept(this);
}

void IRVisitor::visit(const IndexExpr *op) {
  op->value.accept(this);
}

void IRVisitor::visit(const Map *op) {
  op->target.accept(this);
  for (auto &n : op->neighbors) {
    n.accept(this);
  }
  for (auto &p : op->partial_actuals) {
    p.accept(this);
  }
}

void IRVisitor::visit(const Func *op) {
  if (op->getBody().defined()) {
    op->getBody().accept(this);
  }
}

#ifdef GPU
void IRVisitor::visit(const GPUKernel *op) {
  op->body.accept(this);
}
#endif


// class IRVisitorCallGraph
void IRVisitorCallGraph::visit(const CallStmt *op) {
  if (visited.find(op->callee) == visited.end()) {
    op->callee.accept(this);
    visited.insert(op->callee);
  }

  for (auto &actual : op->actuals) {
    actual.accept(this);
  }
}

void IRVisitorCallGraph::visit(const Map *op) {
  if (visited.find(op->function) == visited.end()) {
    op->function.accept(this);
    visited.insert(op->function);
  }

  IRVisitor::visit(op);
}

// IRQuery
bool IRQuery::query(const Expr &expr) {
  result = init;
  expr.accept(this);
  return result;
}

bool IRQuery::query(const Stmt &stmt) {
  result = init;
  stmt.accept(this);
  return result;
}

}} // namespace simit::ir
