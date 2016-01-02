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

namespace simit {
namespace hir {

void TypeChecker::visit(RangeIndexSet::Ptr set) {
  retIndexSet = std::make_shared<ir::IndexSet>(set->range);
}

void TypeChecker::visit(SetIndexSet::Ptr set) {
  if (!ctx.hasSymbol(set->setName)) {
    reportUndeclared("set", set->setName, set);
    return;
  }

  ir::Expr setExpr = ctx.getSymbol(set->setName).getExpr();
  
  if (!setExpr.type().isSet()) {
    reportError("index set must be a set, a range, or dynamic (*)", set);
  } else {
    retIndexSet = std::make_shared<ir::IndexSet>(setExpr);
  }
}

void TypeChecker::visit(DynamicIndexSet::Ptr set) {
  retIndexSet = std::make_shared<ir::IndexSet>();
}

void TypeChecker::visit(ElementType::Ptr type) {
  if (!ctx.containsElementType(type->ident)) {
    reportUndeclared("element type", type->ident, type);
  } else {
    retIRType = ctx.getElementType(type->ident);
  }
}

void TypeChecker::visit(Endpoint::Ptr end) {
  if (!ctx.hasSymbol(end->setName)) {
    reportUndeclared("set", end->setName, end);
  } else {
    retExpr = ctx.getSymbol(end->setName).getExpr();
  }
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

  if (type->length->val < 1) {
    const auto msg = "tuple must have length greater than or equal to one";
    reportError(msg, type->length);
  } else if (elementType.defined()) {
    retIRType = ir::Type(ir::TupleType::make(elementType, type->length->val));
  }
}

void TypeChecker::visit(ScalarType::Ptr type) {
  ir::ScalarType componentType;
  switch (type->type) {
    case ScalarType::Type::INT:
      componentType = ir::ScalarType(ir::ScalarType::Int);
      break;
    case ScalarType::Type::FLOAT:
      componentType = ir::ScalarType(ir::ScalarType::Float);
      break;
    case ScalarType::Type::BOOL:
      componentType = ir::ScalarType(ir::ScalarType::Boolean);
      break;
    default:
      unreachable;
      break;
  }
  
  retIRType = ir::Type(ir::TensorType::make(componentType));
}

void TypeChecker::visit(NDTensorType::Ptr type) {
  const ir::Type blockType = getIRType(type->blockType);
  bool typeChecked = blockType.defined();

  std::vector<ir::IndexSet> indexSets;
  for (auto is : type->indexSets) {
    const Ptr<ir::IndexSet> indexSet = getIndexSet(is);
    if (indexSet) {
      indexSets.push_back(*indexSet);
    } else {
      typeChecked = false;
    }
  }

  if (!typeChecked) {
    return;
  }

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
    } else if (blockTensorType->order() == indexSets.size()) {
      for (size_t i = 0; i < indexSets.size(); ++i) {
        std::vector<ir::IndexSet> dimension;
        dimension.push_back(indexSets[i]);
        dimension.insert(dimension.end(),
                         blockDimensions[i].getIndexSets().begin(),
                         blockDimensions[i].getIndexSets().end());
        dimensions.push_back(ir::IndexDomain(dimension));
      }
    } else {
      const auto msg = "blocked tensors with non-scalar blocks must have "
                       "same number of dimensions as its blocks";
      reportError(msg, type);
      return;
    }
  
    retIRType = ir::TensorType::make(componentType, dimensions);
  }

  if (type->transposed) {
    const auto tensorType = retIRType.toTensor();
    const auto dimensions = tensorType->getDimensions();
    const auto componentType = tensorType->componentType;
    retIRType = ir::TensorType::make(componentType, dimensions, true);
  }
}

void TypeChecker::visit(IdentDecl::Ptr decl) {
  const ir::Type type = getIRType(decl->type);
  retVar = ir::Var(decl->name->ident, type);
  retField = ir::Field(decl->name->ident, type);
}

