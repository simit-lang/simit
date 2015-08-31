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

void IRVisitor::visit(const Call *op) {
  for (auto &actual : op->actuals) {
    actual.accept(this);
  }
}

void IRVisitor::visit(const Neg *op) {
  op->a.accept(this);
}

void IRVisitor::visit(const Add *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(const Sub *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(const Mul *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(const Div *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(const Eq *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(const Ne *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(const Gt *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(const Lt *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(const Ge *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(const Le *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(const And *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(const Or *op) {
  op->a.accept(this);
  op->b.accept(this);
}

void IRVisitor::visit(const Not *op) {
  op->a.accept(this);
}

void IRVisitor::visit(const Xor *op) {
  op->a.accept(this);
  op->b.accept(this);
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

void IRVisitor::visit(const Block *op) {
  op->first.accept(this);
  if (op->rest.defined()) {
    op->rest.accept(this);
  }
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

void IRVisitor::visit(const TupleRead *op) {
  op->tuple.accept(this);
  op->index.accept(this);
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
  if (op->neighbors.defined()) {
    op->neighbors.accept(this);
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


// class IRVisitorCallGraph
void IRVisitorCallGraph::visit(const Call *op) {
  if (visited.find(op->func) == visited.end()) {
    op->func.accept(this);
    visited.insert(op->func);
  }

  for (auto &actual : op->actuals) {
    actual.accept(this);
  }
}

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
