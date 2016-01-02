#include "hir_rewriter.h"

namespace simit {
namespace hir {

void HIRRewriter::visit(Program::Ptr program) {
  for (unsigned i = 0; i < program->elems.size(); ++i) {
    const auto elem = program->elems[i];
    program->elems[i] = rewrite<HIRNode>(elem);
  }
  node = program;
}

void HIRRewriter::visit(StmtBlock::Ptr stmtBlock) {
  for (unsigned i = 0; i < stmtBlock->stmts.size(); ++i) {
    stmtBlock->stmts[i] = rewrite<Stmt>(stmtBlock->stmts[i]);
  }
  node = stmtBlock;
}

void HIRRewriter::visit(SetType::Ptr type) {
  type->element = rewrite<ElementType>(type->element);
  for (unsigned i = 0; i < type->endpoints.size(); ++i) {
    type->endpoints[i] = rewrite<Endpoint>(type->endpoints[i]);
  }
  node = type;
}

void HIRRewriter::visit(TupleType::Ptr type) {
  type->element = rewrite<ElementType>(type->element);
  type->length = rewrite<TupleLength>(type->length);
  node = type;
}

void HIRRewriter::visit(NDTensorType::Ptr type) {
  for (unsigned i = 0; i < type->indexSets.size(); ++i) {
    type->indexSets[i] = rewrite<IndexSet>(type->indexSets[i]);
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
  for (unsigned i = 0; i < decl->fields.size(); ++i) {
    decl->fields[i] = rewrite<Field>(decl->fields[i]);
  }
  node = decl;
}

void HIRRewriter::visit(Argument::Ptr arg) {
  visit(to<IdentDecl>(arg));
}

void HIRRewriter::visit(ExternDecl::Ptr decl) {
  decl->var = rewrite<Argument>(decl->var);
  node = decl;
}

void HIRRewriter::visit(FuncDecl::Ptr decl) {
  decl->name = rewrite<Identifier>(decl->name);
  for (unsigned i = 0; i < decl->args.size(); ++i) {
    decl->args[i] = rewrite<Argument>(decl->args[i]);
  }
  for (unsigned i = 0; i < decl->results.size(); ++i) {
    decl->results[i] = rewrite<IdentDecl>(decl->results[i]);
  }
  decl->body = rewrite<StmtBlock>(decl->body);
  node = decl;
}

void HIRRewriter::visit(ProcDecl::Ptr decl) {
  visit(to<FuncDecl>(decl)); 
}

void HIRRewriter::visit(VarDecl::Ptr decl) {
  decl->var = rewrite<IdentDecl>(decl->var);
  if (decl->initVal) {
    decl->initVal = rewrite<Expr>(decl->initVal);
  }
  node = decl;
}

void HIRRewriter::visit(ConstDecl::Ptr decl) {
  visit(to<VarDecl>(decl));
}

void HIRRewriter::visit(WhileStmt::Ptr stmt) {
  stmt->cond = rewrite<Expr>(stmt->cond);
  stmt->body = rewrite<StmtBlock>(stmt->body);
  node = stmt;
}

void HIRRewriter::visit(DoWhileStmt::Ptr stmt) {
  visit(to<WhileStmt>(stmt));
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
  stmt->expr = rewrite<Expr>(stmt->expr);
  node = stmt;
}

void HIRRewriter::visit(ExprStmt::Ptr stmt) {
  stmt->expr = rewrite<Expr>(stmt->expr);
  node = stmt;
}

void HIRRewriter::visit(AssignStmt::Ptr stmt) {
  for (unsigned i = 0; i < stmt->lhs.size(); ++i) {
    stmt->lhs[i] = rewrite<Expr>(stmt->lhs[i]);
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
  for (unsigned i = 0; i < expr->partialActuals.size(); ++i) {
    expr->partialActuals[i] = rewrite<Expr>(expr->partialActuals[i]);
  }
  expr->target = rewrite<Identifier>(expr->target);
  node = expr;
}

void HIRRewriter::visit(UnaryExpr::Ptr expr) {
  expr->operand = rewrite<Expr>(expr->operand);
  node = expr;
}

void HIRRewriter::visit(BinaryExpr::Ptr expr) {
  expr->lhs = rewrite<Expr>(expr->lhs);
  expr->rhs = rewrite<Expr>(expr->rhs);
  node = expr;
}

void HIRRewriter::visit(NaryExpr::Ptr expr) {
  for (unsigned i = 0; i < expr->operands.size(); ++i) {
    expr->operands[i] = rewrite<Expr>(expr->operands[i]);
  }
  node = expr;
}

void HIRRewriter::visit(OrExpr::Ptr expr) {
  visit(to<BinaryExpr>(expr));
}

void HIRRewriter::visit(AndExpr::Ptr expr) {
  visit(to<BinaryExpr>(expr));
}

void HIRRewriter::visit(XorExpr::Ptr expr) {
  visit(to<BinaryExpr>(expr));
}

void HIRRewriter::visit(EqExpr::Ptr expr) {
  visit(to<NaryExpr>(expr));
}

void HIRRewriter::visit(NotExpr::Ptr expr) {
  visit(to<UnaryExpr>(expr));
}

void HIRRewriter::visit(AddExpr::Ptr expr) {
  visit(to<BinaryExpr>(expr));
}

void HIRRewriter::visit(SubExpr::Ptr expr) {
  visit(to<BinaryExpr>(expr));
}

void HIRRewriter::visit(MulExpr::Ptr expr) {
  visit(to<BinaryExpr>(expr));
}

void HIRRewriter::visit(DivExpr::Ptr expr) {
  visit(to<BinaryExpr>(expr));
}

void HIRRewriter::visit(ElwiseMulExpr::Ptr expr) {
  visit(to<BinaryExpr>(expr));
}

void HIRRewriter::visit(ElwiseDivExpr::Ptr expr) {
  visit(to<BinaryExpr>(expr));
}

void HIRRewriter::visit(NegExpr::Ptr expr) {
  visit(to<UnaryExpr>(expr));
}

void HIRRewriter::visit(ExpExpr::Ptr expr) {
  visit(to<BinaryExpr>(expr));
}

void HIRRewriter::visit(TransposeExpr::Ptr expr) {
  visit(to<UnaryExpr>(expr));
}

void HIRRewriter::visit(CallExpr::Ptr expr) {
  expr->func = rewrite<Identifier>(expr->func);
  for (unsigned i = 0; i < expr->operands.size(); ++i) {
    expr->operands[i] = rewrite<Expr>(expr->operands[i]);
  }
  node = expr;
}

void HIRRewriter::visit(TensorReadExpr::Ptr expr) {
  expr->tensor = rewrite<Expr>(expr->tensor);
  for (unsigned i = 0; i < expr->indices.size(); ++i) {
    expr->indices[i] = rewrite<ReadParam>(expr->indices[i]);
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

void HIRRewriter::visit(DenseNDTensor::Ptr tensor) {
  for (unsigned i = 0; i < tensor->elems.size(); ++i) {
    tensor->elems[i] = rewrite<DenseTensorElement>(tensor->elems[i]);
  }
  node = tensor;
}

void HIRRewriter::visit(DenseTensorLiteral::Ptr tensor) {
  tensor->tensor = rewrite<DenseTensorElement>(tensor->tensor);
  node = tensor;
}

void HIRRewriter::visit(Test::Ptr test) {
  test->func = rewrite<Identifier>(test->func);
  for (unsigned i = 0; i < test->args.size(); ++i) {
    test->args[i] = rewrite<Expr>(test->args[i]);
  }
  test->expected = rewrite<Expr>(test->expected);
  node = test;
}

}
}