void TypeChecker::visit(Field::Ptr field) {
  // Let visit method for IdentDecl node take care of field creation.
  HIRVisitor::visit(field);

  iassert(retField.name != "");
  iassert(retField.type.isTensor());
}

void TypeChecker::visit(ElementTypeDecl::Ptr decl) {
  std::vector<ir::Field> fields;
  for (auto f : decl->fields) {
    const ir::Field field = getField(f);
    fields.push_back(field);
  }

  const std::string name = decl->name->ident;
  if (ctx.containsElementType(name)) {
    reportMultipleDefs("element type", name, decl);
  } else {
    ctx.addElementType(ir::ElementType::make(name, fields));
  }
}

void TypeChecker::visit(ExternDecl::Ptr decl) {
  const ir::Var externVar = getVar(decl->var);

  if (ctx.hasSymbol(externVar.getName())) {
    reportMultipleDefs("variable or constant", externVar.getName(), decl);
  } else {
    ctx.addSymbol(externVar);
  }
}

void TypeChecker::visit(FuncDecl::Ptr decl) {
  ctx.scope();

  std::vector<ir::Var> arguments;
  for (auto arg : decl->args) {
    const ir::Var argVar = getVar(arg);
    const auto access = arg->inout ? internal::Symbol::ReadWrite : 
                        internal::Symbol::Read;
    ctx.addSymbol(argVar.getName(), argVar, access);
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

  const std::string name = decl->name->ident;
  if (ctx.containsFunction(name)) {
    reportMultipleDefs("function or procedure", name, decl);
  } else {
    ctx.addFunction(ir::Func(name, arguments, results, ir::Stmt()));
  }
}

void TypeChecker::visit(VarDecl::Ptr decl) {
  typeCheckVarOrConstDecl(decl);
}

void TypeChecker::visit(ConstDecl::Ptr decl) {
  typeCheckVarOrConstDecl(decl, true);
}

void TypeChecker::visit(WhileStmt::Ptr stmt) {
  const Ptr<Expr::Type> condType = inferType(stmt->cond);
 
  ctx.scope();
  stmt->body->accept(this);
  ctx.unscope();

  if (condType && (condType->size() != 1 || !isBoolean(condType->at(0)))) {
    std::stringstream errMsg;
    errMsg << "expected a boolean conditional expression but got an "
           << "expression of type " << typeString(condType);
    reportError(errMsg.str(), stmt->cond);
  }
}

void TypeChecker::visit(IfStmt::Ptr stmt) {
  const Ptr<Expr::Type> condType = inferType(stmt->cond);
  
  ctx.scope();
  stmt->ifBody->accept(this);
  ctx.unscope();

  if (stmt->elseBody) {
    ctx.scope();
    stmt->elseBody->accept(this);
    ctx.unscope();
  }

  if (condType && (condType->size() != 1 || !isBoolean(condType->at(0)))) {
    std::stringstream errMsg;
    errMsg << "expected a boolean conditional expression but got an "
           << "expression of type " << typeString(condType);
    reportError(errMsg.str(), stmt->cond);
  }
}

void TypeChecker::visit(IndexSetDomain::Ptr domain) {
  const Ptr<ir::IndexSet> set = getIndexSet(domain->set);

  if (set && set->getKind() != ir::IndexSet::Set) {
    reportError("for-loop cannot iterate over a non-set domain", domain);
  }
}

void TypeChecker::visit(RangeDomain::Ptr domain) {
  const Ptr<Expr::Type> lowerType = inferType(domain->lower);
  const Ptr<Expr::Type> upperType = inferType(domain->upper);

  if (lowerType && (lowerType->size() != 1 || !isInt(lowerType->at(0)))) {
    std::stringstream errMsg;
    errMsg << "expected lower bound of for-loop range to be integral but got "
           << "an expression of type " << typeString(lowerType);
    reportError(errMsg.str(), domain->lower);
  }
  if (upperType && (upperType->size() != 1 || !isInt(upperType->at(0)))) {
    std::stringstream errMsg;
    errMsg << "expected upper bound of for-loop range to be integral but got "
           << "an expression of type " << typeString(upperType);
    reportError(errMsg.str(), domain->upper);
  }
}

void TypeChecker::visit(ForStmt::Ptr stmt) {
  ctx.scope();
  stmt->domain->accept(this);
  
  const ir::Var loopVar = ir::Var(stmt->loopVar->ident, ir::Int);
  ctx.addSymbol(stmt->loopVar->ident, loopVar, internal::Symbol::Read);
  
  stmt->body->accept(this);
  ctx.unscope();
}

void TypeChecker::visit(PrintStmt::Ptr stmt) {
  const Ptr<Expr::Type> exprType = inferType(stmt->expr);

  if (exprType && (exprType->size() != 1 || !exprType->at(0).isTensor())) {
    std::stringstream errMsg;
    errMsg << "cannot print an expression of type " << typeString(exprType);
    reportError(errMsg.str(), stmt->expr);
  }
}

void TypeChecker::visit(AssignStmt::Ptr stmt) {
  const Ptr<Expr::Type> exprType = inferType(stmt->expr);

  Expr::Type lhsType;
  for (auto lhs : stmt->lhs) {
    markCheckWritable(lhs);
    skipCheckDeclared = isa<VarExpr>(lhs);
    
    const Ptr<Expr::Type> ltype = inferType(lhs);
    if (ltype && ltype->size() == 1) {
      lhsType.push_back(ltype->at(0));
    } else {
      lhsType.push_back(ir::Type());
    }
    
    checkWritable.reset();
    skipCheckDeclared = false;
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
        errMsg << "cannot assign a value of type '" 
               << typeString(exprType->at(i)) << "' to a target of type '" 
               << typeString(lhsType[i]) << "'";
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
  const Ptr<Expr::Type> exprType = inferType(param->expr);

  if (!exprType) {
    return;
  }

  if (exprType->size() != 1 || !isInt(exprType->at(0))) {
    std::stringstream errMsg;
    errMsg << "expected an integral tensor index but got an index of type "
           << typeString(exprType);
    reportError(errMsg.str(), param->expr);
  } else {
    retType = exprType;
  }
}

void TypeChecker::visit(MapExpr::Ptr expr) {
  std::vector<ir::Type> actualsType;
  bool typeChecked = true;
  for (auto param : expr->partialActuals) {
    const Ptr<Expr::Type> paramType = inferType(param);

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
  
  ir::Func func;
  const std::string funcName = expr->func->ident;
  if (ctx.containsFunction(funcName)) {
    func = ctx.getFunction(funcName);
    
    retType = std::make_shared<Expr::Type>();
    for (const auto &res : func.getResults()) {
      retType->push_back(res.getType());
    }
  } else {
    reportUndeclared("function", funcName, expr->func);
  }
  
  const ir::Expr target = ctx.getSymbol(expr->target->ident).getExpr();
  if (target.type().isSet()) {
    const ir::SetType *targetSet = target.type().toSet();
    actualsType.push_back(targetSet->elementType);
    if (targetSet->endpointSets.size() > 0) {
    }
    // TODO: Check arguments.
  } else {
    reportError("map operation can only be applied to sets", expr->target);
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
  Ptr<Expr::Type> repType;
  for (const auto operand : expr->operands) {
    const Ptr<Expr::Type> opndType = inferType(operand);
    
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
        errMsg << "value of type " << typeString(opndType) << " cannot be "
               << "compared to value of type " << typeString(repType);
        reportError(errMsg.str(), operand);
      }
    }
  }
  
  retType = std::make_shared<Expr::Type>();
  const auto componentType = ir::ScalarType(ir::ScalarType::Boolean);
  retType->push_back(ir::Type(ir::TensorType::make(componentType)));
}

void TypeChecker::visit(NotExpr::Ptr expr) {
  const Ptr<Expr::Type> opndType = inferType(expr->operand);

  if (opndType && (opndType->size() != 1 || !isBoolean(opndType->at(0)))) {
    std::stringstream errMsg;
    errMsg << "expected a boolean operand but got an operand of type "
           << typeString(opndType);
    reportError(errMsg.str(), expr->operand);
  }

  retType = std::make_shared<Expr::Type>();
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
  const Ptr<Expr::Type> lhsType = inferType(expr->lhs);
  const Ptr<Expr::Type> rhsType = inferType(expr->rhs);

  bool typeChecked = (bool)lhsType && (bool)rhsType;
  if (lhsType && (lhsType->size() != 1 || !(*lhsType)[0].isTensor())) {
    std::stringstream errMsg;
    errMsg << "expected left operand of multiplication operation to be a "
           << "tensor but got an operand of type " << typeString(lhsType);
    reportError(errMsg.str(), expr->lhs);
    typeChecked = false;
  }
  if (rhsType && (rhsType->size() != 1 || !(*rhsType)[0].isTensor())) {
    std::stringstream errMsg;
    errMsg << "expected right operand of multiplication operation to be a "
           << "tensor but got an operand of type " << typeString(rhsType);
    reportError(errMsg.str(), expr->rhs);
    typeChecked = false;
  }
 
  if (!typeChecked) {
    return;
  }

  const ir::TensorType *ltype = (*lhsType)[0].toTensor();
  const ir::TensorType *rtype = (*rhsType)[0].toTensor();
  const std::vector<ir::IndexDomain> ldimensions = ltype->getDimensions();
  const std::vector<ir::IndexDomain> rdimensions = rtype->getDimensions();
  if (ltype->order() == 0 || rtype->order() == 0) {
    // Scale
    if (ltype->componentType != rtype->componentType) {
      std::stringstream errMsg;
      errMsg << "cannot multiply tensors of type " << typeString(lhsType) 
             << " and type " << typeString(rhsType);
      reportError(errMsg.str(), expr);
    } else {
      const auto tensorType = (ltype->order() > 0) ? lhsType->at(0) : 
                              rhsType->at(0);
      retType = std::make_shared<Expr::Type>();
      retType->push_back(tensorType);
    }
  } else if (ltype->order() == 1 && rtype->order() == 1) {
    // Vector-Vector Multiplication (inner and outer product)
    if (ltype->isColumnVector && rtype->isColumnVector) {
      reportError("cannot multiply two column vectors", expr);
    } else if (!ltype->isColumnVector && !rtype->isColumnVector) {
      reportError("cannot multiply two row vectors", expr);
    } else if (!compareTypes((*lhsType)[0], (*rhsType)[0])) {
      std::stringstream errMsg;
      errMsg << "cannot multiply vectors of type " << typeString(lhsType) 
             << " and type " << typeString(rhsType);
      reportError(errMsg.str(), expr);
    } else {
      std::vector<ir::IndexDomain> dom;
      if (ltype->isColumnVector) {
        dom.push_back(ldimensions[0]);
        dom.push_back(rdimensions[0]);
      }
      const auto tensorType = ir::TensorType::make(ltype->componentType, dom);
      retType = std::make_shared<Expr::Type>();
      retType->push_back(ir::Type(tensorType));  
    }
  } else if (ltype->order() == 2 && rtype->order() == 1) {
    // Matrix-Vector
    if (ldimensions[1] != rdimensions[0] || 
        ltype->componentType != rtype->componentType) {
      std::stringstream errMsg;
      errMsg << "cannot multiply a matrix of type " << typeString(lhsType)
             << " by a vector of type " << typeString(rhsType);
      reportError(errMsg.str(), expr);
    //} else if (!rtype->isColumnVector) {
    //  reportError("Cannot multiply a matrix by a row vector", expr);
    } else {
      const auto tensorType = ir::TensorType::make(ltype->componentType, 
                                                   {ldimensions[0]}, true);
      retType = std::make_shared<Expr::Type>();
      retType->push_back(ir::Type(tensorType));
    }
  } else if (ltype->order() == 1 && rtype->order() == 2) {
    // Vector-Matrix
    if (ldimensions[0] != rdimensions[0] || 
        ltype->componentType != rtype->componentType) {
      std::stringstream errMsg;
      errMsg << "cannot multiply a vector of type " << typeString(lhsType)
             << " by a matrix of type " << typeString(rhsType);
      reportError(errMsg.str(), expr);
    //} else if (ltype->isColumnVector) {
    //  reportError("Cannot multiply a column vector by a matrix", expr);
    } else {
      const auto tensorType = ir::TensorType::make(ltype->componentType, 
                                                   {rdimensions[1]});
      retType = std::make_shared<Expr::Type>();
      retType->push_back(ir::Type(tensorType));
    }
  } else if (ltype->order() == 2 && rtype->order() == 2) {
    // Matrix-Matrix
    if (ldimensions[1] != rdimensions[0] || 
        ltype->componentType != rtype->componentType) {
      std::stringstream errMsg;
      errMsg << "cannot multiply a matrix of type " << typeString(lhsType)
             << " by a matrix of type " << typeString(rhsType);
      reportError(errMsg.str(), expr);
    } else {
      const std::vector<ir::IndexDomain> dom = {ldimensions[0], rdimensions[1]};
      const auto tensorType = ir::TensorType::make(ltype->componentType, dom);
      retType = std::make_shared<Expr::Type>();
      retType->push_back(ir::Type(tensorType));
    }
  } else {
    reportError("cannot multiply tensors of order 3 or greater using *", expr);
  }
}

void TypeChecker::visit(DivExpr::Ptr expr) {
  const Ptr<Expr::Type> lhsType = inferType(expr->lhs);
  const Ptr<Expr::Type> rhsType = inferType(expr->rhs);

  bool typeChecked = (bool)lhsType && (bool)rhsType;
  if (lhsType && (lhsType->size() != 1 || !(*lhsType)[0].isTensor())) {
    std::stringstream errMsg;
    errMsg << "expected left operand of division operation to be a tensor "
           << "but got an operand of type " << typeString(lhsType);
    reportError(errMsg.str(), expr->lhs);
    typeChecked = false;
  }
  if (rhsType && (rhsType->size() != 1 || !(*rhsType)[0].isTensor())) {
    std::stringstream errMsg;
    errMsg << "expected right operand of division operation to be a tensor "
           << "but got an operand of type " << typeString(rhsType);
    reportError(errMsg.str(), expr->rhs);
    typeChecked = false;
  }
 
  if (!typeChecked) return;

  const unsigned lhsOrder = (*lhsType)[0].toTensor()->order();
  const unsigned rhsOrder = (*rhsType)[0].toTensor()->order();
  if (lhsOrder > 0 && rhsOrder > 0) {
    std::stringstream errMsg;
    errMsg << "division of a non-scalar tensor of type " << typeString(lhsType)
           << " by a non-scalar tensor of type " << typeString(rhsType)
           << " is not supported";
    reportError(errMsg.str(), expr);
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
  const Ptr<Expr::Type> opndType = inferType(expr->operand);

  if (!opndType) {
    return;
  }

  if (opndType->size() != 1 || !(*opndType)[0].isTensor()) {
    std::stringstream errMsg;
    errMsg << "expected operand of tensor negation to be a tensor but got an "
           << "operand of type " << typeString(opndType);
    reportError(errMsg.str(), expr->operand);
  } else {
    retType = opndType;
  }
}

void TypeChecker::visit(ExpExpr::Ptr expr) {
  // TODO: Implement.
  not_supported_yet; 
}

void TypeChecker::visit(TransposeExpr::Ptr expr) {
  const Ptr<Expr::Type> opndType = inferType(expr->operand);

  if (!opndType) {
    return;
  }

  if (opndType->size() != 1 || !opndType->at(0).isTensor() || 
      opndType->at(0).toTensor()->order() > 2) {
    std::stringstream errMsg;
    errMsg << "operand of tensor transpose must be a tensor of order 2 or "
           << "less, but got an operand of type " << typeString(opndType);
    reportError(errMsg.str(), expr->operand);
    return;
  }

  const ir::TensorType *tensorType = opndType->at(0).toTensor();
  const std::vector<ir::IndexDomain> dimensions = tensorType->getDimensions();
  switch (tensorType->order()) {
    case 0:
      retType = opndType;
      break;
    case 1:
    {
      const auto exprType = ir::TensorType::make(tensorType->componentType, 
          dimensions, !tensorType->isColumnVector);
      retType = std::make_shared<Expr::Type>();
      retType->push_back(ir::Type(exprType));
      break;
    }
    case 2:
    {
      const auto exprType = ir::TensorType::make(
          tensorType->componentType, {dimensions[1], dimensions[0]});
      retType = std::make_shared<Expr::Type>();
      retType->push_back(ir::Type(exprType));
      break;
    }
    default:
      unreachable;
      break;
  }
}

void TypeChecker::visit(CallExpr::Ptr expr) {
  iassert(ctx.containsFunction(expr->func->ident));
  const ir::Func func = ctx.getFunction(expr->func->ident);

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
      const Ptr<Expr::Type> argType = inferType(operand);
  
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
        errMsg << "expected argument of type '" 
               << typeString(func.getArguments()[i].getType()) 
               << "' but got an argument of type " << typeString(argType);
        reportError(errMsg.str(), operand);
      }
    }
  }

  retType = std::make_shared<Expr::Type>();
  for (const auto &res : func.getResults()) {
    retType->push_back(res.getType());
  }
}

