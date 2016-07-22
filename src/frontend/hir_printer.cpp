#include <iostream>

#include "hir_printer.h"
#include "error.h"

namespace simit {
namespace hir {

void HIRPrinter::visit(Program::Ptr program) {
  for (auto elem : program->elems) {
    elem->accept(this);
    oss << std::endl;
  }
}

void HIRPrinter::visit(StmtBlock::Ptr stmtBlock) {
  for (auto stmt : stmtBlock->stmts) {
    stmt->accept(this);
    oss << std::endl;
  }
}

void HIRPrinter::visit(RangeIndexSet::Ptr set) {
  oss << "0.." << set->range;
}

void HIRPrinter::visit(SetIndexSet::Ptr set) {
  oss << set->setName;
}

void HIRPrinter::visit(DynamicIndexSet::Ptr set) {
  oss << "*";
}

void HIRPrinter::visit(ElementType::Ptr set) {
  oss << set->ident;
}

void HIRPrinter::visit(Endpoint::Ptr end) {
  oss << end->setName;
}

void HIRPrinter::visit(SetType::Ptr type) {
  if (type->type == SetType::Type::UNSTRUCTURED) {
    oss << "set{";
    type->element->accept(this);
    oss << "}";
    if (type->endpoints.size() > 0) {
      oss << "(";
      bool printDelimiter = false;
      for (auto endpoint : type->endpoints) {
        if (printDelimiter) {
          oss << ", ";
        }
        endpoint->accept(this);
        printDelimiter = true;
      }
      oss << ")";
    }
  }
  else if (type->type == SetType::Type::LATTICE_LINK) {
    oss << "lattice[" << type->dimensions << "]{";
    type->element->accept(this);
    oss << "}(";
    type->latticePointSet->accept(this);
    oss << ")";
  }
  else {
    unreachable;
  }
}

void HIRPrinter::visit(TupleLength::Ptr length) {
  oss << length->val;
}

void HIRPrinter::visit(TupleType::Ptr type) {
  oss << "(";
  type->element->accept(this);
  oss << " * ";
  type->length->accept(this);
  oss << ")";
}

void HIRPrinter::visit(ScalarType::Ptr type) {
  switch (type->type) {
    case ScalarType::Type::INT:
      oss << "int";
      break;
    case ScalarType::Type::FLOAT:
      oss << "float";
      break;
    case ScalarType::Type::BOOL:
      oss << "bool";
      break;
    case ScalarType::Type::COMPLEX:
      oss << "complex";
      break;
    default:
      unreachable;
      break;
  }
}

void HIRPrinter::visit(NDTensorType::Ptr type) {
  oss << "tensor";
  if (type->indexSets.size() > 0) {
    oss << "[";
    bool printDelimiter = false;
    for (auto indexSet : type->indexSets) {
      if (printDelimiter) {
        oss << ", ";
      }
      indexSet->accept(this);
      printDelimiter = true;
    }
    oss << "]";
  }
  oss << "(";
  type->blockType->accept(this);
  oss << ")";
  if (type->transposed) {
    oss << "'";
  }
}

void HIRPrinter::visit(Identifier::Ptr ident) {
  oss << ident->ident;
}

void HIRPrinter::visit(IdentDecl::Ptr decl) {
  decl->name->accept(this);
  oss << " : ";
  decl->type->accept(this);
}

void HIRPrinter::visit(Field::Ptr field) {
  field->field->accept(this);
  oss << ";";
}

void HIRPrinter::visit(ElementTypeDecl::Ptr decl) {
  oss << "element ";
  decl->name->accept(this);
  oss << std::endl;
  indent();
  for (auto field : decl->fields) {
    printIndent();
    field->accept(this);
    oss << std::endl;
  }
  dedent();
  oss << "end";
}

void HIRPrinter::visit(Argument::Ptr arg) {
  if (arg->isInOut()) {
    oss << "inout ";
  }
  arg->arg->accept(this);
}

void HIRPrinter::visit(ExternDecl::Ptr decl) {
  oss << "extern ";
  decl->var->accept(this);
  oss << ";";
}

void HIRPrinter::visit(FuncDecl::Ptr decl) {
  if (decl->exported) {
    oss << "export ";
  }
  oss << "func ";
  decl->name->accept(this);
  oss << "(";
  bool printDelimiter = false;
  for (auto arg : decl->args) {
    if (printDelimiter) {
      oss << ", ";
    }
    arg->accept(this);
    printDelimiter = true;
  }
  oss << ") ";
  if (decl->results.size() > 0) {
    oss << "-> (";
    printDelimiter = false;
    for (auto result : decl->results) {
      if (printDelimiter) {
        oss << ", ";
      }
      result->accept(this);
      printDelimiter = true;
    }
    oss << ")";
  }
  oss << std::endl;
  indent();
  decl->body->accept(this);
  dedent();
  oss << "end";
}

void HIRPrinter::visit(VarDecl::Ptr decl) {
  printVarOrConstDecl(decl);
}

void HIRPrinter::visit(ConstDecl::Ptr decl) {
  printVarOrConstDecl(decl, true);
}

void HIRPrinter::visit(WhileStmt::Ptr stmt) {
  printIndent();
  oss << "while ";
  stmt->cond->accept(this);
  oss << std::endl;
  indent();
  stmt->body->accept(this);
  dedent();
  printIndent();
  oss << "end";
}

void HIRPrinter::visit(DoWhileStmt::Ptr stmt) {
  printIndent();
  oss << "do " << std::endl;
  indent();
  stmt->body->accept(this);
  dedent();
  printIndent();
  oss << "end while ";
  stmt->cond->accept(this);
}

void HIRPrinter::visit(IfStmt::Ptr stmt) {
  printIndent();
  oss << "if ";
  stmt->cond->accept(this);
  oss << std::endl;
  indent();
  stmt->ifBody->accept(this);
  dedent();
  if (stmt->elseBody) {
    printIndent();
    oss << "else" << std::endl;
    indent();
    stmt->elseBody->accept(this);
    dedent();
    oss << std::endl;
  }
  printIndent();
  oss << "end";
}

void HIRPrinter::visit(IndexSetDomain::Ptr domain) {
  domain->set->accept(this);
}

void HIRPrinter::visit(RangeDomain::Ptr domain) {
  domain->lower->accept(this);
  oss << " : ";
  domain->upper->accept(this);
}

void HIRPrinter::visit(ForStmt::Ptr stmt) {
  printIndent();
  oss << "for ";
  stmt->loopVar->accept(this);
  oss << " in ";
  stmt->domain->accept(this);
  oss << std::endl;
  indent();
  stmt->body->accept(this);
  dedent();
  printIndent();
  oss << "end";
}

void HIRPrinter::visit(PrintStmt::Ptr stmt) {
  printIndent();
  oss << (stmt->printNewline ? "println " : "print ");
  bool printDelimiter = false;
  for (auto arg : stmt->args) {
    if (printDelimiter) {
      oss << ", ";
    }
    arg->accept(this);
    printDelimiter = true;
  }
  oss << ";";
}

void HIRPrinter::visit(ExprStmt::Ptr stmt) {
  printIndent();
  stmt->expr->accept(this);
  oss << ";";
}

void HIRPrinter::visit(AssignStmt::Ptr stmt) {
  printIndent();
  bool printDelimiter = false;
  for (auto lhs : stmt->lhs) {
    if (printDelimiter) {
      oss << ", ";
    }
    lhs->accept(this);
    printDelimiter = true;
  }
  oss << " = ";
  stmt->expr->accept(this);
  oss << ";";
}

void HIRPrinter::visit(Slice::Ptr slice) {
  oss << ":";
}

void HIRPrinter::visit(ExprParam::Ptr param) {
  param->expr->accept(this);
}

void HIRPrinter::visit(MapExpr::Ptr expr) {
  printMapOrApply(expr);
}

void HIRPrinter::visit(OrExpr::Ptr expr) {
  printBinaryExpr(expr, "or");
}

void HIRPrinter::visit(AndExpr::Ptr expr) {
  printBinaryExpr(expr, "and");
}

void HIRPrinter::visit(XorExpr::Ptr expr) {
  printBinaryExpr(expr, "xor");
}

void HIRPrinter::visit(EqExpr::Ptr expr) {
  oss << "(";
  expr->operands[0]->accept(this);
  oss << ")";
  for (unsigned i = 0; i < expr->ops.size(); ++i) {
    switch (expr->ops[i]) {
      case EqExpr::Op::LT:
        oss << " < ";
        break;
      case EqExpr::Op::LE:
        oss << " <= ";
        break;
      case EqExpr::Op::GT:
        oss << " > ";
        break;
      case EqExpr::Op::GE:
        oss << " >= ";
        break;
      case EqExpr::Op::EQ:
        oss << " == ";
        break;
      case EqExpr::Op::NE:
        oss << " != ";
        break;
      default:
        unreachable;
        break;
    }
    oss << "(";
    expr->operands[i + 1]->accept(this);
    oss << ")";
  }
}

void HIRPrinter::visit(NotExpr::Ptr expr) {
  printUnaryExpr(expr, "not");
}

void HIRPrinter::visit(AddExpr::Ptr expr) {
  printBinaryExpr(expr, "+");
}

void HIRPrinter::visit(SubExpr::Ptr expr) {
  printBinaryExpr(expr, "-");
}

void HIRPrinter::visit(MulExpr::Ptr expr) {
  printBinaryExpr(expr, "*");
}

void HIRPrinter::visit(DivExpr::Ptr expr) {
  printBinaryExpr(expr, "/");
}

void HIRPrinter::visit(LeftDivExpr::Ptr expr) {
  printBinaryExpr(expr, "\\");
}

void HIRPrinter::visit(ElwiseMulExpr::Ptr expr) {
  printBinaryExpr(expr, ".*");
}

void HIRPrinter::visit(ElwiseDivExpr::Ptr expr) {
  printBinaryExpr(expr, "./");
}

void HIRPrinter::visit(NegExpr::Ptr expr) {
  printUnaryExpr(expr, expr->negate ? "-" : "+");
}

void HIRPrinter::visit(ExpExpr::Ptr expr) {
  printBinaryExpr(expr, "^");
}

void HIRPrinter::visit(TransposeExpr::Ptr expr) {
  printUnaryExpr(expr, "'", true);
}

void HIRPrinter::visit(CallExpr::Ptr expr) {
  expr->func->accept(this);
  oss << "(";
  bool printDelimiter = false;
  for (auto arg : expr->args) {
    if (printDelimiter) {
      oss << ", ";
    }
    if (arg) {
      arg->accept(this);
    } else {
      oss << "?";
    }
    printDelimiter = true;
  }
  oss << ")";
}

void HIRPrinter::visit(TensorReadExpr::Ptr expr) {
  expr->tensor->accept(this);
  oss << "(";
  bool printDelimiter = false;
  for (auto param : expr->indices) {
    if (printDelimiter) {
      oss << ", ";
    }
    param->accept(this);
    printDelimiter = true;
  }
  oss << ")";
}

void HIRPrinter::visit(SetReadExpr::Ptr expr) {
  expr->set->accept(this);
  oss << "[";
  bool printDelimiter = false;
  for (auto param : expr->indices) {
    if (printDelimiter) {
      oss << ", ";
    }
    param->accept(this);
    printDelimiter = true;
  }
  oss << "]";
}

void HIRPrinter::visit(TupleReadExpr::Ptr expr) {
  expr->tuple->accept(this);
  oss << "(";
  expr->index->accept(this);
  oss << ")";
}

void HIRPrinter::visit(FieldReadExpr::Ptr expr) {
  expr->setOrElem->accept(this);
  oss << ".";
  expr->field->accept(this);
}

void HIRPrinter::visit(VarExpr::Ptr lit) {
  oss << lit->ident;
}

void HIRPrinter::visit(IntLiteral::Ptr lit) {
  oss << lit->val;
}

void HIRPrinter::visit(FloatLiteral::Ptr lit) {
  oss << lit->val;
}

void HIRPrinter::visit(BoolLiteral::Ptr lit) {
  printBoolean(lit->val);
}

void HIRPrinter::visit(ComplexLiteral::Ptr lit) {
  printComplex(lit->val);
}

void HIRPrinter::visit(IntVectorLiteral::Ptr lit) {
  oss << "[";
  bool printDelimiter = false;
  for (auto val : lit->vals) {
    if (printDelimiter) {
      oss << ", ";
    }
    oss << val;
    printDelimiter = true;
  }
  oss << "]";
  if (lit->transposed) {
    oss << "'";
  }
}

void HIRPrinter::visit(FloatVectorLiteral::Ptr lit) {
  oss << "[";
  bool printDelimiter = false;
  for (auto val : lit->vals) {
    if (printDelimiter) {
      oss << ", ";
    }
    oss << val;
    printDelimiter = true;
  }
  oss << "]";
  if (lit->transposed) {
    oss << "'";
  }
}

void HIRPrinter::visit(ComplexVectorLiteral::Ptr lit) {
  oss << "[";
  bool printDelimiter = false;
  for (auto val : lit->vals) {
    if (printDelimiter) {
      oss << ", ";
    }
    printComplex(val);
    printDelimiter = true;
  }
  oss << "]";
  if (lit->transposed) {
    oss << "'";
  }
}

void HIRPrinter::visit(NDTensorLiteral::Ptr lit) {
  oss << "[";
  bool printDelimiter = false;
  for (auto elem : lit->elems) {
    if (printDelimiter) {
      oss << ", ";
    }
    elem->accept(this);
    printDelimiter = true;
  }
  oss << "]";
  if (lit->transposed) {
    oss << "'";
  }
}

void HIRPrinter::visit(ApplyStmt::Ptr stmt) {
  printMapOrApply(stmt->map, true);
  oss << ";";
}

void HIRPrinter::visit(Test::Ptr test) {
  oss << "%! ";
  test->func->accept(this);
  oss << "(";
  bool printDelimiter = false;
  for (auto arg : test->args) {
    if (printDelimiter) {
      oss << ", ";
    }
    arg->accept(this);
    printDelimiter = true;
  }
  oss << ") == ";
  test->expected->accept(this);
  oss << ";";
}

void HIRPrinter::printVarOrConstDecl(VarDecl::Ptr decl, const bool isConst) {
  printIndent();
  oss << (isConst ? "const " : "var ");
  decl->name->accept(this);
  if (decl->type) {
    oss << " : ";
    decl->type->accept(this);
  }
  if (decl->initVal) {
    oss << " = ";
    decl->initVal->accept(this);
  }
  oss << ";";
}

void HIRPrinter::printMapOrApply(MapExpr::Ptr expr, const bool isApply) {
  oss << (isApply ? "apply " : "map ");
  expr->func->accept(this);
  if (expr->partialActuals.size() > 0) {
    oss << "(";
    bool printDelimiter = false;
    for (auto param : expr->partialActuals) {
      if (printDelimiter) {
        oss << ", ";
      }
      param->accept(this);
      printDelimiter = true;
    }
    oss << ")";
  }
  oss << " to ";
  expr->target->accept(this);
  if (expr->through) {
    oss << " through ";
    expr->through->accept(this);
  }
  if (expr->isReduced()) {
    oss << " reduce ";
    switch (to<ReducedMapExpr>(expr)->op) {
      case ReducedMapExpr::ReductionOp::SUM:
        oss << "+";
        break;
      default:
        unreachable;
        break;
    }
  }
}

void HIRPrinter::printUnaryExpr(UnaryExpr::Ptr expr, const std::string op, 
                                const bool opAfterOperand) {
  if (!opAfterOperand) {
    oss << op;
  }
  oss << "(";
  expr->operand->accept(this);
  oss << ")";
  if (opAfterOperand) {
    oss << op;
  }
}

void HIRPrinter::printBinaryExpr(BinaryExpr::Ptr expr, const std::string op) {
  oss << "(";
  expr->lhs->accept(this);
  oss << ") " << op << " (";
  expr->rhs->accept(this);
  oss << ")";
}

std::ostream &operator<<(std::ostream &oss, HIRNode &node) {
  HIRPrinter printer(oss);
  node.accept(&printer);
  return oss;
}

}
}

