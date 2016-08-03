#include "fir_visitor.h"
#include "fir.h"

namespace simit {
namespace fir {

void FIRVisitor::visit(Program::Ptr program) {
  for (auto elem : program->elems) {
    elem->accept(this);
  }
}

void FIRVisitor::visit(StmtBlock::Ptr stmtBlock) {
  for (auto stmt : stmtBlock->stmts) {
    stmt->accept(this);
  }
}

void FIRVisitor::visit(GenericIndexSet::Ptr set) {
  visit(to<SetIndexSet>(set));
}

void FIRVisitor::visit(Endpoint::Ptr end) {
  end->set->accept(this);
  if (end->element) {
    end->element->accept(this);
  }
}

void FIRVisitor::visit(UnstructuredSetType::Ptr type) {
  type->element->accept(this);
  for (auto endpoint : type->endpoints) {
    endpoint->accept(this);
  }
}

void FIRVisitor::visit(LatticeLinkSetType::Ptr type) {
  type->element->accept(this);
  type->latticePointSet->accept(this);
}

void FIRVisitor::visit(TupleType::Ptr type) {
  type->element->accept(this);
  type->length->accept(this);
}

void FIRVisitor::visit(NDTensorType::Ptr type) {
  for (auto indexSet : type->indexSets) {
    indexSet->accept(this);
  }
  type->blockType->accept(this);
}

void FIRVisitor::visit(IdentDecl::Ptr decl) {
  decl->name->accept(this);
  decl->type->accept(this);
}

void FIRVisitor::visit(FieldDecl::Ptr decl) {
  visit(to<IdentDecl>(decl));
}

void FIRVisitor::visit(ElementTypeDecl::Ptr decl) {
  decl->name->accept(this);
  for (auto field : decl->fields) {
    field->accept(this);
  }
}

void FIRVisitor::visit(Argument::Ptr arg) {
  visit(to<IdentDecl>(arg));
}

void FIRVisitor::visit(InOutArgument::Ptr arg) {
  visit(to<Argument>(arg)); 
}

void FIRVisitor::visit(ExternDecl::Ptr decl) {
  visit(to<IdentDecl>(decl));
}

void FIRVisitor::visit(FuncDecl::Ptr decl) {
  decl->name->accept(this);
  for (auto genericParam : decl->genericParams) {
    genericParam->accept(this);
  }
  for (auto arg : decl->args) {
    arg->accept(this);
  }
  for (auto result : decl->results) {
    result->accept(this);
  }
  if (decl->body) {
    decl->body->accept(this);
  }
}

void FIRVisitor::visit(VarDecl::Ptr decl) {
  decl->name->accept(this);
  if (decl->type) {
    decl->type->accept(this);
  }
  if (decl->initVal) {
    decl->initVal->accept(this);
  }
}

void FIRVisitor::visit(ConstDecl::Ptr decl) {
  visit(to<VarDecl>(decl));
}

void FIRVisitor::visit(WhileStmt::Ptr stmt) {
  stmt->cond->accept(this);
  stmt->body->accept(this);
}

void FIRVisitor::visit(DoWhileStmt::Ptr stmt) {
  visit(to<WhileStmt>(stmt));
}

void FIRVisitor::visit(IfStmt::Ptr stmt) {
  stmt->cond->accept(this);
  stmt->ifBody->accept(this);
  if (stmt->elseBody) {
    stmt->elseBody->accept(this);
  }
}

void FIRVisitor::visit(IndexSetDomain::Ptr domain) {
  domain->set->accept(this);
}

void FIRVisitor::visit(RangeDomain::Ptr domain) {
  domain->lower->accept(this);
  domain->upper->accept(this);
}

void FIRVisitor::visit(ForStmt::Ptr stmt) {
  stmt->loopVar->accept(this);
  stmt->domain->accept(this);
  stmt->body->accept(this);
}

void FIRVisitor::visit(PrintStmt::Ptr stmt) {
  for (auto arg : stmt->args) {
    arg->accept(this);
  }
}

void FIRVisitor::visit(ExprStmt::Ptr stmt) {
  stmt->expr->accept(this);
}

void FIRVisitor::visit(AssignStmt::Ptr stmt) {
  for (auto lhs : stmt->lhs) {
    lhs->accept(this);
  }
  stmt->expr->accept(this);
}

void FIRVisitor::visit(ExprParam::Ptr param) {
  param->expr->accept(this);
}

void FIRVisitor::visit(MapExpr::Ptr expr) {
  expr->func->accept(this);
  for (auto genericArg : expr->genericArgs) {
    genericArg->accept(this);
  }
  for (auto arg : expr->partialActuals) {
    arg->accept(this);
  }
  expr->target->accept(this);
  if (expr->through) {
    expr->through->accept(this);
  }
}

void FIRVisitor::visit(ReducedMapExpr::Ptr expr) {
  visit(to<MapExpr>(expr));
}

void FIRVisitor::visit(UnreducedMapExpr::Ptr expr) {
  visit(to<MapExpr>(expr));
}

void FIRVisitor::visit(OrExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRVisitor::visit(AndExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRVisitor::visit(XorExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRVisitor::visit(EqExpr::Ptr expr) {
  visitNaryExpr(expr);
}

void FIRVisitor::visit(NotExpr::Ptr expr) {
  visitUnaryExpr(expr);
}

void FIRVisitor::visit(AddExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRVisitor::visit(SubExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRVisitor::visit(MulExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRVisitor::visit(DivExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRVisitor::visit(LeftDivExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRVisitor::visit(ElwiseMulExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRVisitor::visit(ElwiseDivExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRVisitor::visit(NegExpr::Ptr expr) {
  visitUnaryExpr(expr);
}

void FIRVisitor::visit(ExpExpr::Ptr expr) {
  visitBinaryExpr(expr);
}

void FIRVisitor::visit(TransposeExpr::Ptr expr) {
  visitUnaryExpr(expr);
}

void FIRVisitor::visit(CallExpr::Ptr expr) {
  expr->func->accept(this);
  for (auto genericArg : expr->genericArgs) {
    genericArg->accept(this);
  }
  for (auto arg : expr->args) {
    arg->accept(this);
  }
}

void FIRVisitor::visit(TensorReadExpr::Ptr expr) {
  expr->tensor->accept(this);
  for (auto param : expr->indices) {
    param->accept(this);
  }
}

void FIRVisitor::visit(SetReadExpr::Ptr expr) {
  expr->set->accept(this);
  for (auto param : expr->indices) {
    param->accept(this);
  }
}

void FIRVisitor::visit(TupleReadExpr::Ptr expr) {
  expr->tuple->accept(this);
  expr->index->accept(this);
}

void FIRVisitor::visit(FieldReadExpr::Ptr expr) {
  expr->setOrElem->accept(this);
  expr->field->accept(this);
}

void FIRVisitor::visit(ParenExpr::Ptr expr) {
  expr->expr->accept(this);
}

void FIRVisitor::visit(RangeConst::Ptr expr) {
  visit(to<VarExpr>(expr));
}

void FIRVisitor::visit(NDTensorLiteral::Ptr lit) {
  for (auto elem : lit->elems) {
    elem->accept(this);
  }
}

void FIRVisitor::visit(ApplyStmt::Ptr stmt) {
  stmt->map->accept(this);
}

void FIRVisitor::visit(Test::Ptr test) {
  test->func->accept(this);
  for (auto arg : test->args) {
    arg->accept(this);
  }
  test->expected->accept(this);
}

void FIRVisitor::visitUnaryExpr(UnaryExpr::Ptr expr) {
  expr->operand->accept(this);
}

void FIRVisitor::visitBinaryExpr(BinaryExpr::Ptr expr) {
  expr->lhs->accept(this);
  expr->rhs->accept(this);
}

void FIRVisitor::visitNaryExpr(NaryExpr::Ptr expr) {
  for (auto operand : expr->operands) {
    operand->accept(this);
  }
}

}
}