void TypeChecker::visit(TensorReadExpr::Ptr expr) {
  const Ptr<Expr::Type> lhsType = inferType(expr->tensor);

  if (!lhsType) {
    return;
  }

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
        const Ptr<Expr::Type> paramType = inferType(paramExpr);

        if (!paramType) {
          continue;
        } else if (paramType->size() != 1) {
          std::stringstream errMsg;
          if (paramType->size() == 0) {
            errMsg << "must pass a value as index";
          } else {
            errMsg << "cannot pass multiple values of types " 
                   << typeString(paramType) << " as a single index";
          }
          reportError(errMsg.str(), param);
          continue;
        } //else if (paramType->at(0)

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

    retType = std::make_shared<Expr::Type>();
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
    } else if (expr->indices[0]->isSlice()) {
      reportError("tuple access requires an integral index", expr->indices[0]);
      return;
    } else {
      const Expr::Ptr indexExpr = to<ExprParam>(expr->indices[0])->expr;
      const Ptr<Expr::Type> indexType = inferType(indexExpr);
      if (indexType->size() != 1 || !isInt(indexType->at(0))) {
        std::stringstream errMsg;
        errMsg << "tuple access expects an integral index but got an index of " 
               << "type " << typeString(indexType);
        reportError(errMsg.str(), expr->indices[0]);
      }
    }

    retType = std::make_shared<Expr::Type>();
    retType->push_back(lhsType->at(0).toTuple()->elementType);
  } else {
    std::stringstream errMsg;
    errMsg << "cannot access elements from objects of type " 
           << typeString(lhsType);
    reportError(errMsg.str(), expr->tensor);
  }
}

