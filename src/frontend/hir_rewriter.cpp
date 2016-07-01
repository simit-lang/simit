#include "hir_rewriter.h"

namespace simit {
namespace hir {

void HIRRewriter::visit(Program::Ptr program) {
  for (auto &elem : program->elems) {
    elem = rewrite<HIRNode>(elem);
  }
  node = program;
}

void HIRRewriter::visit(StmtBlock::Ptr stmtBlock) {
  for (auto &stmt : stmtBlock->stmts) {
    stmt = rewrite<Stmt>(stmt);
  }
  node = stmtBlock;
}

void HIRRewriter::visit(SetType::Ptr type) {
  type->element = rewrite<ElementType>(type->element);
  if (type->type == SetType::Type::UNSTRUCTURED) {
    for (auto &endpoint : type->endpoints) {
      endpoint = rewrite<Endpoint>(endpoint);
    }
  }
  else if (type->type == SetType::Type::LATTICE_LINK) {
    type->latticePointSet = rewrite<Endpoint>(type->latticePointSet);
  }
  else {
    unreachable;
  }
  node = type;
}

void HIRRewriter::visit(TupleType::Ptr type) {
  type->element = rewrite<ElementType>(type->element);
  type->length = rewrite<TupleLength>(type->length);
  node = type;
}

void HIRRewriter::visit(NDTensorType::Ptr type) {
  for (auto &is : type->indexSets) {
    is = rewrite<IndexSet>(is);
  }
  type->blockType = rewrite<TensorType>(type->blockType);
  node = type;
}

void HIRRewriter::visit(IdentDecl::Ptr decl) {
  decl->name = rewrite<Identifier>(decl->name);
  decl->type = rewrite<Type>(decl->type);
  node = decl;
}

void HIRRewriter::visit(Field::Ptr field) {
  field->field = rewrite<IdentDecl>(field->field);
  node = field;
}

void HIRRewriter::visit(ElementTypeDecl::Ptr decl) {
  decl->name = rewrite<Identifier>(decl->name);
  for (auto &field : decl->fields) {
    field = rewrite<Field>(field);
  }
  node = decl;
}

void HIRRewriter::visit(Argument::Ptr arg) {
  arg->arg = rewrite<IdentDecl>(arg->arg);
  node = arg;
}

void HIRRewriter::visit(ExternDecl::Ptr decl) {
  decl->var = rewrite<IdentDecl>(decl->var);
  node = decl;
}

void HIRRewriter::visit(FuncDecl::Ptr decl) {
  decl->name = rewrite<Identifier>(decl->name);
  for (auto &arg : decl->args) {
    arg = rewrite<Argument>(arg);
  }
  for (auto &result : decl->results) {
    result = rewrite<IdentDecl>(result);
  }
  if (decl->body != nullptr) {
    decl->body = rewrite<StmtBlock>(decl->body);
  }
  node = decl;
}

void HIRRewriter::visit(VarDecl::Ptr decl) {
  decl->name = rewrite<Identifier>(decl->name);
  if (decl->type) {
    decl->type = rewrite<Type>(decl->type);
  }
  if (decl->initVal) {
    decl->initVal = rewrite<Expr>(decl->initVal);
  }
  node = decl;
}

void HIRRewriter::visit(WhileStmt::Ptr stmt) {
  stmt->cond = rewrite<Expr>(stmt->cond);
  stmt->body = rewrite<StmtBlock>(stmt->body);
  node = stmt;
}

void HIRRewriter::visit(IfStmt::Ptr stmt) {
  stmt->cond = rewrite<Expr>(stmt->cond);
  stmt->ifBody = rewrite<Stmt>(stmt->ifBody);
  if (stmt->elseBody) {
    stmt->elseBody = rewrite<Stmt>(stmt->elseBody);
  }
  node = stmt;
}

void HIRRewriter::visit(IndexSetDomain::Ptr domain) {
  domain->set = rewrite<SetIndexSet>(domain->set);
  node = domain;
}

void HIRRewriter::visit(RangeDomain::Ptr domain) {
  domain->lower = rewrite<Expr>(domain->lower);
  domain->upper = rewrite<Expr>(domain->upper);
  node = domain;
}

void HIRRewriter::visit(ForStmt::Ptr stmt) {
  stmt->loopVar = rewrite<Identifier>(stmt->loopVar);
  stmt->domain = rewrite<ForDomain>(stmt->domain);
  stmt->body = rewrite<StmtBlock>(stmt->body);
  node = stmt;
}

void HIRRewriter::visit(PrintStmt::Ptr stmt) {
  for (auto &arg : stmt->args) {
    arg = rewrite<Expr>(arg);
  }
  node = stmt;
}

void HIRRewriter::visit(ExprStmt::Ptr stmt) {
  stmt->expr = rewrite<Expr>(stmt->expr);
  node = stmt;
}

void HIRRewriter::visit(AssignStmt::Ptr stmt) {
  for (auto &lhs : stmt->lhs) {
    lhs = rewrite<Expr>(lhs);
  }
  stmt->expr = rewrite<Expr>(stmt->expr);
  node = stmt;
}

void HIRRewriter::visit(ExprParam::Ptr param) {
  param->expr = rewrite<Expr>(param->expr);
  node = param;
}

void HIRRewriter::visit(MapExpr::Ptr expr) {
  expr->func = rewrite<Identifier>(expr->func);
  for (auto &arg : expr->partialActuals) {
    arg = rewrite<Expr>(arg);
  }
  expr->target = rewrite<Identifier>(expr->target);
  if (expr->through) {
    expr->through = rewrite<Identifier>(expr->through);
  }
  node = expr;
}

void HIRRewriter::visit(OrExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRRewriter::visit(AndExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRRewriter::visit(XorExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRRewriter::visit(EqExpr::Ptr expr) {
  visitNaryExpr(expr);
}

void HIRRewriter::visit(NotExpr::Ptr expr) {
  visitUnaryExpr(expr);
}

void HIRRewriter::visit(AddExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRRewriter::visit(SubExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRRewriter::visit(MulExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRRewriter::visit(DivExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRRewriter::visit(LeftDivExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRRewriter::visit(ElwiseMulExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRRewriter::visit(ElwiseDivExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRRewriter::visit(NegExpr::Ptr expr) {
  visitUnaryExpr(expr);
}

void HIRRewriter::visit(ExpExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRRewriter::visit(TransposeExpr::Ptr expr) {
  visitUnaryExpr(expr);
}

void HIRRewriter::visit(CallExpr::Ptr expr) {
  expr->func = rewrite<Identifier>(expr->func);
  for (auto &arg : expr->args) {
    if (arg) {
      arg = rewrite<Expr>(arg);
    }
  }
  node = expr;
}

void HIRRewriter::visit(TensorReadExpr::Ptr expr) {
  expr->tensor = rewrite<Expr>(expr->tensor);
  for (auto &index : expr->indices) {
    index = rewrite<ReadParam>(index);
  }
  node = expr;
}

void HIRRewriter::visit(SetReadExpr::Ptr expr) {
  expr->set = rewrite<Expr>(expr->set);
  for (auto &index : expr->indices) {
    index = rewrite<ReadParam>(index);
  }
  node = expr;
}

void HIRRewriter::visit(TupleReadExpr::Ptr expr) {
  expr->tuple = rewrite<Expr>(expr->tuple);
  expr->index = rewrite<Expr>(expr->index);
  node = expr;
}

void HIRRewriter::visit(FieldReadExpr::Ptr expr) {
  expr->setOrElem = rewrite<Expr>(expr->setOrElem);
  expr->field = rewrite<Identifier>(expr->field);
  node = expr;
}

void HIRRewriter::visit(ParenExpr::Ptr expr) {
  expr->expr = rewrite<Expr>(expr->expr);
  node = expr;
}

void HIRRewriter::visit(NDTensorLiteral::Ptr lit) {
  for (auto &elem : lit->elems) {
    elem = rewrite<DenseTensorLiteral>(elem);
  }
  node = lit;
}

void HIRRewriter::visit(ApplyStmt::Ptr stmt) {
  stmt->map = rewrite<UnreducedMapExpr>(stmt->map);
  node = stmt;
}

void HIRRewriter::visit(Test::Ptr test) {
  test->func = rewrite<Identifier>(test->func);
  for (auto &arg : test->args) {
    arg = rewrite<Expr>(arg);
  }
  test->expected = rewrite<Expr>(test->expected);
  node = test;
}

void HIRRewriter::visitUnaryExpr(UnaryExpr::Ptr expr) {
  expr->operand = rewrite<Expr>(expr->operand);
  node = expr;
}

void HIRRewriter::visitBinaryExpr(BinaryExpr::Ptr expr) {
  expr->lhs = rewrite<Expr>(expr->lhs);
  expr->rhs = rewrite<Expr>(expr->rhs);
  node = expr;
}

void HIRRewriter::visitNaryExpr(NaryExpr::Ptr expr) {
  for (auto &operand : expr->operands) {
    operand = rewrite<Expr>(operand);
  }
  node = expr;
}

}
}

