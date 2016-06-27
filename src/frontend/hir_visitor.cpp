#include "hir_visitor.h"
#include "hir.h"

namespace simit {
namespace hir {

void HIRVisitor::visit(Program::Ptr program) {
  for (auto elem : program->elems) {
    elem->accept(this);
  }
}

void HIRVisitor::visit(StmtBlock::Ptr stmtBlock) {
  for (auto stmt : stmtBlock->stmts) {
    stmt->accept(this);
  }
}

void HIRVisitor::visit(GenericIndexSet::Ptr set) {
  visit(to<SetIndexSet>(set));
}

void HIRVisitor::visit(SetType::Ptr type) {
  type->element->accept(this);
  for (auto endpoint : type->endpoints) {
    endpoint->accept(this);
  }
}

void HIRVisitor::visit(TupleType::Ptr type) {
  type->element->accept(this);
  type->length->accept(this);
}

void HIRVisitor::visit(NDTensorType::Ptr type) {
  for (auto indexSet : type->indexSets) {
    indexSet->accept(this);
  }
  type->blockType->accept(this);
}

void HIRVisitor::visit(IdentDecl::Ptr decl) {
  decl->name->accept(this);
  decl->type->accept(this);
}

void HIRVisitor::visit(FieldDecl::Ptr decl) {
  decl->field->accept(this);
}

void HIRVisitor::visit(ElementTypeDecl::Ptr decl) {
  decl->name->accept(this);
  for (auto field : decl->fields) {
    field->accept(this);
  }
}

void HIRVisitor::visit(Argument::Ptr arg) {
  arg->arg->accept(this);
}

void HIRVisitor::visit(InOutArgument::Ptr arg) {
  visit(to<Argument>(arg)); 
}

void HIRVisitor::visit(ExternDecl::Ptr decl) {
  decl->var->accept(this);
}

void HIRVisitor::visit(FuncDecl::Ptr decl) {
  decl->name->accept(this);
  for (auto typeParam : decl->typeParams) {
    typeParam->accept(this);
  }
  for (auto arg : decl->args) {
    arg->accept(this);
  }
  for (auto result : decl->results) {
    result->accept(this);
  }
  decl->body->accept(this);
}

void HIRVisitor::visit(VarDecl::Ptr decl) {
  decl->name->accept(this);
  if (decl->type) {
    decl->type->accept(this);
  }
  if (decl->initVal) {
    decl->initVal->accept(this);
  }
}

void HIRVisitor::visit(ConstDecl::Ptr decl) {
  visit(to<VarDecl>(decl));
}

void HIRVisitor::visit(WhileStmt::Ptr stmt) {
  stmt->cond->accept(this);
  stmt->body->accept(this);
}

void HIRVisitor::visit(DoWhileStmt::Ptr stmt) {
  visit(to<WhileStmt>(stmt));
}

void HIRVisitor::visit(IfStmt::Ptr stmt) {
  stmt->cond->accept(this);
  stmt->ifBody->accept(this);
  if (stmt->elseBody) {
    stmt->elseBody->accept(this);
  }
}

void HIRVisitor::visit(IndexSetDomain::Ptr domain) {
  domain->set->accept(this);
}

void HIRVisitor::visit(RangeDomain::Ptr domain) {
  domain->lower->accept(this);
  domain->upper->accept(this);
}

void HIRVisitor::visit(ForStmt::Ptr stmt) {
  stmt->loopVar->accept(this);
  stmt->domain->accept(this);
  stmt->body->accept(this);
}

void HIRVisitor::visit(PrintStmt::Ptr stmt) {
  for (auto arg : stmt->args) {
    arg->accept(this);
  }
}

void HIRVisitor::visit(ExprStmt::Ptr stmt) {
  stmt->expr->accept(this);
}

void HIRVisitor::visit(AssignStmt::Ptr stmt) {
  for (auto lhs : stmt->lhs) {
    lhs->accept(this);
  }
  visit(to<ExprStmt>(stmt));
}

void HIRVisitor::visit(ExprParam::Ptr param) {
  param->expr->accept(this);
}

void HIRVisitor::visit(MapExpr::Ptr expr) {
  expr->func->accept(this);
  for (auto param : expr->partialActuals) {
    param->accept(this);
  }
  expr->target->accept(this);
}

void HIRVisitor::visit(ReducedMapExpr::Ptr expr) {
  visit(to<MapExpr>(expr));
}

void HIRVisitor::visit(UnreducedMapExpr::Ptr expr) {
  visit(to<MapExpr>(expr));
}

void HIRVisitor::visit(OrExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRVisitor::visit(AndExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRVisitor::visit(XorExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRVisitor::visit(EqExpr::Ptr expr) {
  visitNaryExpr(expr);
}

void HIRVisitor::visit(NotExpr::Ptr expr) {
  visitUnaryExpr(expr);
}

void HIRVisitor::visit(AddExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRVisitor::visit(SubExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRVisitor::visit(MulExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRVisitor::visit(DivExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRVisitor::visit(LeftDivExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRVisitor::visit(ElwiseMulExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRVisitor::visit(ElwiseDivExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRVisitor::visit(NegExpr::Ptr expr) {
  visitUnaryExpr(expr);
}

void HIRVisitor::visit(ExpExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void HIRVisitor::visit(TransposeExpr::Ptr expr) {
  visitUnaryExpr(expr);
}

void HIRVisitor::visit(CallExpr::Ptr expr) {
  expr->func->accept(this);
  for (auto arg : expr->args) {
    if (arg) {
      arg->accept(this);
    }
  }
}

void HIRVisitor::visit(TensorReadExpr::Ptr expr) {
  expr->tensor->accept(this);
  for (auto param : expr->indices) {
    param->accept(this);
  }
}

void HIRVisitor::visit(TupleReadExpr::Ptr expr) {
  expr->tuple->accept(this);
  if (expr->index) {
    expr->index->accept(this);
  }
}

void HIRVisitor::visit(FieldReadExpr::Ptr expr) {
  expr->setOrElem->accept(this);
  expr->field->accept(this);
}

void HIRVisitor::visit(ParenExpr::Ptr expr) {
  expr->expr->accept(this);
}

void HIRVisitor::visit(NDTensorLiteral::Ptr lit) {
  for (auto elem : lit->elems) {
    elem->accept(this);
  }
}

void HIRVisitor::visit(ApplyStmt::Ptr stmt) {
  stmt->map->accept(this);
}

void HIRVisitor::visit(Test::Ptr test) {
  test->func->accept(this);
  for (auto arg : test->args) {
    arg->accept(this);
  }
  test->expected->accept(this);
}

void HIRVisitor::visitUnaryExpr(UnaryExpr::Ptr expr) {
  expr->operand->accept(this);
}

void HIRVisitor::visitBinaryExpr(BinaryExpr::Ptr expr) {
  expr->lhs->accept(this);
  expr->rhs->accept(this);
}

void HIRVisitor::visitNaryExpr(NaryExpr::Ptr expr) {
  for (auto operand : expr->operands) {
    operand->accept(this);
  }
}

}
}