void TypeChecker::visit(TupleReadExpr::Ptr expr) {
  // Tuple reads are parsed as tensor reads during parsing.
  unreachable;
}

void TypeChecker::visit(FieldReadExpr::Ptr expr) {
  const Ptr<Expr::Type> lhsType = inferType(expr->setOrElem);

  if (!lhsType) {
    return;
  }

  if (lhsType->size() != 1) {
    const auto msg = "can only access fields of a single set or element";
    reportError(msg, expr->setOrElem);
    return;
  }

  const ir::Type type = lhsType->at(0);
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

  const std::string fieldName = expr->field->ident;
  if (!elemType->hasField(fieldName)) {
    std::stringstream errMsg;
    errMsg << "undefined field '" << fieldName << "'";
    reportError(errMsg.str(), expr->field);
    return;
  }

  retType = std::make_shared<Expr::Type>();
  if (type.isElement()) {
    retType->push_back(elemType->field(fieldName).type);
  } else {
    const std::string varName = to<VarExpr>(expr->setOrElem)->ident;
    const ir::Expr setExpr = ctx.getSymbol(varName).getExpr();
    retType->push_back(getFieldType(setExpr, fieldName));
  }
}

void TypeChecker::visit(VarExpr::Ptr expr) {
  if (!ctx.hasSymbol(expr->ident)) {
    if (!skipCheckDeclared) {
      reportUndeclared("variable or constant", expr->ident, expr);
    }
    return;
  }
  
  const internal::Symbol varSym = ctx.getSymbol(expr->ident);
  if (expr == checkWritable && !varSym.isWritable()) {
    std::stringstream errMsg;
    errMsg << "'" << expr->ident << "' is not writable";
    reportError(errMsg.str(), expr);
  } else if (expr != checkWritable && !varSym.isReadable()) {
    std::stringstream errMsg;
    errMsg << "'" << expr->ident << "' is not readable";
    reportError(errMsg.str(), expr);
  }

  retType = std::make_shared<Expr::Type>();
  retType->push_back(varSym.getExpr().type());
}

