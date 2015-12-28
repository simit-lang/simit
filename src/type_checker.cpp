#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>

#include "type_checker.h"
#include "program_context.h"
#include "types.h"
#include "domain.h"
#include "error.h"
#include "ir.h"
#include "hir.h"

// TODO: Refactor to reduce code duplication.
namespace simit {
namespace hir {

void TypeChecker::visit(RangeIndexSet::Ptr set) {
  retIndexSet = std::make_shared<ir::IndexSet>(set->range);
}

void TypeChecker::visit(SetIndexSet::Ptr set) {
  if (!ctx.hasSymbol(set->setName)) {
    return;
  }

  ir::Expr setExpr = ctx.getSymbol(set->setName).getExpr();
  
  if (!setExpr.type().isSet()) {
    reportError("index set must be a set, a range, or dynamic (*)", set);
    return;
  }

  retIndexSet = std::make_shared<ir::IndexSet>(setExpr);
}

void TypeChecker::visit(DynamicIndexSet::Ptr set) {
  retIndexSet = std::make_shared<ir::IndexSet>();
}

void TypeChecker::visit(ElementType::Ptr set) {
  if (!ctx.containsElementType(set->ident)) return;

  retIRType = ctx.getElementType(set->ident);
}

void TypeChecker::visit(Endpoint::Ptr end) {
  if (!ctx.hasSymbol(end->setName)) return;

  retExpr = ctx.getSymbol(end->setName).getExpr();
}

void TypeChecker::visit(SetType::Ptr type) {
  const ir::Type elementType = getIRType(type->element);
  bool typeChecked = elementType.defined();

  // TODO: Check for heterogeneous edge sets.
  std::vector<ir::Expr> endpoints;
  for (auto end : type->endpoints) {
    const ir::Expr endpoint = getExpr(end);
    if (endpoint.defined()) {
      endpoints.push_back(endpoint);
    } else {
      typeChecked = false;
    }
  }

  if (typeChecked) {
    retIRType = ir::Type(ir::SetType::make(elementType, endpoints));
  }
}

void TypeChecker::visit(TupleType::Ptr type) {
  const ir::Type elementType = getIRType(type->element);

  if (type->length < 1) {
    const auto err = ParseError(type->lineNum, type->colNum, type->lineNum, 
      type->colNum, "Tuple must have length greater than or equal to one");
    errors->push_back(err);
    return;
  }

  if (!elementType.defined()) return;

  retIRType = ir::Type(ir::TupleType::make(elementType, type->length));
}

void TypeChecker::visit(ScalarTensorType::Ptr type) {
  ir::ScalarType componentType;
  
  switch (type->type) {
    case ScalarTensorType::Type::INT:
      componentType = ir::ScalarType(ir::ScalarType::Int);
      break;
    case ScalarTensorType::Type::FLOAT:
      componentType = ir::ScalarType(ir::ScalarType::Float);
      break;
    case ScalarTensorType::Type::BOOL:
      componentType = ir::ScalarType(ir::ScalarType::Boolean);
      break;
    default:
      iassert(false);
      break;
  }
  
  retIRType = ir::Type(ir::TensorType::make(componentType));
}

void TypeChecker::visit(NonScalarTensorType::Ptr type) {
  const ir::Type blockType = getIRType(type->blockType);
  bool typeChecked = blockType.defined();

  std::vector<ir::IndexSet> indexSets;
  for (auto is : type->indexSets) {
    const IndexSetPtr indexSet = getIndexSet(is);
    if (indexSet) {
      indexSets.push_back(*indexSet);
    } else {
      typeChecked = false;
    }
  }

  if (!typeChecked) return;

  if (indexSets.empty()) {
    retIRType = blockType;
  } else {
    const auto blockTensorType = blockType.toTensor();
    const auto componentType = blockTensorType->componentType;
    const auto blockDimensions = blockTensorType->getDimensions();
  
    std::vector<ir::IndexDomain> dimensions;
    if (blockTensorType->order() == 0) {
      for (size_t i = 0; i< indexSets.size(); ++i) {
        dimensions.push_back(ir::IndexDomain(indexSets[i]));
      }
    } else {
      for (size_t i = 0; i< indexSets.size(); ++i) {
        std::vector<ir::IndexSet> dimension;
        dimension.push_back(indexSets[i]);
        dimension.insert(dimension.end(),
                         blockDimensions[i].getIndexSets().begin(),
                         blockDimensions[i].getIndexSets().end());
        dimensions.push_back(ir::IndexDomain(dimension));
      }
    }
  
    retIRType = ir::Type(ir::TensorType::make(componentType, dimensions));
  }

  if (type->transposed) {
    const auto tensorType = retIRType.toTensor();
    const auto dimensions = tensorType->getDimensions();
    const auto componentType = tensorType->componentType;
    retIRType = ir::Type(ir::TensorType::make(componentType, dimensions, true));
  }
}

void TypeChecker::visit(Field::Ptr field) {
  const ir::Type type = getIRType(field->type);
  retField = ir::Field(field->name, type);
}

void TypeChecker::visit(ElementTypeDecl::Ptr decl) {
  iassert(!ctx.containsElementType(decl->ident));
  
  std::vector<ir::Field> fields;
  for (auto f : decl->fields) {
    const ir::Field field = getField(f);
    fields.push_back(field);
  }

  ctx.addElementType(ir::ElementType::make(decl->ident, fields));
}

void TypeChecker::visit(IdentDecl::Ptr decl) {
  const ir::Type type = getIRType(decl->type);
  retVar = ir::Var(decl->ident, type);
}

void TypeChecker::visit(ExternDecl::Ptr decl) {
  const ir::Var externVar = getVar(decl->var);
  ctx.addSymbol(externVar);
}

void TypeChecker::visit(FuncDecl::Ptr decl) {
  ctx.scope();

  std::vector<ir::Var> arguments;
  for (auto arg : decl->args) {
    const ir::Var argVar = getVar(arg);
    ctx.addSymbol(argVar);
    arguments.push_back(argVar);
  }
  
  std::vector<ir::Var> results;
  for (auto res : decl->results) {
    const ir::Var result = getVar(res);
    ctx.addSymbol(result);
    results.push_back(result);
  }

  decl->body->accept(this);

  ctx.unscope();

  iassert(!ctx.containsFunction(decl->name));
  ctx.addFunction(ir::Func(decl->name, arguments, results, ir::Stmt()));
}

void TypeChecker::visit(VarDecl::Ptr decl) {
  typeCheckVarOrConstDecl(decl);
}

void TypeChecker::visit(ConstDecl::Ptr decl) {
  typeCheckVarOrConstDecl(decl, true);
}

void TypeChecker::visit(WhileStmt::Ptr stmt) {
  const TypePtr condType = inferType(stmt->cond);
 
  ctx.scope();
  stmt->body->accept(this);
  ctx.unscope();

  if (condType && (condType->size() != 1 || !isScalar((*condType)[0]) || 
      !(*condType)[0].toTensor()->componentType.isBoolean())) {
    std::stringstream errMsg;
    errMsg << "Conditional expression must be boolean";
    if (condType->size() > 0) {
      errMsg << " (actual type: " << typeString(condType) << ")";
    }
    const auto err = ParseError(stmt->cond->lineNum, stmt->cond->colNum, 
                                stmt->cond->lineNum, stmt->cond->colNum,
                                errMsg.str());
    errors->push_back(err);
  }
}

void TypeChecker::visit(IfStmt::Ptr stmt) {
  const TypePtr condType = inferType(stmt->cond);
  
  ctx.scope();
  stmt->ifBody->accept(this);
  ctx.unscope();

  if (stmt->elseBody) {
    ctx.scope();
    stmt->elseBody->accept(this);
    ctx.unscope();
  }

  if (condType && (condType->size() != 1 || !isScalar((*condType)[0]) || 
      !(*condType)[0].toTensor()->componentType.isBoolean())) {
    std::stringstream errMsg;
    errMsg << "Conditional expression must be boolean";
    if (condType->size() > 0) {
      errMsg << " (actual type: " << typeString(condType) << ")";
    }
    const auto err = ParseError(stmt->cond->lineNum, stmt->cond->colNum, 
                                stmt->cond->lineNum, stmt->cond->colNum,
                                errMsg.str());
    errors->push_back(err);
  }
}

void TypeChecker::visit(IndexSetDomain::Ptr domain) {
  const IndexSetPtr set = getIndexSet(domain->set);

  if (set && set->getKind() != ir::IndexSet::Set) {
    const auto err = ParseError(domain->lineNum, domain->colNum, 
                                domain->lineNum, domain->colNum, 
                                "For-loop cannot iterate over non-set domain");
    errors->push_back(err);
  }
}

void TypeChecker::visit(RangeDomain::Ptr domain) {
  const TypePtr lowerType = inferType(domain->lower);
  const TypePtr upperType = inferType(domain->upper);

  if (lowerType && (lowerType->size() != 1 || !isScalar((*lowerType)[0]) ||  
      !(*lowerType)[0].toTensor()->componentType.isInt())) {
    std::stringstream errMsg;
    errMsg << "Lower bound of for-loop range must be integral";
    if (lowerType->size() > 0) {
      errMsg << " (actual type: " << typeString(lowerType) << ")";
    }
    const auto err = ParseError(domain->lower->lineNum, domain->lower->colNum, 
                                domain->lower->lineNum, domain->lower->colNum, 
                                errMsg.str());
    errors->push_back(err);
  }
  if (upperType && (upperType->size() != 1 || !isScalar((*upperType)[0]) ||  
      !(*upperType)[0].toTensor()->componentType.isInt())) {
    std::stringstream errMsg;
    errMsg << "Upper bound of for-loop range must be integral";
    if (upperType->size() > 0) {
      errMsg << " (actual type: " << typeString(upperType) << ")";
    }
    const auto err = ParseError(domain->upper->lineNum, domain->upper->colNum, 
                                domain->upper->lineNum, domain->upper->colNum, 
                                errMsg.str());
    errors->push_back(err);
  }
}

void TypeChecker::visit(ForStmt::Ptr stmt) {
  ctx.scope();

  stmt->domain->accept(this);
  
  const ir::Var loopVar = ir::Var(stmt->loopVarName, ir::Int);
  ctx.addSymbol(stmt->loopVarName, loopVar, internal::Symbol::ReadWrite);
  
  stmt->body->accept(this);

  ctx.unscope();
}

void TypeChecker::visit(PrintStmt::Ptr stmt) {
  const TypePtr exprType = inferType(stmt->expr);

  if (exprType && (exprType->size() != 1 || !(*exprType)[0].isTensor())) {
    std::stringstream errMsg;
    errMsg << "Cannot print a non-tensor expression";
    if (exprType->size() > 0) {
      errMsg << " (actual type: " << typeString(exprType) << ")";
    }
    const auto err = ParseError(stmt->expr->lineNum, stmt->expr->colNum, 
                                stmt->expr->lineNum, stmt->expr->colNum, 
                                errMsg.str());
    errors->push_back(err);
  }
}

void TypeChecker::visit(ExprStmt::Ptr stmt) {
  stmt->expr->accept(this);
}

void TypeChecker::visit(AssignStmt::Ptr stmt) {
  const TypePtr exprType = inferType(stmt->expr);

  Type lhsType;
  for (auto lhs : stmt->lhs) {
    const TypePtr ltype = inferType(lhs);
    if (ltype && ltype->size() == 1) {
      lhsType.push_back(ltype->at(0));
    } else {
      lhsType.push_back(ir::Type());
    }
  }

  if (!exprType) {
    return;
  }

  if (stmt->lhs.size() != exprType->size()) {
    std::stringstream errMsg;
    errMsg << "cannot assign an expression returning " << exprType->size()
           << " values to " << stmt->lhs.size() << " targets";
    reportError(errMsg.str(), stmt);
    return;
  }

  for (unsigned i = 0; i < stmt->lhs.size(); ++i) {
    if (lhsType[i].defined() && !compareTypes(lhsType[i], exprType->at(i))) {
      // Allow initialization of tensors with scalars.
      // TODO: Check that scalar is zero?
      if (!lhsType[i].isTensor() || !isScalar(exprType->at(i)) || 
          lhsType[i].toTensor()->componentType != 
          exprType->at(i).toTensor()->componentType) {
        std::stringstream errMsg;
        errMsg << "cannot assign a value of type \'" 
               << typeString(exprType->at(i)) << "\' to a target of type \'" 
               << typeString(lhsType[i]) << "\'";
        reportError(errMsg.str(), stmt->lhs[i]);
      }
    }
    
    if (isa<VarExpr>(stmt->lhs[i])) {
      const std::string varName = to<VarExpr>(stmt->lhs[i])->ident;
      if (!ctx.hasSymbol(varName)) {
        ctx.addSymbol(ir::Var(varName, exprType->at(i)));
      }
    }
  }
}

void TypeChecker::visit(ExprParam::Ptr param) {
  const TypePtr exprType = inferType(param->expr);

  if (!exprType) {
    return;
  }

  if (exprType->size() != 1 || !isScalar((*exprType)[0]) ||  
      !(*exprType)[0].toTensor()->componentType.isInt()) {
    std::stringstream errMsg;
    errMsg << "Tensor index must be integral";
    if (exprType->size() > 0) {
      errMsg << " (actual type: " << typeString(exprType) << ")";
    }
    const auto err = ParseError(param->expr->lineNum, param->expr->colNum, 
                                param->expr->lineNum, param->expr->colNum, 
                                errMsg.str());
    errors->push_back(err);
    return;
  }

  retType = exprType;
}

void TypeChecker::visit(MapExpr::Ptr expr) {
  Type actualsType;
  bool typeChecked = true;
  for (auto param : expr->partialActuals) {
    const TypePtr paramType = inferType(param);

    if (!paramType) {
      typeChecked = false;
      continue;
    }

    if (paramType->size() == 1) {
      actualsType.push_back(paramType->at(0));
    } else {
      std::stringstream errMsg;
      if (paramType->size() == 0) {
        errMsg << "must pass a value as argument";
      } else {
        errMsg << "cannot pass multiple values of types " 
               << typeString(paramType) << " as a single argument";
      }
      reportError(errMsg.str(), param);
      typeChecked = false;
    }
  }

  iassert(ctx.containsFunction(expr->funcName));
  const ir::Func func = ctx.getFunction(expr->funcName);
  
  const ir::Expr target = ctx.getSymbol(expr->targetName).getExpr();
  if (target.type().isSet()) {
    const ir::SetType *targetSet = target.type().toSet();
    actualsType.push_back(targetSet->elementType);
    if (targetSet->endpointSets.size() > 0) {
    }
    // TODO: Check arguments.
  } else {
    const auto err = ParseError(expr->targetNameLineNum, expr->targetNameColNum, 
                                expr->targetNameLineNum, expr->targetNameColNum, 
                                "map can only be applied to sets");
    errors->push_back(err);
  }

  retType = std::make_shared<Type>();
  for (const auto &res : func.getResults()) {
    retType->push_back(res.getType());
  }
}

void TypeChecker::visit(OrExpr::Ptr expr) {
  typeCheckBinaryBoolean(expr);
}

void TypeChecker::visit(AndExpr::Ptr expr) {
  typeCheckBinaryBoolean(expr);
}

void TypeChecker::visit(XorExpr::Ptr expr) {
  typeCheckBinaryBoolean(expr);
}

void TypeChecker::visit(EqExpr::Ptr expr) {
  TypePtr repType;
  for (const auto operand : expr->operands) {
    const TypePtr opndType = inferType(operand);
    
    if (!opndType) {
      continue;
    }
    
    if (opndType->size() != 1 || !isScalar(opndType->at(0))) {
      std::stringstream errMsg;
      errMsg << "comparison operations can only be performed on scalar "
             << "values, not values of type " << typeString(opndType);
      reportError(errMsg.str(), operand);
    } else {
      if (!repType) {
        repType = opndType;
      } else if (!compareTypes(repType->at(0), opndType->at(0))) {
        std::stringstream errMsg;
        errMsg << "value of type " << typeString(opndType)
               << " cannot be compared to value of type " << typeString(repType);
        reportError(errMsg.str(), operand);
      }
    }
  }
  
  retType = std::make_shared<Type>();
  const auto componentType = ir::ScalarType(ir::ScalarType::Boolean);
  retType->push_back(ir::Type(ir::TensorType::make(componentType)));
}

void TypeChecker::visit(NotExpr::Ptr expr) {
  const TypePtr opndType = inferType(expr->operand);

  if (opndType && (opndType->size() != 1 || !isScalar((*opndType)[0]) || 
      !(*opndType)[0].toTensor()->componentType.isBoolean())) {
    std::stringstream errMsg;
    errMsg << "expected a boolean operand but got an operand of type "
           << typeString(opndType);
    reportError(errMsg.str(), expr->operand);
  }

  retType = std::make_shared<Type>();
  const auto componentType = ir::ScalarType(ir::ScalarType::Boolean);
  retType->push_back(ir::Type(ir::TensorType::make(componentType)));
}

void TypeChecker::visit(AddExpr::Ptr expr) {
  typeCheckBinaryElwise(expr); 
}

void TypeChecker::visit(SubExpr::Ptr expr) {
  typeCheckBinaryElwise(expr); 
}

void TypeChecker::visit(MulExpr::Ptr expr) {
  const TypePtr lhsType = inferType(expr->lhs);
  const TypePtr rhsType = inferType(expr->rhs);

  bool typeChecked = (bool)lhsType && (bool)rhsType;
  if (lhsType && (lhsType->size() != 1 || !(*lhsType)[0].isTensor())) {
    std::stringstream errMsg;
    errMsg << "Left operand of element-wise operation must be a tensor";
    if (lhsType->size() > 0) {
      errMsg << " (actual type: " << typeString(lhsType) << ")";
    }
    const auto err = ParseError(expr->lhs->lineNum, expr->lhs->colNum, 
                                expr->lhs->lineNum, expr->lhs->colNum,
                                errMsg.str());
    errors->push_back(err);
    typeChecked = false;
  }
  if (rhsType && (rhsType->size() != 1 || !(*rhsType)[0].isTensor())) {
    std::stringstream errMsg;
    errMsg << "Right operand of element-wise operation must be a tensor";
    if (rhsType->size() > 0) {
      errMsg << " (actual type: " << typeString(rhsType) << ")";
    }
    const auto err = ParseError(expr->rhs->lineNum, expr->rhs->colNum, 
                                expr->rhs->lineNum, expr->rhs->colNum,
                                errMsg.str());
    errors->push_back(err);
    typeChecked = false;
  }
  
  if (!typeChecked) return;

  const ir::TensorType *ltype = (*lhsType)[0].toTensor();
  const ir::TensorType *rtype = (*rhsType)[0].toTensor();
  const std::vector<ir::IndexDomain> ldimensions = ltype->getDimensions();
  const std::vector<ir::IndexDomain> rdimensions = rtype->getDimensions();
  if (ltype->order() == 0 || rtype->order() == 0) {
    // Scale
    if (ltype->componentType != rtype->componentType) {
      std::stringstream errMsg;
      errMsg << "Cannot multiply tensors of type " << typeString(lhsType) 
             << " and type " << typeString(rhsType);
      reportError(errMsg.str(), expr);
    } else {
      const auto tensorType = (ltype->order() > 0) ? lhsType->at(0) : 
                              rhsType->at(0);
      retType = std::make_shared<Type>();
      retType->push_back(tensorType);
    }
  } else if (ltype->order() == 1 && rtype->order() == 1) {
    // Vector-Vector Multiplication (inner and outer product)
    if (ltype->isColumnVector && rtype->isColumnVector) {
      reportError("Cannot multiply two column vectors", expr);
    } else if (!ltype->isColumnVector && !rtype->isColumnVector) {
      reportError("Cannot multiply two row vectors", expr);
    } else if (!compareTypes((*lhsType)[0], (*rhsType)[0])) {
      std::stringstream errMsg;
      errMsg << "Cannot multiply vectors of type " << typeString(lhsType) 
             << " and type " << typeString(rhsType);
      reportError(errMsg.str(), expr);
    } else {
      std::vector<ir::IndexDomain> dom;
      if (ltype->isColumnVector) {
        dom.push_back(ldimensions[0]);
        dom.push_back(rdimensions[0]);
      }
      const auto tensorType = ir::TensorType::make(ltype->componentType, dom);
      retType = std::make_shared<Type>();
      retType->push_back(ir::Type(tensorType));  
    }
  } else if (ltype->order() == 2 && rtype->order() == 1) {
    // Matrix-Vector
    if (ldimensions[1] != rdimensions[0] || 
        ltype->componentType != rtype->componentType) {
      std::stringstream errMsg;
      errMsg << "Cannot multiply a matrix of type " << typeString(lhsType)
             << " by a vector of type " << typeString(rhsType);
      reportError(errMsg.str(), expr);
    //} else if (!rtype->isColumnVector) {
    //  reportError("Cannot multiply a matrix by a row vector", expr);
    } else {
      const auto tensorType = ir::TensorType::make(ltype->componentType, 
                                                   {ldimensions[0]}, true);
      retType = std::make_shared<Type>();
      retType->push_back(ir::Type(tensorType));
    }
  } else if (ltype->order() == 1 && rtype->order() == 2) {
    // Vector-Matrix
    if (ldimensions[0] != rdimensions[0] || 
        ltype->componentType != rtype->componentType) {
      std::stringstream errMsg;
      errMsg << "Cannot multiply a vector of type " << typeString(lhsType)
             << " by a matrix of type " << typeString(rhsType);
      reportError(errMsg.str(), expr);
    //} else if (ltype->isColumnVector) {
    //  reportError("Cannot multiply a column vector by a matrix", expr);
    } else {
      const auto tensorType = ir::TensorType::make(ltype->componentType, 
                                                   {rdimensions[1]});
      retType = std::make_shared<Type>();
      retType->push_back(ir::Type(tensorType));
    }
  } else if (ltype->order() == 2 && rtype->order() == 2) {
    // Matrix-Matrix
    if (ldimensions[1] != rdimensions[0] || 
        ltype->componentType != rtype->componentType) {
      std::stringstream errMsg;
      errMsg << "Cannot multiply a matrix of type " << typeString(lhsType)
             << " by a matrix of type " << typeString(rhsType);
      reportError(errMsg.str(), expr);
    } else {
      const std::vector<ir::IndexDomain> dom = {ldimensions[0], rdimensions[1]};
      const auto tensorType = ir::TensorType::make(ltype->componentType, dom);
      retType = std::make_shared<Type>();
      retType->push_back(ir::Type(tensorType));
    }
  } else {
    reportError("Cannot multiply tensors of order 3 or greater using *", expr);
  }
}

void TypeChecker::visit(DivExpr::Ptr expr) {
  const TypePtr lhsType = inferType(expr->lhs);
  const TypePtr rhsType = inferType(expr->rhs);

  bool typeChecked = (bool)lhsType && (bool)rhsType;
  if (lhsType && (lhsType->size() != 1 || !(*lhsType)[0].isTensor())) {
    std::stringstream errMsg;
    errMsg << "Left operand of element-wise operation must be a tensor";
    if (lhsType->size() > 0) {
      errMsg << " (actual type: " << typeString(lhsType) << ")";
    }
    const auto err = ParseError(expr->lhs->lineNum, expr->lhs->colNum, 
                                expr->lhs->lineNum, expr->lhs->colNum,
                                errMsg.str());
    errors->push_back(err);
    typeChecked = false;
  }
  if (rhsType && (rhsType->size() != 1 || !(*rhsType)[0].isTensor())) {
    std::stringstream errMsg;
    errMsg << "Right operand of element-wise operation must be a tensor";
    if (rhsType->size() > 0) {
      errMsg << " (actual type: " << typeString(rhsType) << ")";
    }
    const auto err = ParseError(expr->rhs->lineNum, expr->rhs->colNum, 
                                expr->rhs->lineNum, expr->rhs->colNum,
                                errMsg.str());
    errors->push_back(err);
    typeChecked = false;
  }
 
  if (!typeChecked) return;

  const unsigned lhsOrder = (*lhsType)[0].toTensor()->order();
  const unsigned rhsOrder = (*rhsType)[0].toTensor()->order();
  if (lhsOrder > 0 && rhsOrder > 0) {
    std::stringstream errMsg;
    errMsg << "Division of two non-scalar tensors is not supported (operands "
           << "are of type " << typeString(lhsType) << " and type " 
           << typeString(rhsType) << ")";
    const auto err = ParseError(expr->lineNum, expr->colNum, expr->lineNum,
                                expr->colNum, errMsg.str());
    errors->push_back(err);
    return;
  }

  retType = (lhsOrder > 0) ? lhsType : rhsType;
}

void TypeChecker::visit(ElwiseMulExpr::Ptr expr) {
  typeCheckBinaryElwise(expr); 
}

void TypeChecker::visit(ElwiseDivExpr::Ptr expr) {
  typeCheckBinaryElwise(expr); 
}

void TypeChecker::visit(NegExpr::Ptr expr) {
  const TypePtr opndType = inferType(expr->operand);

  if (!opndType) return;

  if (opndType->size() != 1 || !(*opndType)[0].isTensor()) {
    std::stringstream errMsg;
    errMsg << "Operand of tensor negation must be a tensor";
    if (opndType->size() > 0) {
      errMsg << " (actual type: " << typeString(opndType) << ")";
    }
    const auto err = ParseError(expr->operand->lineNum, expr->operand->colNum, 
                                expr->operand->lineNum, expr->operand->colNum,
                                errMsg.str());
    errors->push_back(err);
    return;
  }

  retType = opndType;
}

void TypeChecker::visit(ExpExpr::Ptr expr) {
  // TODO: implement
  iassert(false);
}

void TypeChecker::visit(TransposeExpr::Ptr expr) {
  const TypePtr opndType = inferType(expr->operand);

  if (!opndType) return;

  if (opndType->size() != 1 || !(*opndType)[0].isTensor() || 
      (*opndType)[0].toTensor()->order() > 2) {
    std::stringstream errMsg;
    errMsg << "Operand of tensor transpose must be a tensor of order 2 or less";
    if (opndType->size() > 0) {
      errMsg << " (actual type: " << typeString(opndType) << ")";
    }
    reportError(errMsg.str(), expr->operand);
    return;
  }

  const ir::TensorType *tensorType = (*opndType)[0].toTensor();
  const std::vector<ir::IndexDomain> dimensions = tensorType->getDimensions();
  switch (tensorType->order()) {
    case 0:
      retType = opndType;
      break;
    case 1:
    {
      const auto exprType = ir::TensorType::make(tensorType->componentType, 
          dimensions, !tensorType->isColumnVector);
      retType = std::make_shared<Type>();
      retType->push_back(ir::Type(exprType));
      break;
    }
    case 2:
    {
      const auto exprType = ir::TensorType::make(
          tensorType->componentType, {dimensions[1], dimensions[0]});
      retType = std::make_shared<Type>();
      retType->push_back(ir::Type(exprType));
      break;
    }
    default:
      iassert(false);
      break;
  }
}

void TypeChecker::visit(CallExpr::Ptr expr) {
  iassert(ctx.containsFunction(expr->funcName));
  const ir::Func func = ctx.getFunction(expr->funcName);

  if (expr->operands.size() != func.getArguments().size()) {
    if (func.getKind() != ir::Func::Intrinsic) { 
      std::stringstream errMsg;
      errMsg << "passed in " << expr->operands.size() << " arguments but "
             << "function expects " << func.getArguments().size();
      reportError(errMsg.str(), expr);
    } // TODO: Special handling for intrinsics?
  } else {
    for (unsigned i = 0; i < expr->operands.size(); ++i) {
      const Expr::Ptr operand = expr->operands[i];
      const TypePtr argType = inferType(operand);
  
      if (!argType) {
        continue;
      }
  
      if (argType->size() != 1) {
        std::stringstream errMsg;
        errMsg << argType->size() << " values passed in as one argument";
        reportError(errMsg.str(), operand);
        continue;
      }

      if (!compareTypes(argType->at(0), func.getArguments()[i].getType())) {
        std::stringstream errMsg;
        errMsg << "expected argument of type \'" 
               << typeString(func.getArguments()[i].getType()) 
               << "\' but got an argument of type " << typeString(argType);
        reportError(errMsg.str(), operand);
      }
    }
  }

  retType = std::make_shared<Type>();
  for (const auto &res : func.getResults()) {
    retType->push_back(res.getType());
  }
}

void TypeChecker::visit(TensorReadExpr::Ptr expr) {
  const TypePtr lhsType = inferType(expr->tensor);

  if (!lhsType) return;

  if (lhsType->size() != 1) {
    const auto msg = "can only access elements of a single tensor or tuple";
    reportError(msg, expr->tensor);
    return;
  }

  if (lhsType->at(0).isTensor()) {
    const ir::TensorType *tensorType = lhsType->at(0).toTensor();
   
    if (tensorType->getDimensions().size() != expr->indices.size()) {
      std::stringstream errMsg;
      errMsg << "tensor access expected " << tensorType->getDimensions().size()
             << " indices but got " << expr->indices.size();
      reportError(errMsg.str(), expr);
      return;
    }

    std::vector<ir::IndexDomain> dims;
    for (unsigned i = 0; i < expr->indices.size(); ++i) {
      const ReadParam::Ptr param = expr->indices[i];

      if (param->isSlice()) {
        dims.push_back(tensorType->getDimensions()[i]);
      } else {
        const Expr::Ptr paramExpr = to<ExprParam>(param)->expr;
        const TypePtr paramType = inferType(paramExpr);

        if (!paramType) {
          continue;
        }

        // TODO: Check arguments.
        /*if (paramType->size() != 1 || !isScalar(paramType->at(0)) ||
            !paramType->at(0).toTensor()->componentType.isInt()) {
          std::stringstream errMsg;
          errMsg << "expected an integral index but got an index of type " 
                 << typeString(paramType);
          reportError(errMsg.str(), param);
        }*/
      }
    }

    retType = std::make_shared<Type>();
    if (dims.empty()) {
      retType->push_back(tensorType->getBlockType());
    } else {
      retType->push_back(ir::TensorType::make(tensorType->componentType, dims));
    }
  } else if (lhsType->at(0).isTuple()) {
    if (expr->indices.size() != 1) {
      std::stringstream errMsg;
      errMsg << "tuple access expects exactly one index but got " 
             << expr->indices.size();
      reportError(errMsg.str(), expr);
      return;
    }

    if (expr->indices[0]->isSlice()) {
      reportError("tuple access requires an integral index", expr->indices[0]);
      return;
    }

    const Expr::Ptr indexExpr = to<ExprParam>(expr->indices[0])->expr;
    const TypePtr indexType = inferType(indexExpr);
    if (indexType->size() != 1 || !isScalar(indexType->at(0)) ||
        !indexType->at(0).toTensor()->componentType.isInt()) {
      std::stringstream errMsg;
      errMsg << "expected an integral index but got an index of type " 
             << typeString(indexType);
      reportError(errMsg.str(), expr->indices[0]);
    }

    retType = std::make_shared<Type>();
    retType->push_back(lhsType->at(0).toTuple()->elementType);
  } else {
    std::stringstream errMsg;
    errMsg << "cannot access elements from objects of type " 
           << typeString(lhsType);
    reportError(errMsg.str(), expr->tensor);
  }
}

void TypeChecker::visit(FieldReadExpr::Ptr expr) {
  const TypePtr lhsType = inferType(expr->setOrElem);

  if (!lhsType) return;

  if (lhsType->size() != 1) {
    const auto msg = "can only access fields of a single set or element";
    reportError(msg, expr->setOrElem);
    return;
  }

  const ir::Type type = (*lhsType)[0];

  const ir::ElementType *elemType = nullptr;
  if (type.isElement()) {
    elemType = type.toElement();
  } else if (type.isSet()) {
    elemType = type.toSet()->elementType.toElement();
  }
  
  if (elemType == nullptr) {
    const auto msg = "field accesses are only valid for sets and elements";
    reportError(msg, expr->setOrElem);
    return;
  }

  if (!elemType->hasField(expr->fieldName)) {
    std::stringstream errMsg;
    errMsg << "undefined field \'" << expr->fieldName << "\'";
    reportError(errMsg.str(), expr);
    return;
  }

  retType = std::make_shared<Type>();
  if (type.isElement()) {
    retType->push_back(elemType->field(expr->fieldName).type);
  } else {
    const std::string varName = to<VarExpr>(expr->setOrElem)->ident;
    const ir::Expr setExpr = ctx.getSymbol(varName).getExpr();
    retType->push_back(getFieldType(setExpr, expr->fieldName));
  }
}

void TypeChecker::visit(VarExpr::Ptr expr) {
  if (!ctx.hasSymbol(expr->ident)) return;

  retType = std::make_shared<Type>();
  retType->push_back(ctx.getSymbol(expr->ident).getExpr().type());
}

void TypeChecker::visit(IntLiteral::Ptr lit) {
  retType = std::make_shared<Type>();
  const auto componentType = ir::ScalarType(ir::ScalarType::Int);
  retType->push_back(ir::Type(ir::TensorType::make(componentType)));
}

void TypeChecker::visit(FloatLiteral::Ptr lit) {
  retType = std::make_shared<Type>();
  const auto componentType = ir::ScalarType(ir::ScalarType::Float);
  retType->push_back(ir::Type(ir::TensorType::make(componentType)));
}

void TypeChecker::visit(BoolLiteral::Ptr lit) {
  retType = std::make_shared<Type>();
  const auto componentType = ir::ScalarType(ir::ScalarType::Boolean);
  retType->push_back(ir::Type(ir::TensorType::make(componentType)));
}

void TypeChecker::visit(DenseIntVector::Ptr lit) {
  retTensorVals.addIntValues(lit->vals.size()); 
}

void TypeChecker::visit(DenseFloatVector::Ptr lit) {
  retTensorVals.addFloatValues(lit->vals.size()); 
}

void TypeChecker::visit(DenseNDTensor::Ptr lit) {
  iassert(lit->elems.size() > 1);
  
  TensorValues tensorVals = getTensorVals(lit->elems[0]);
  tensorVals.addDimension();

  for (unsigned i = 1; i < lit->elems.size(); ++i) {
    const TensorValues right = getTensorVals(lit->elems[i]);
    tensorVals.merge(right);
  }

  retTensorVals = tensorVals;
}

void TypeChecker::visit(DenseTensorLiteral::Ptr lit) {
  try {
    TensorValues tensorVals = getTensorVals(lit->tensor);
    const std::vector<ir::IndexDomain> idoms(tensorVals.dimSizes.rbegin(), 
                                             tensorVals.dimSizes.rend());
    const auto elemType = (tensorVals.type == TensorValues::Type::INT) ?
                          ir::ScalarType::Int : ir::ScalarType::Float;
    iassert(idoms.size() == 1 || !lit->transposed);

    retType = std::make_shared<Type>();
    retType->push_back(ir::TensorType::make(elemType, idoms, lit->transposed));
  } catch (std::exception &err) {
    reportError(std::string(err.what()), lit);
  }
}

// TODO: Remove or move to separate semantic analysis pass.
/*void TypeChecker::visit(Test::Ptr test) {
  const auto msg = "non-literal values for tests are currently unsupported";
  
  for (auto arg : test->args) {
    arg->accept(this);
    if (!isa<TensorLiteral>(arg)) {
      reportError(msg, arg);
    }
  }

  test->expected->accept(this);
  if (!isa<TensorLiteral>(test->expected)) {
    reportError(msg, test->expected);
  }
}*/

void TypeChecker::typeCheckVarOrConstDecl(VarDecl::Ptr decl, 
                                          const bool isConst) {
  const ir::Var var = getVar(decl->var);
  iassert(!ctx.hasSymbol(var.getName()));
  ctx.addSymbol(var);

  if (!decl->initVal) {
    return;
  }

  const ir::Type varType = var.getType();
  const TypePtr initValType = inferType(decl->initVal);
  if (!initValType || (initValType->size() == 1 && 
      compareTypes(varType, initValType->at(0)))) {
    // Initial value type matches declared variable/constant type.
    return;
  }

  std::stringstream errMsg;
  errMsg << "attempting to initialize a variable or constant of type \'"
         << typeString(var.getType()) << "\' with an expression of type "
         << typeString(initValType);

  iassert(varType.isTensor());
  if (initValType->size() != 1 || !initValType->at(0).isTensor()) {
    reportError(errMsg.str(), decl);
    return;
  }

  // Check if attempting to initialize a tensor with a scalar.
  const ir::Type initIRType = initValType->at(0);
  const auto varTensorType = varType.toTensor();
  const auto initTensorType = initIRType.toTensor();
  if (isScalar(initIRType) && 
      varTensorType->componentType == initTensorType->componentType) {
    // TODO: Verify that initial value is zero?
    return;
  }

  // Check if initial value type is equivalent to declared constant type.
  const ir::Type varBlockType = varTensorType->getBlockType();
  const ir::Type initBlockType = initTensorType->getBlockType();
  if (isConst && compareTypes(varBlockType, initBlockType)) {
    const auto varDims = varTensorType->getOuterDimensions();
    const auto initDims = initTensorType->getOuterDimensions();

    // Search for first "non-trivial" dimensions of both types.
    std::vector<ir::IndexSet>::const_iterator varDimsIt = varDims.begin();
    for (; varDimsIt != varDims.end(); ++varDimsIt) {
      if (*varDimsIt != ir::IndexSet(1)) {
        break;
      }
    }
    std::vector<ir::IndexSet>::const_iterator initDimsIt = initDims.begin();
    for (; initDimsIt != initDims.end(); ++initDimsIt) {
      if (*initDimsIt != ir::IndexSet(1)) {
        break;
      }
    }
    
    if (std::equal(varDimsIt, varDims.end(), initDimsIt)) {
      return;
    }
  }

  reportError(errMsg.str(), decl);
}

void TypeChecker::typeCheckBinaryElwise(BinaryExpr::Ptr expr) {
  const TypePtr lhsType = inferType(expr->lhs);
  const TypePtr rhsType = inferType(expr->rhs);

  bool typeChecked = (bool)lhsType && (bool)rhsType;
  if (lhsType && (lhsType->size() != 1 || !(*lhsType)[0].isTensor())) {
    std::stringstream errMsg;
    errMsg << "Left operand of element-wise operation must be a tensor";
    if (lhsType->size() > 0) {
      errMsg << " (actual type: " << typeString(lhsType) << ")";
    }
    const auto err = ParseError(expr->lhs->lineNum, expr->lhs->colNum, 
                                expr->lhs->lineNum, expr->lhs->colNum,
                                errMsg.str());
    errors->push_back(err);
    typeChecked = false;
  }
  if (rhsType && (rhsType->size() != 1 || !(*rhsType)[0].isTensor())) {
    std::stringstream errMsg;
    errMsg << "Right operand of element-wise operation must be a tensor";
    if (rhsType->size() > 0) {
      errMsg << " (actual type: " << typeString(rhsType) << ")";
    }
    const auto err = ParseError(expr->rhs->lineNum, expr->rhs->colNum, 
                                expr->rhs->lineNum, expr->rhs->colNum,
                                errMsg.str());
    errors->push_back(err);
    typeChecked = false;
  }
 
  if (!typeChecked) return;

  const ir::TensorType *ltype = (*lhsType)[0].toTensor();
  const ir::TensorType *rtype = (*rhsType)[0].toTensor();
  const unsigned lhsOrder = ltype->order();
  const unsigned rhsOrder = rtype->order();
  const bool hasScalarOperand = (lhsOrder == 0 || rhsOrder == 0);
  if ((hasScalarOperand && (ltype->componentType != rtype->componentType)) ||
      (!hasScalarOperand && !compareTypes((*lhsType)[0], (*rhsType)[0]))) {
    std::stringstream errMsg;
    errMsg << "Cannot perform element-wise operation on tensors of type "
           << typeString(lhsType) << " and type " << typeString(rhsType);
    const auto err = ParseError(expr->lineNum, expr->colNum, expr->lineNum,
                                expr->colNum, errMsg.str());
    errors->push_back(err);
    return;
  }

  retType = (lhsOrder > 0) ? lhsType : rhsType;
}

void TypeChecker::typeCheckBinaryBoolean(BinaryExpr::Ptr expr) {
  const TypePtr lhsType = inferType(expr->lhs);
  const TypePtr rhsType = inferType(expr->rhs);

  if (lhsType && (lhsType->size() != 1 || !isScalar((*lhsType)[0]) || 
      !(*lhsType)[0].toTensor()->componentType.isBoolean())) {
    std::stringstream errMsg;
    errMsg << "Left operand of boolean operation must be boolean";
    if (lhsType->size() > 0) {
      errMsg << " (actual type: " << typeString(lhsType) << ")";
    }
    const auto err = ParseError(expr->lhs->lineNum, expr->lhs->colNum, 
                                expr->lhs->lineNum, expr->lhs->colNum,
                                errMsg.str());
    errors->push_back(err);
  }
  if (rhsType && (rhsType->size() != 1 || !isScalar((*rhsType)[0]) || 
      !(*rhsType)[0].toTensor()->componentType.isBoolean())) {
    std::stringstream errMsg;
    errMsg << "Right operand of boolean operation must be boolean";
    if (rhsType->size() > 0) {
      errMsg << " (actual type: " << typeString(rhsType) << ")";
    }
    const auto err = ParseError(expr->rhs->lineNum, expr->rhs->colNum, 
                                expr->rhs->lineNum, expr->rhs->colNum,
                                errMsg.str());
    errors->push_back(err);
  }

  retType = std::make_shared<Type>();
  const auto componentType = ir::ScalarType(ir::ScalarType::Boolean);
  retType->push_back(ir::Type(ir::TensorType::make(componentType)));
}

}
}
