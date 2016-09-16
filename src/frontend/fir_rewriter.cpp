#include "fir_rewriter.h"

namespace simit {
namespace fir {

void FIRRewriter::visit(Program::Ptr program) {
  for (auto &elem : program->elems) {
    elem = rewrite<FIRNode>(elem);
  }
  node = program;
}

void FIRRewriter::visit(StmtBlock::Ptr stmtBlock) {
  for (auto &stmt : stmtBlock->stmts) {
    stmt = rewrite<Stmt>(stmt);
  }
  node = stmtBlock;
}

void FIRRewriter::visit(Endpoint::Ptr end) {
  end->set = rewrite<SetIndexSet>(end->set);
  if (end->element) {
    end->element = rewrite<ElementType>(end->element);
  }
  node = end;
}

void FIRRewriter::visit(HomogeneousEdgeSetType::Ptr type) {
  type->element = rewrite<ElementType>(type->element);
  type->endpoint = rewrite<Endpoint>(type->endpoint);
  type->arity = rewrite<TupleLength>(type->arity);
  node = type;
}

void FIRRewriter::visit(HeterogeneousEdgeSetType::Ptr type) {
  type->element = rewrite<ElementType>(type->element);
  for (auto &endpoint : type->endpoints) {
    endpoint = rewrite<Endpoint>(endpoint);
  }
  node = type;
}

void FIRRewriter::visit(GridSetType::Ptr type) {
  type->element = rewrite<ElementType>(type->element);
  type->underlyingPointSet = rewrite<Endpoint>(type->underlyingPointSet);
  node = type;
}

void FIRRewriter::visit(TupleElement::Ptr type) {
  if (type->name) {
    type->name = rewrite<Identifier>(type->name);
  }
  type->element = rewrite<ElementType>(type->element);
  node = type;
}

void FIRRewriter::visit(NamedTupleType::Ptr type) {
  for (auto &elem : type->elems) {
    elem = rewrite<TupleElement>(elem);
  }
  node = type;
}

void FIRRewriter::visit(UnnamedTupleType::Ptr type) {
  type->element = rewrite<ElementType>(type->element);
  type->length = rewrite<TupleLength>(type->length);
  node = type;
}

void FIRRewriter::visit(NDTensorType::Ptr type) {
  for (auto &is : type->indexSets) {
    is = rewrite<IndexSet>(is);
  }
  type->blockType = rewrite<TensorType>(type->blockType);
  node = type;
}

void FIRRewriter::visit(IdentDecl::Ptr decl) {
  decl->name = rewrite<Identifier>(decl->name);
  decl->type = rewrite<Type>(decl->type);
  node = decl;
}

void FIRRewriter::visit(ElementTypeDecl::Ptr decl) {
  decl->name = rewrite<Identifier>(decl->name);
  for (auto &field : decl->fields) {
    field = rewrite<FieldDecl>(field);
  }
  node = decl;
}

void FIRRewriter::visit(FuncDecl::Ptr decl) {
  decl->name = rewrite<Identifier>(decl->name);
  for (auto &genericParam : decl->genericParams) {
    genericParam = rewrite<GenericParam>(genericParam);
  }
  for (auto &arg : decl->args) {
    arg = rewrite<Argument>(arg);
  }
  for (auto &result : decl->results) {
    result = rewrite<IdentDecl>(result);
  }
  if (decl->body) {
    decl->body = rewrite<StmtBlock>(decl->body);
  }
  node = decl;
}

void FIRRewriter::visit(VarDecl::Ptr decl) {
  decl->name = rewrite<Identifier>(decl->name);
  if (decl->type) {
    decl->type = rewrite<Type>(decl->type);
  }
  if (decl->initVal) {
    decl->initVal = rewrite<Expr>(decl->initVal);
  }
  node = decl;
}

void FIRRewriter::visit(WhileStmt::Ptr stmt) {
  stmt->cond = rewrite<Expr>(stmt->cond);
  stmt->body = rewrite<StmtBlock>(stmt->body);
  node = stmt;
}

void FIRRewriter::visit(IfStmt::Ptr stmt) {
  stmt->cond = rewrite<Expr>(stmt->cond);
  stmt->ifBody = rewrite<Stmt>(stmt->ifBody);
  if (stmt->elseBody) {
    stmt->elseBody = rewrite<Stmt>(stmt->elseBody);
  }
  node = stmt;
}

void FIRRewriter::visit(IndexSetDomain::Ptr domain) {
  domain->set = rewrite<SetIndexSet>(domain->set);
  node = domain;
}

void FIRRewriter::visit(RangeDomain::Ptr domain) {
  domain->lower = rewrite<Expr>(domain->lower);
  domain->upper = rewrite<Expr>(domain->upper);
  node = domain;
}

void FIRRewriter::visit(ForStmt::Ptr stmt) {
  stmt->loopVar = rewrite<Identifier>(stmt->loopVar);
  stmt->domain = rewrite<ForDomain>(stmt->domain);
  stmt->body = rewrite<StmtBlock>(stmt->body);
  node = stmt;
}

void FIRRewriter::visit(PrintStmt::Ptr stmt) {
  for (auto &arg : stmt->args) {
    arg = rewrite<Expr>(arg);
  }
  node = stmt;
}

void FIRRewriter::visit(ExprStmt::Ptr stmt) {
  stmt->expr = rewrite<Expr>(stmt->expr);
  node = stmt;
}

void FIRRewriter::visit(AssignStmt::Ptr stmt) {
  stmt->expr = rewrite<Expr>(stmt->expr);
  for (auto &lhs : stmt->lhs) {
    lhs = rewrite<Expr>(lhs);
  }
  node = stmt;
}

void FIRRewriter::visit(ExprParam::Ptr param) {
  param->expr = rewrite<Expr>(param->expr);
  node = param;
}

void FIRRewriter::visit(MapExpr::Ptr expr) {
  expr->func = rewrite<Identifier>(expr->func);
  for (auto &genericArg : expr->genericArgs) {
    genericArg = rewrite<IndexSet>(genericArg);
  }
  for (auto &arg : expr->partialActuals) {
    arg = rewrite<Expr>(arg);
  }
  expr->target = rewrite<SetIndexSet>(expr->target);
  if (expr->through) {
    expr->through = rewrite<SetIndexSet>(expr->through);
  }
  node = expr;
}

void FIRRewriter::visit(OrExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRRewriter::visit(AndExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRRewriter::visit(XorExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRRewriter::visit(EqExpr::Ptr expr) {
  visitNaryExpr(expr);
}

void FIRRewriter::visit(NotExpr::Ptr expr) {
  visitUnaryExpr(expr);
}

void FIRRewriter::visit(AddExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRRewriter::visit(SubExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRRewriter::visit(MulExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRRewriter::visit(DivExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRRewriter::visit(LeftDivExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRRewriter::visit(ElwiseMulExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRRewriter::visit(ElwiseDivExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRRewriter::visit(NegExpr::Ptr expr) {
  visitUnaryExpr(expr);
}

void FIRRewriter::visit(ExpExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRRewriter::visit(TransposeExpr::Ptr expr) {
  visitUnaryExpr(expr);
}

void FIRRewriter::visit(CallExpr::Ptr expr) {
  expr->func = rewrite<Identifier>(expr->func);
  for (auto &genericArg : expr->genericArgs) {
    genericArg = rewrite<IndexSet>(genericArg);
  }
  for (auto &arg : expr->args) {
    arg = rewrite<Expr>(arg);
  }
  node = expr;
}

void FIRRewriter::visit(TensorReadExpr::Ptr expr) {
  expr->tensor = rewrite<Expr>(expr->tensor);
  for (auto &index : expr->indices) {
    index = rewrite<ReadParam>(index);
  }
  node = expr;
}

void FIRRewriter::visit(SetReadExpr::Ptr expr) {
  expr->set = rewrite<Expr>(expr->set);
  for (auto &index : expr->indices) {
    index = rewrite<Expr>(index);
  }
  node = expr;
}

void FIRRewriter::visit(NamedTupleReadExpr::Ptr expr) {
  expr->tuple = rewrite<Expr>(expr->tuple);
  expr->elem = rewrite<Identifier>(expr->elem);
  node = expr;
}

void FIRRewriter::visit(UnnamedTupleReadExpr::Ptr expr) {
  expr->tuple = rewrite<Expr>(expr->tuple);
  expr->index = rewrite<Expr>(expr->index);
  node = expr;
}

void FIRRewriter::visit(FieldReadExpr::Ptr expr) {
  expr->setOrElem = rewrite<Expr>(expr->setOrElem);
  expr->field = rewrite<Identifier>(expr->field);
  node = expr;
}

void FIRRewriter::visit(ParenExpr::Ptr expr) {
  expr->expr = rewrite<Expr>(expr->expr);
  node = expr;
}

void FIRRewriter::visit(NDTensorLiteral::Ptr lit) {
  for (auto &elem : lit->elems) {
    elem = rewrite<DenseTensorLiteral>(elem);
  }
  node = lit;
}

void FIRRewriter::visit(ApplyStmt::Ptr stmt) {
  stmt->map = rewrite<UnreducedMapExpr>(stmt->map);
  node = stmt;
}

void FIRRewriter::visit(Test::Ptr test) {
  test->func = rewrite<Identifier>(test->func);
  for (auto &arg : test->args) {
    arg = rewrite<Expr>(arg);
  }
  test->expected = rewrite<Expr>(test->expected);
  node = test;
}

void FIRRewriter::visitUnaryExpr(UnaryExpr::Ptr expr) {
  expr->operand = rewrite<Expr>(expr->operand);
  node = expr;
}

void FIRRewriter::visitBinaryExpr(BinaryExpr::Ptr expr) {
  expr->lhs = rewrite<Expr>(expr->lhs);
  expr->rhs = rewrite<Expr>(expr->rhs);
  node = expr;
}

void FIRRewriter::visitNaryExpr(NaryExpr::Ptr expr) {
  for (auto &operand : expr->operands) {
    operand = rewrite<Expr>(operand);
  }
  node = expr;
}

}
}