void TypeChecker::visit(IntLiteral::Ptr lit) {
  retType = std::make_shared<Expr::Type>();
  const auto componentType = ir::ScalarType(ir::ScalarType::Int);
  retType->push_back(ir::Type(ir::TensorType::make(componentType)));
}

void TypeChecker::visit(FloatLiteral::Ptr lit) {
  retType = std::make_shared<Expr::Type>();
  const auto componentType = ir::ScalarType(ir::ScalarType::Float);
  retType->push_back(ir::Type(ir::TensorType::make(componentType)));
}

void TypeChecker::visit(BoolLiteral::Ptr lit) {
  retType = std::make_shared<Expr::Type>();
  const auto componentType = ir::ScalarType(ir::ScalarType::Boolean);
  retType->push_back(ir::Type(ir::TensorType::make(componentType)));
}

void TypeChecker::visit(IntVectorLiteral::Ptr lit) {
  typeCheckDenseTensorLiteral(lit);
}

void TypeChecker::visit(FloatVectorLiteral::Ptr lit) {
  typeCheckDenseTensorLiteral(lit);
}

void TypeChecker::visit(NDTensorLiteral::Ptr lit) {
  typeCheckDenseTensorLiteral(lit);
}

void TypeChecker::typeCheckDenseTensorLiteral(DenseTensorLiteral::Ptr lit) {
  try {
    const DenseTensorType tensorType = getDenseTensorType(lit);
    const std::vector<ir::IndexDomain> idoms(tensorType.dimSizes.rbegin(), 
                                             tensorType.dimSizes.rend());
    const auto elemType = (tensorType.type == DenseTensorType::Type::INT) ?
                          ir::ScalarType::Int : ir::ScalarType::Float;
    iassert(idoms.size() == 1 || !lit->transposed);

    retType = std::make_shared<Expr::Type>();
    retType->push_back(ir::TensorType::make(elemType, idoms, lit->transposed));
  } catch (std::exception &err) {
    reportError(std::string(err.what()), lit);
  }
}

