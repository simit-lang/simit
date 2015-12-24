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
  node = type;
}

void HIRRewriter::visit(NonScalarTensorType::Ptr type) {
  for (unsigned i = 0; i < type->indexSets.size(); ++i) {
    type->indexSets[i] = rewrite<IndexSet>(type->indexSets[i]);
  }
  type->blockType = rewrite<TensorType>(type->blockType);
  node = type;
}

void HIRRewriter::visit(Field::Ptr field) {
  field->type = rewrite<TensorType>(field->type);
  node = field;
}

void HIRRewriter::visit(ElementTypeDecl::Ptr decl) {
  for (unsigned i = 0; i < decl->fields.size(); ++i) {
    decl->fields[i] = rewrite<Field>(decl->fields[i]);
  }
  node = decl;
}

void HIRRewriter::visit(IdentDecl::Ptr decl) {
  decl->type = rewrite<Type>(decl->type);
  node = decl;
}

void HIRRewriter::visit(Argument::Ptr arg) {
  visit(static_cast<IdentDecl::Ptr>(arg));
}

void HIRRewriter::visit(ExternDecl::Ptr decl) {
  decl->var = rewrite<Argument>(decl->var);
  node = decl;
}

void HIRRewriter::visit(FuncDecl::Ptr decl) {
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
  visit(static_cast<FuncDecl::Ptr>(decl)); 
}

void HIRRewriter::visit(VarDecl::Ptr decl) {
  decl->var = rewrite<IdentDecl>(decl->var);
  if (decl->initVal) {
    decl->initVal = rewrite<Expr>(decl->initVal);
  }
  node = decl;
}

void HIRRewriter::visit(ConstDecl::Ptr decl) {
  visit(static_cast<VarDecl::Ptr>(decl));
  node = decl;
}

void HIRRewriter::visit(WhileStmt::Ptr stmt) {
  stmt->cond = rewrite<Expr>(stmt->cond);
  stmt->body = rewrite<StmtBlock>(stmt->body);
  node = stmt;
}

void HIRRewriter::visit(DoWhileStmt::Ptr stmt) {
  visit(static_cast<WhileStmt::Ptr>(stmt));
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
  visit(static_cast<ExprStmt::Ptr>(stmt));
  node = stmt;
}

void HIRRewriter::visit(ExprParam::Ptr param) {
  param->expr = rewrite<Expr>(param->expr);
  node = param;
}

void HIRRewriter::visit(MapExpr::Ptr expr) {
  for (unsigned i = 0; i < expr->partialActuals.size(); ++i) {
    expr->partialActuals[i] = rewrite<Expr>(expr->partialActuals[i]);
  }
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
  visit(static_cast<BinaryExpr::Ptr>(expr));
}

void HIRRewriter::visit(AndExpr::Ptr expr) {
  visit(static_cast<BinaryExpr::Ptr>(expr));
}

void HIRRewriter::visit(XorExpr::Ptr expr) {
  visit(static_cast<BinaryExpr::Ptr>(expr));
}

void HIRRewriter::visit(EqExpr::Ptr expr) {
  visit(static_cast<NaryExpr::Ptr>(expr));
}

void HIRRewriter::visit(NotExpr::Ptr expr) {
  visit(static_cast<UnaryExpr::Ptr>(expr));
}

void HIRRewriter::visit(AddExpr::Ptr expr) {
  visit(static_cast<BinaryExpr::Ptr>(expr));
}

void HIRRewriter::visit(SubExpr::Ptr expr) {
  visit(static_cast<BinaryExpr::Ptr>(expr));
}

void HIRRewriter::visit(MulExpr::Ptr expr) {
  visit(static_cast<BinaryExpr::Ptr>(expr));
}

void HIRRewriter::visit(DivExpr::Ptr expr) {
  visit(static_cast<BinaryExpr::Ptr>(expr));
}

void HIRRewriter::visit(ElwiseMulExpr::Ptr expr) {
  visit(static_cast<BinaryExpr::Ptr>(expr));
}

void HIRRewriter::visit(ElwiseDivExpr::Ptr expr) {
  visit(static_cast<BinaryExpr::Ptr>(expr));
}

void HIRRewriter::visit(NegExpr::Ptr expr) {
  visit(static_cast<UnaryExpr::Ptr>(expr));
}

void HIRRewriter::visit(ExpExpr::Ptr expr) {
  visit(static_cast<BinaryExpr::Ptr>(expr));
}

void HIRRewriter::visit(TransposeExpr::Ptr expr) {
  visit(static_cast<UnaryExpr::Ptr>(expr));
}

void HIRRewriter::visit(CallExpr::Ptr expr) {
  visit(static_cast<NaryExpr::Ptr>(expr));
}

void HIRRewriter::visit(TensorReadExpr::Ptr expr) {
  expr->tensor = rewrite<Expr>(expr->tensor);
  for (unsigned i = 0; i < expr->indices.size(); ++i) {
    expr->indices[i] = rewrite<ReadParam>(expr->indices[i]);
  }
  node = expr;
}

void HIRRewriter::visit(FieldReadExpr::Ptr expr) {
  expr->setOrElem = rewrite<Expr>(expr->setOrElem);
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
  for (unsigned i = 0; i < test->args.size(); ++i) {
    test->args[i] = rewrite<Expr>(test->args[i]);
  }
  test->expected = rewrite<Expr>(test->expected);
  node = test;
}

}
}

