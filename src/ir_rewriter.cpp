#include "ir_rewriter.h"

#include <vector>

namespace simit {
namespace ir {

Expr IRRewriter::rewrite(Expr e) {
  if (e.defined()) {
    e.accept(this);
    e = expr;
  }
  else {
    e = Expr();
  }
  expr = Expr();
  stmt = Stmt();
  func = Func();
  return e;
}

Stmt IRRewriter::rewrite(Stmt s) {
  if (s.defined()) {
    s.accept(this);
    Stmt spilledStmts = getSpilledStmts();
    if (spilledStmts.defined()) {
      stmt = Block::make(spilledStmts, stmt);
    }
    s = stmt;
  }
  else {
    s = Stmt();
  }
  expr = Expr();
  stmt = Stmt();
  func = Func();
  return s;
}

Func IRRewriter::rewrite(Func f) {
  if (f.defined()) {
    f.accept(this);
    f = func;
  }
  else {
    f = Func();
  }
  expr = Expr();
  stmt = Stmt();
  func = Func();
  return f;
}

void IRRewriter::visit(const Literal *op) {
  expr = op;
}

void IRRewriter::visit(const VarExpr *op) {
  expr = op;
}

void IRRewriter::visit(const Load *op) {
  Expr buffer = rewrite(op->buffer);
  Expr index = rewrite(op->index);
  if (buffer == op->buffer && index == op->index) {
    expr = op;
  }
  else {
    expr = Load::make(buffer, index);
  }
}

void IRRewriter::visit(const FieldRead *op) {
  Expr elementOrSet = rewrite(op->elementOrSet);
  if (elementOrSet == op->elementOrSet) {
    expr = op;
  }
  else {
    expr = FieldRead::make(elementOrSet, op->fieldName);
  }
}

void IRRewriter::visit(const Length *op) {
  if (op->indexSet.getKind() == IndexSet::Set) {
    Expr set = rewrite(op->indexSet.getSet());
    if (set == op->indexSet.getSet()) {
      expr = op;
    }
    else {
      expr = Length::make(IndexSet(set));
    }
  } else {
    expr = op;
  }
}

void IRRewriter::visit(const IndexRead *op) {
  Expr edgeSet = rewrite(op->edgeSet);
  if (edgeSet == op->edgeSet) {
    expr = op;
  }
  else if (op->kind != IndexRead::GridDim) {
    expr = IndexRead::make(edgeSet, op->kind);
  }
  else {
    expr = IndexRead::make(edgeSet, op->kind, op->index);
  }
}

void IRRewriter::visit(const Neg *op) {
  Expr a = rewrite(op->a);
  if (a == op->a) {
    expr = op;
  }
  else {
    expr = Neg::make(a);
  }
}

template <class T>
Expr visitBinaryOp(const T *op, IRRewriter *rw) {
  Expr a = rw->rewrite(op->a);
  Expr b = rw->rewrite(op->b);
  if (a == op->a && b == op->b) {
    return op;
  }
  else {
    return T::make(a, b);
  }
}

void IRRewriter::visit(const Add *op) {
  expr = visitBinaryOp(op, this);
}

void IRRewriter::visit(const Sub *op) {
  expr = visitBinaryOp(op, this);
}

void IRRewriter::visit(const Mul *op) {
  expr = visitBinaryOp(op, this);
}

void IRRewriter::visit(const Div *op) {
  expr = visitBinaryOp(op, this);
}

void IRRewriter::visit(const Rem *op) {
  expr = visitBinaryOp(op, this);
}

void IRRewriter::visit(const Eq *op) {
  expr = visitBinaryOp(op, this);
}

void IRRewriter::visit(const Ne *op) {
  expr = visitBinaryOp(op, this);
}

void IRRewriter::visit(const Gt *op) {
  expr = visitBinaryOp(op, this);
}

void IRRewriter::visit(const Lt *op) {
  expr = visitBinaryOp(op, this);
}

void IRRewriter::visit(const Ge *op) {
  expr = visitBinaryOp(op, this);
}

void IRRewriter::visit(const Le *op) {
  expr = visitBinaryOp(op, this);
}

void IRRewriter::visit(const And *op) {
  expr = visitBinaryOp(op, this);
}

void IRRewriter::visit(const Or *op) {
  expr = visitBinaryOp(op, this);
}

void IRRewriter::visit(const Not *op) {
  Expr a = rewrite(op->a);
  if (a == op->a) {
    expr = op;
  }
  else {
    expr = Not::make(a);
  }
}

void IRRewriter::visit(const Xor *op) {
  expr = visitBinaryOp(op, this);
}

void IRRewriter::visit(const VarDecl *op) {
  stmt = op;
}

void IRRewriter::visit(const AssignStmt *op) {
  Expr value = rewrite(op->value);
  if (value == op->value) {
    stmt = op;
  }
  else {
    stmt = AssignStmt::make(op->var, value, op->cop);
  }
}

void IRRewriter::visit(const CallStmt *op) {
  std::vector<Expr> actuals(op->actuals.size());
  bool actualsSame = true;
  for (size_t i=0; i < op->actuals.size(); ++i) {
    actuals[i] = rewrite(op->actuals[i]);
    if (actuals[i] != op->actuals[i]) {
      actualsSame = false;
    }
  }
  if (actualsSame) {
    stmt = op;
  }
  else {
    stmt = CallStmt::make(op->results, op->callee, actuals);
  }
}

void IRRewriter::visit(const Store *op) {
  Expr buffer = rewrite(op->buffer);
  Expr index = rewrite(op->index);
  Expr value = rewrite(op->value);
  if (buffer == op->buffer && op->index == index && value == op->value) {
    stmt = op;
  }
  else {
    stmt = Store::make(buffer, index, value, op->cop);
  }
}

void IRRewriter::visit(const FieldWrite *op) {
  Expr elementOrSet = rewrite(op->elementOrSet);
  Expr value = rewrite(op->value);
  if (elementOrSet == op->elementOrSet && value == op->value) {
    stmt = op;
  }
  else {
    stmt = FieldWrite::make(elementOrSet, op->fieldName, value, op->cop);
  }
}

void IRRewriter::visit(const Scope *op) {
  Stmt scopedStmt = rewrite(op->scopedStmt);
  if (scopedStmt == op->scopedStmt) {
    stmt = op;
  }
  else {
    stmt = Scope::make(scopedStmt);
  }
}

void IRRewriter::visit(const IfThenElse *op) {
  Expr condition = rewrite(op->condition);
  Stmt spilledCond = getSpilledStmts();
  Stmt thenBody = rewrite(op->thenBody);
  Stmt elseBody = rewrite(op->elseBody);
  if (condition == op->condition && thenBody == op->thenBody &&
      elseBody == op->elseBody) {
    stmt = op;
  }
  else {
    if (elseBody.defined()) {
      stmt = IfThenElse::make(condition, thenBody, elseBody);
    } else {
      stmt = IfThenElse::make(condition, thenBody);
    }
    if (spilledCond.defined()) {
      stmt = Block::make(spilledCond, stmt);
    }
  }
}

void IRRewriter::visit(const ForRange *op) {
  Expr start = rewrite(op->start);
  Expr end = rewrite(op->end);
  Stmt spilledBounds = getSpilledStmts();
  Stmt body = rewrite(op->body);

  if (body == op->body && start == op->start && end == op->end) {
    stmt = op;
  }
  else {
    stmt = ForRange::make(op->var, start, end, body);
    if (spilledBounds.defined()) {
      stmt = Block::make(spilledBounds, stmt);
    }
  }
}

void IRRewriter::visit(const For *op) {
  Stmt body = rewrite(op->body);
  if (body == op->body) {
    stmt = op;
  }
  else {
    stmt = For::make(op->var, op->domain, body);
  }
}

void IRRewriter::visit(const While *op) {
  Expr condition = rewrite(op->condition);
  Stmt spilledCond = getSpilledStmts();
  Stmt body = rewrite(op->body);

  if (condition == op->condition && body == op->body) {
    stmt = op;
  }
  else {
    if (spilledCond.defined()) {
      body = Block::make(body, spilledCond);
    }
    stmt = While::make(condition, body);
    if (spilledCond.defined()) {
      stmt = Block::make(spilledCond, stmt);
    }
  }
}

void IRRewriter::visit(const Kernel *op) {
  Stmt body = rewrite(op->body);
  if (body == op->body) {
    stmt = op;
  }
  else {
    stmt = Kernel::make(op->var, op->domain, body);
  }
}

void IRRewriter::visit(const Block *op) {
  vector<Stmt> newStmts;

  for (Stmt stmt : op->stmts) {
    newStmts.push_back(rewrite(stmt));
  }

  if (newStmts == op->stmts) {
    stmt = op;
  } else {
    newStmts.resize(std::remove_if(newStmts.begin(), newStmts.end(),
                        [](Stmt s){return !s.defined();}) - newStmts.begin());
    if (newStmts.size()) {
      stmt = Block::make(newStmts);
    } else {
      stmt = Stmt();
    }
  }
}

void IRRewriter::visit(const Print *op) {
  Expr expr = rewrite(op->expr);
  if (expr == op->expr) {
    stmt = op;
  } else {
    stmt = Print::make(expr, op->format);
  }
}

void IRRewriter::visit(const Comment *op) {
  Stmt commentedStmt = rewrite(op->commentedStmt);
  if (commentedStmt == op->commentedStmt) {
    stmt = op;
  }
  else if (op->commentedStmt.defined() && !commentedStmt.defined()) {
    // If the comment was commenting a statement that was removed, then we
    // remove the comment too
    stmt = Stmt();
  }
  else {
    stmt = Comment::make(op->comment, commentedStmt,
                         op->footerSpace, op->headerSpace);
  }
}

void IRRewriter::visit(const Pass *op) {
  stmt = op;
}

void IRRewriter::visit(const TensorWrite *op) {
  Expr tensor = rewrite(op->tensor);
  std::vector<Expr> indices(op->indices.size());
  bool indicesSame = true;
  for (size_t i=0; i < op->indices.size(); ++i) {
    indices[i] = rewrite(op->indices[i]);
    if (indices[i] != op->indices[i]) {
      indicesSame = false;
    }
  }
  Expr value = rewrite(op->value);
  if (tensor == op->tensor && indicesSame && value == op->value) {
    stmt = op;
  }
  else {
    stmt = TensorWrite::make(tensor, indices, value, op->cop);
  }
}

void IRRewriter::visit(const UnnamedTupleRead *op) {
  Expr tuple = rewrite(op->tuple);
  Expr index = rewrite(op->index);
  if (tuple == op->tuple && index == op->index) {
    expr = op;
  }
  else {
    expr = UnnamedTupleRead::make(tuple, index);
  }
}

void IRRewriter::visit(const NamedTupleRead *op) {
  Expr tuple = rewrite(op->tuple);
  if (tuple == op->tuple) {
    expr = op;
  } else {
    expr = NamedTupleRead::make(tuple, op->elementName);
  }
}

void IRRewriter::visit(const SetRead *op) {
  Expr set = rewrite(op->set);
  std::vector<Expr> indices(op->indices.size());
  bool indicesSame = true;
  for (size_t i=0; i < op->indices.size(); ++i) {
    indices[i] = rewrite(op->indices[i]);
    if (indices[i] != op->indices[i]) {
      indicesSame = false;
    }
  }
  if (set == op->set && indicesSame) {
    expr = op;
  }
  else {
    expr = SetRead::make(set, indices);
  }
}

void IRRewriter::visit(const TensorRead *op) {
  Expr tensor = rewrite(op->tensor);
  std::vector<Expr> indices(op->indices.size());
  bool indicesSame = true;
  for (size_t i=0; i < op->indices.size(); ++i) {
    indices[i] = rewrite(op->indices[i]);
    if (indices[i] != op->indices[i]) {
      indicesSame = false;
    }
  }
  if (tensor == op->tensor && indicesSame) {
    expr = op;
  }
  else {
    expr = TensorRead::make(tensor, indices);
  }
}

void IRRewriter::visit(const IndexedTensor *op) {
  Expr tensor = rewrite(op->tensor);
  if (tensor == op->tensor) {
    expr = op;
  }
  else {
    expr = IndexedTensor::make(tensor, op->indexVars);
  }
}

void IRRewriter::visit(const IndexExpr *op) {
  Expr value = rewrite(op->value);
  if (value == op->value) {
    expr = op;
  }
  else {
    const bool isColumnVector = op->type.toTensor()->isColumnVector;
    expr = IndexExpr::make(op->resultVars, value, isColumnVector);
  }
}

void IRRewriter::visit(const Map *op) {
  Expr target = rewrite(op->target);

  std::vector<Expr> neighbors(op->neighbors.size());
  bool neighborsSame = true;
  for (size_t i=0; i < op->neighbors.size(); ++i) {
    neighbors[i] = rewrite(op->neighbors[i]);
    if (neighbors[i] != op->neighbors[i]) {
      neighborsSame = false;
    }
  }

  Expr through;
  if (op->through.defined()) {
    through = rewrite(op->through);
  }

  std::vector<Expr> partial_actuals(op->partial_actuals.size());
  bool actualsSame = true;
  for (size_t i=0; i < op->partial_actuals.size(); ++i) {
    partial_actuals[i] = rewrite(op->partial_actuals[i]);
    if (partial_actuals[i] != op->partial_actuals[i]) {
      actualsSame = false;
    }
  }

  if (target == op->target && through == op->through &&
      neighborsSame && actualsSame) {
    stmt = op;
  }
  else {
    stmt = Map::make(op->vars, op->function, partial_actuals, target,
                     neighbors, through, op->reduction);
  }
}

void IRRewriter::visit(const Func *f) {
  Stmt body = rewrite(f->getBody());

  if (body == f->getBody()) {
    func = *f;
  }
  else {
    if (!body.defined()) {
      body = Pass::make();
    }
    func = Func(*f, body);
  }
}

#ifdef GPU
void IRRewriter::visit(const GPUKernel *op) {
  Stmt body = rewrite(op->body);
  if (body == op->body) {
    stmt = op;
  }
  else {
    stmt = GPUKernel::make(body, op->sharding, op->reads, op->writes);
  }
}
#endif

void IRRewriter::spill(Stmt stmt) {
  spilledStmts.push_back(stmt);
}

Stmt IRRewriter::getSpilledStmts() {
  Stmt spillBlock;
  if (spilledStmts.size() > 0) {
    spillBlock = Block::make(spilledStmts);
    spilledStmts.clear();
  }
  return spillBlock;
}


// class IRRewriterCallGraph
void IRRewriterCallGraph::visit(const CallStmt *op) {
  IRRewriter::visit(op);
  op = to<CallStmt>(stmt);

  Func callee = op->callee;
  if (visited.find(op->callee) == visited.end()) {
    callee = rewrite(op->callee);
    visited[op->callee] = callee;
  }
  else {
    callee = visited[op->callee];
  }

  stmt = (callee != op->callee)
      ? CallStmt::make(op->results, callee, op->actuals) : op;
}

void IRRewriterCallGraph::visit(const Map *op) {
  IRRewriter::visit(op);
  op = to<Map>(stmt);

  Func function = op->function;
  if (visited.find(op->function) == visited.end()) {
    function = rewrite(op->function);
    visited[op->function] = function;
  }
  else {
    function = visited[op->function];
  }

  stmt = (function != op->function)
      ? Map::make(op->vars, function, op->partial_actuals,
                  op->target, op->neighbors, op->through, op->reduction)
      : op;
}
}} // namespace simit::ir