TypeChecker::DenseTensorType 
    TypeChecker::getDenseTensorType(DenseTensorLiteral::Ptr lit) {
  DenseTensorType tensorType;
  
  if (isa<IntVectorLiteral>(lit)) {
    tensorType.addIntValues(to<IntVectorLiteral>(lit)->vals.size());
  } else if (isa<FloatVectorLiteral>(lit)) {
    tensorType.addFloatValues(to<FloatVectorLiteral>(lit)->vals.size());
  } else {
    const auto ndTensorLit = to<NDTensorLiteral>(lit);
    iassert(!ndTensorLit->transposed);
  
    tensorType = getDenseTensorType(ndTensorLit->elems[0]);
    tensorType.addDimension();
  
    for (unsigned i = 1; i < ndTensorLit->elems.size(); ++i) {
      const DenseTensorType right = getDenseTensorType(ndTensorLit->elems[i]);
      tensorType.merge(right);
    }
  }

  return tensorType;
}

void TypeChecker::visit(Test::Ptr test) {
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
}

void TypeChecker::typeCheckVarOrConstDecl(VarDecl::Ptr decl, 
                                          const bool isConst) {
  const ir::Var var = getVar(decl->var);
  const auto initType = decl->initVal ? inferType(decl->initVal) : 
                        Ptr<Expr::Type>();

  if (ctx.hasSymbol(var.getName(), true)) {
    reportMultipleDefs("variable or constant", var.getName(), decl);
  } else {
    const auto access = isConst ? internal::Symbol::Read : 
                        internal::Symbol::ReadWrite;
    ctx.addSymbol(var.getName(), var, access);
  }

  const ir::Type varType = var.getType();
  if (!initType || (initType->size() == 1 && 
      compareTypes(varType, initType->at(0)))) {
    // Initial value type matches declared variable/constant type.
    return;
  }

  std::stringstream errMsg;
  errMsg << "cannot initialize a variable or constant of type '"
         << typeString(var.getType()) << "' with an expression of type "
         << typeString(initType);

  iassert(varType.isTensor());
  if (initType->size() != 1 || !initType->at(0).isTensor()) {
    reportError(errMsg.str(), decl);
    return;
  }

  // Check if attempting to initialize a tensor with a scalar.
  const ir::Type initIRType = initType->at(0);
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
  const Ptr<Expr::Type> lhsType = inferType(expr->lhs);
  const Ptr<Expr::Type> rhsType = inferType(expr->rhs);

  bool typeChecked = (bool)lhsType && (bool)rhsType;
  if (lhsType && (lhsType->size() != 1 || !lhsType->at(0).isTensor())) {
    std::stringstream errMsg;
    errMsg << "expected left operand of element-wise operation to be a tensor "
           << "but got an operand of type " << typeString(lhsType);
    reportError(errMsg.str(), expr->lhs);
    typeChecked = false;
  }
  if (rhsType && (rhsType->size() != 1 || !rhsType->at(0).isTensor())) {
    std::stringstream errMsg;
    errMsg << "expected right operand of element-wise operation to be a tensor "
           << "but got an operand of type " << typeString(rhsType);
    reportError(errMsg.str(), expr->rhs);
    typeChecked = false;
  }
 
  if (!typeChecked) {
    return;
  }

  const ir::TensorType *ltype = lhsType->at(0).toTensor();
  const ir::TensorType *rtype = rhsType->at(0).toTensor();
  const unsigned lhsOrder = ltype->order();
  const unsigned rhsOrder = rtype->order();
  const bool hasScalarOperand = (lhsOrder == 0 || rhsOrder == 0);
  if (hasScalarOperand ? (ltype->componentType != rtype->componentType) : 
      !compareTypes(lhsType->at(0), rhsType->at(0))) {
    std::stringstream errMsg;
    errMsg << "cannot perform element-wise operation on tensors of type "
           << typeString(lhsType) << " and type " << typeString(rhsType);
    reportError(errMsg.str(), expr);
  } else {
    retType = (lhsOrder > 0) ? lhsType : rhsType;
  }
}

void TypeChecker::typeCheckBinaryBoolean(BinaryExpr::Ptr expr) {
  const Ptr<Expr::Type> lhsType = inferType(expr->lhs);
  const Ptr<Expr::Type> rhsType = inferType(expr->rhs);

  if (lhsType && (lhsType->size() != 1 || !isBoolean(lhsType->at(0)))) {
    std::stringstream errMsg;
    errMsg << "expected left operand of boolean operation to be a boolean "
           << "but got an operand of type " << typeString(lhsType);
    reportError(errMsg.str(), expr->lhs);
  }
  if (rhsType && (rhsType->size() != 1 || !isBoolean(rhsType->at(0)))) {
    std::stringstream errMsg;
    errMsg << "expected right operand of boolean operation to be a boolean "
           << "but got an operand of type " << typeString(rhsType);
    reportError(errMsg.str(), expr->rhs);
  }

  retType = std::make_shared<Expr::Type>();
  const auto componentType = ir::ScalarType(ir::ScalarType::Boolean);
  retType->push_back(ir::Type(ir::TensorType::make(componentType)));
}

}
}
