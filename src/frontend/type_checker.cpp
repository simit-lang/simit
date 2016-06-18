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

void TypeChecker::visit(Program::Ptr program) {
  for (auto elem : program->elems) {
    if (isa<ConstDecl>(elem)) {
      typeCheckVarOrConstDecl(to<ConstDecl>(elem), true, true);
    } else {
      elem->accept(this);
    }
  }
}

void TypeChecker::visit(RangeIndexSet::Ptr set) {
  retIndexSet = std::make_shared<ir::IndexSet>(set->range);
}

void TypeChecker::visit(SetIndexSet::Ptr set) {
  // Check that index set has been previously declared.
  if (!ctx.hasSymbol(set->setName)) {
    reportUndeclared("set", set->setName, set);
    return;
  }

  const ir::Expr setExpr = ctx.getSymbol(set->setName).getExpr();
  
  // Check that index set pointed to by identifier is indeed of set type.
  if (!setExpr.type().isSet()) {
    reportError("index set must be a set, a range, or dynamic (*)", set);
    return;
  }

  retIndexSet = std::make_shared<ir::IndexSet>(setExpr);
}

void TypeChecker::visit(DynamicIndexSet::Ptr set) {
  retIndexSet = std::make_shared<ir::IndexSet>();
}

void TypeChecker::visit(ElementType::Ptr type) {
  // Check that element type has been previously declared.
  if (!ctx.containsElementType(type->ident)) {
    reportUndeclared("element type", type->ident, type);
    return;
  }

  retIRType = ctx.getElementType(type->ident);
}

void TypeChecker::visit(Endpoint::Ptr end) {
  // Check that endpoint has been previously declared.
  if (!ctx.hasSymbol(end->setName)) {
    reportUndeclared("set", end->setName, end);
    return;
  }
 
  retExpr = ctx.getSymbol(end->setName).getExpr();
}

void TypeChecker::visit(SetType::Ptr type) {
  const ir::Type elementType = getIRType(type->element);
  bool typeChecked = elementType.defined();

  std::vector<ir::Expr> endpoints;
  for (auto end : type->endpoints) {
    const ir::Expr endpoint = getExpr(end);
    
    if (!endpoint.defined()) {
      typeChecked = false;
      continue;
    }

    // Check that endpoint is of set type.
    if (!endpoint.type().isSet()) {
      std::stringstream errMsg;
      errMsg << "expected endpoint to be of set type but got an endpoint of "
             << "type " << typeString(endpoint.type());
      reportError(errMsg.str(), end);
      
      typeChecked = false;
      continue;
    }
      
    endpoints.push_back(endpoint);
  }

  if (!typeChecked) {
    return;
  }
 
  retIRType = ir::SetType::make(elementType, endpoints);
}

void TypeChecker::visit(TupleType::Ptr type) {
  const ir::Type elementType = getIRType(type->element);

  // Check that tuple length is positive.
  if (type->length->val < 1) {
    const auto msg = "tuple must have length greater than or equal to one";
    reportError(msg, type->length);
    return;
  }

  if (!elementType.defined()) {
    return;
  }
  
  retIRType = ir::TupleType::make(elementType, type->length->val);
}

void TypeChecker::visit(ScalarType::Ptr type) {
  switch (type->type) {
    case ScalarType::Type::INT:
      retIRType = ir::Int;
      break;
    case ScalarType::Type::FLOAT:
      retIRType = ir::Float;
      break;
    case ScalarType::Type::BOOL:
      retIRType = ir::Boolean;
      break;
    case ScalarType::Type::COMPLEX:
      retIRType = ir::Complex;
      break;
    case ScalarType::Type::STRING:
      retIRType = ir::String;
      break;
    default:
      unreachable;
      break;
  }
}

void TypeChecker::visit(NDTensorType::Ptr type) {
  const ir::Type blockType = getIRType(type->blockType);
  bool typeChecked = blockType.defined();

  std::vector<ir::IndexSet> indexSets;
  for (auto is : type->indexSets) {
    const Ptr<ir::IndexSet> indexSet = getIndexSet(is);

    if (!indexSet) {
      typeChecked = false;
      continue;
    }
    
    indexSets.push_back(*indexSet);
  }

  if (!typeChecked) {
    return;
  }

  ir::Type ndTensorType;
  if (indexSets.empty()) {
    ndTensorType = blockType;
  } else {
    const auto blockTensorType = blockType.toTensor();
    const auto componentType = blockTensorType->getComponentType();
    const auto blockDimensions = blockTensorType->getDimensions();
 
    // Check that tensor type has same number of dimensions as inner block.
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
      const auto msg = "blocked tensor type must contain same number "
                       "of dimensions as its blocks";
      reportError(msg, type);
      return;
    }
 
    ndTensorType = ir::TensorType::make(componentType, dimensions,
                                        dimensions.size() == 1);
  }

  if (type->transposed) {
    const auto tensorType = ndTensorType.toTensor();
    const auto dimensions = tensorType->getDimensions();
    const auto componentType = tensorType->getComponentType();

    // Check that transposed type is a vector.
    if (dimensions.size() != 1) {
      reportError("transposed type must be a vector", type);
      return;
    }

    ndTensorType = ir::TensorType::make(componentType, dimensions);
  }

  retIRType = ndTensorType;
}

void TypeChecker::visit(IdentDecl::Ptr decl) {
  const ir::Type type = getIRType(decl->type);
  retVar = ir::Var(decl->name->ident, type);
}

void TypeChecker::visit(Field::Ptr field) {
  const IdentDecl::Ptr fieldDecl = field->field;
  const ir::Type type = getIRType(fieldDecl->type);
  retField = ir::Field(fieldDecl->name->ident, type);
}

void TypeChecker::visit(ElementTypeDecl::Ptr decl) {
  std::vector<ir::Field> fields;
  for (auto f : decl->fields) {
    const ir::Field field = getField(f);

    if (!field.type.defined()) {
      continue;
    }

    fields.push_back(field);
  }

  const std::string name = decl->name->ident;
  
  // Check that element type has not been previously declared.
  if (ctx.containsElementType(name)) {
    reportMultipleDefs("element type", name, decl);
    return;
  }
  
  ctx.addElementType(ir::ElementType::make(name, fields));
}

void TypeChecker::visit(ExternDecl::Ptr decl) {
  const ir::Var externVar = getVar(decl->var);

  // Check that variable has not been previously declared.
  if (ctx.hasSymbol(externVar.getName())) {
    reportMultipleDefs("variable or constant", externVar.getName(), decl);
    return;
  }
  
  ctx.addSymbol(externVar);
}

void TypeChecker::visit(FuncDecl::Ptr decl) {
  bool typeChecked = true;

  ctx.scope();

  std::vector<ir::Var> arguments;
  for (auto arg : decl->args) {
    const ir::Var argVar = getVar(arg->arg);

    if (!argVar.getType().defined()) {
      typeChecked = false;
    }

    internal::Symbol::Access access = internal::Symbol::Read;
    if (arg->isInOut()) {
      access = internal::Symbol::ReadWrite;
      writableArgs.insert(argVar);
    }
    
    ctx.addSymbol(argVar.getName(), argVar, access);
    arguments.push_back(argVar);
  }
  
  std::vector<ir::Var> results;
  for (auto res : decl->results) {
    const ir::Var result = getVar(res);

    if (!result.getType().defined()) {
      typeChecked = false;
    }

    ctx.addSymbol(result);
    results.push_back(result);
  }

  if (decl->body != nullptr) {
    decl->body->accept(this);
  }
  ctx.unscope();

  if (!typeChecked) {
    return;
  }

  const std::string name = decl->name->ident;
  
  // Check that function has not been previously declared.
  if (ctx.containsFunction(name)) {
    reportMultipleDefs("function or procedure", name, decl);
    return;
  }

  ctx.addFunction(ir::Func(name, arguments, results, ir::Stmt()));
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

  // Check that conditional expression is boolean.
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

  // Check that conditional expression is boolean.
  if (condType && (condType->size() != 1 || !isBoolean(condType->at(0)))) {
    std::stringstream errMsg;
    errMsg << "expected a boolean conditional expression but got an "
           << "expression of type " << typeString(condType);
    reportError(errMsg.str(), stmt->cond);
  }
}

void TypeChecker::visit(RangeDomain::Ptr domain) {
  const Ptr<Expr::Type> lowerType = inferType(domain->lower);
  const Ptr<Expr::Type> upperType = inferType(domain->upper);

  // Check that lower and upper bounds of for-loop range are integral.
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
 
  ir::Type loopVarType;
  if (isa<IndexSetDomain>(stmt->domain)) {
    const auto setDomain = to<IndexSetDomain>(stmt->domain);
    const Ptr<ir::IndexSet> indexSet = getIndexSet(setDomain->set);
    if (indexSet) {
      loopVarType = indexSet->getSet().type().toSet()->elementType;
    }
  } else {
    stmt->domain->accept(this);
    loopVarType = ir::Int;
  }

  const ir::Var loopVar = ir::Var(stmt->loopVar->ident, loopVarType);
  ctx.addSymbol(stmt->loopVar->ident, loopVar, internal::Symbol::Read);
  
  stmt->body->accept(this);
  ctx.unscope();
}

void TypeChecker::visit(PrintStmt::Ptr stmt) {
  for (const auto arg : stmt->args) {
    const Ptr<Expr::Type> argType = inferType(arg);

    // Check that print statement is printing a tensor.
    if (argType && (argType->size() != 1 || !argType->at(0).isTensor())) {
      std::stringstream errMsg;
      errMsg << "cannot print an expression of type " << typeString(argType);
      reportError(errMsg.str(), arg);
    }
  }
}

void TypeChecker::visit(AssignStmt::Ptr stmt) {
  const Ptr<Expr::Type> exprType = inferType(stmt->expr);
  bool typeChecked = (bool)exprType;

  Expr::Type lhsType;
  for (auto lhs : stmt->lhs) {
    // If assigning directly to a variable, then the variable does not have to 
    // be declared beforehand, so skip that check later.
    skipCheckDeclared = isa<VarExpr>(lhs);
    
    const Ptr<Expr::Type> ltype = inferType(lhs);
    if (ltype && ltype->size() == 1) {
      lhsType.push_back(ltype->at(0));
    } else {
      lhsType.push_back(ir::Type());
    }
    
    skipCheckDeclared = false;
  }

  // Check that number of values returned by expression on right-hand side
  // (may not equal to one if it is a map operation or function call) is equal 
  // to number of assignment targets.
  if (typeChecked && stmt->lhs.size() != exprType->size()) {
    std::stringstream errMsg;
    errMsg << "cannot assign an expression returning " << exprType->size()
           << " values to " << stmt->lhs.size() << " targets";
    reportError(errMsg.str(), stmt);
    typeChecked = false;
  }

  if (typeChecked) {
    for (unsigned i = 0; i < stmt->lhs.size(); ++i) {
      // Check that type of value returned by expression on right-hand side 
      // corresponds to type of target on left-hand side.
      if (lhsType[i].defined() && !compareTypes(lhsType[i], exprType->at(i))) {
        // Allow initialization of tensors with scalars.
        if (!lhsType[i].isTensor() || !isScalar(exprType->at(i)) || 
            lhsType[i].toTensor()->getComponentType() != 
            exprType->at(i).toTensor()->getComponentType()) {
          std::stringstream errMsg;
          errMsg << "cannot assign a value of type " 
                 << typeString(exprType->at(i)) << " to a target of type " 
                 << typeString(lhsType[i]);
          reportError(errMsg.str(), stmt->lhs[i]);
          typeChecked = false;
        }
      }

      // Check that expression on right-hand side returns only tensors.
      // TODO: Remove once support for assignment of non-tensors is added.
      if (!exprType->at(i).isTensor()) {
        std::stringstream errMsg;
        errMsg << "cannot assign a non-tensor value of type "
               << typeString(exprType->at(i)) << " to a variable";
        reportError(errMsg.str(), stmt->expr);
      }

      // Check that target is writable.
      if (!stmt->lhs[i]->isWritable()) {
        reportError("assignment target is not writable", stmt->lhs[i]);
      }
    }
  }
  
  for (unsigned i = 0; i < stmt->lhs.size(); ++i) {
    // Mark target variable as having been declared if necessary.
    if (isa<VarExpr>(stmt->lhs[i])) {
      const std::string varName = to<VarExpr>(stmt->lhs[i])->ident;
      if (!ctx.hasSymbol(varName)) {
        const ir::Type varType = typeChecked ? exprType->at(i) : ir::Type();
        const auto var = ir::Var(varName, varType);
        ctx.addSymbol(varName, var, internal::Symbol::Read);
      }
    }
  }
}

void TypeChecker::visit(MapExpr::Ptr expr) {
  typeCheckMapOrApply(expr);
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
    
    // Check that comparison operation is performed on scalar values.
    if (opndType->size() != 1 || !isScalar(opndType->at(0))) {
      std::stringstream errMsg;
      errMsg << "comparison operations can only be performed on scalar "
             << "values, not values of type " << typeString(opndType);
      reportError(errMsg.str(), operand);
      continue;
    }

    // Check that operands of comparison operation are of the same type.
    if (!repType) {
      repType = opndType;
    } else if (!compareTypes(repType->at(0), opndType->at(0))) {
      std::stringstream errMsg;
      errMsg << "value of type " << typeString(opndType) << " cannot be "
             << "compared to value of type " << typeString(repType);
      reportError(errMsg.str(), operand);
    }
  }
  
  retType = std::make_shared<Expr::Type>();
  retType->push_back(ir::Boolean);
}

void TypeChecker::visit(NotExpr::Ptr expr) {
  const Ptr<Expr::Type> opndType = inferType(expr->operand);

  // Check that operand of boolean not is boolean.
  if (opndType && (opndType->size() != 1 || !isBoolean(opndType->at(0)))) {
    std::stringstream errMsg;
    errMsg << "expected a boolean operand but got an operand of type "
           << typeString(opndType);
    reportError(errMsg.str(), expr->operand);
  }

  retType = std::make_shared<Expr::Type>();
  retType->push_back(ir::Boolean);
}

void TypeChecker::visit(AddExpr::Ptr expr) {
  typeCheckBinaryElwise(expr, true); 
}

void TypeChecker::visit(SubExpr::Ptr expr) {
  typeCheckBinaryElwise(expr); 
}

void TypeChecker::visit(MulExpr::Ptr expr) {
  const Ptr<Expr::Type> lhsType = inferType(expr->lhs);
  const Ptr<Expr::Type> rhsType = inferType(expr->rhs);
  bool typeChecked = (bool)lhsType && (bool)rhsType;

  // Check that operands of multiplication operation are numeric tensors.
  if (lhsType && (lhsType->size() != 1 || !lhsType->at(0).isTensor() || 
      !lhsType->at(0).toTensor()->getComponentType().isNumeric())) {
    std::stringstream errMsg;
    errMsg << "expected left operand of multiplication operation to be a "
           << "numeric tensor but got an operand of type " 
           << typeString(lhsType);
    reportError(errMsg.str(), expr->lhs);
    typeChecked = false;
  }
  if (rhsType && (rhsType->size() != 1 || !rhsType->at(0).isTensor() || 
      !rhsType->at(0).toTensor()->getComponentType().isNumeric())) {
    std::stringstream errMsg;
    errMsg << "expected right operand of multiplication operation to be a "
           << "numeric tensor but got an operand of type " 
           << typeString(rhsType);
    reportError(errMsg.str(), expr->rhs);
    typeChecked = false;
  }
 
  if (!typeChecked) {
    return;
  }

  const ir::TensorType *ltype = lhsType->at(0).toTensor();
  const ir::TensorType *rtype = rhsType->at(0).toTensor();
  const std::vector<ir::IndexDomain> ldimensions = ltype->getDimensions();
  const std::vector<ir::IndexDomain> rdimensions = rtype->getDimensions();
  const unsigned lhsOrder = ltype->order();
  const unsigned rhsOrder = rtype->order();

  // Check that operands of multiplication operation contain elements 
  // of the same type.
  if (ltype->getComponentType() != rtype->getComponentType()) {
    std::stringstream errMsg;
    errMsg << "cannot multiply tensors containing elements of type '"
           << ltype->getComponentType() << "' and type '"
           << rtype->getComponentType() << "'";
    reportError(errMsg.str(), expr);
    return;
  }

  if (lhsOrder == 0 || rhsOrder == 0) {
    const auto tensorType = (lhsOrder > 0) ? lhsType->at(0) : rhsType->at(0); 
    retType = std::make_shared<Expr::Type>();
    retType->push_back(tensorType);
  } else if (lhsOrder == 1 && rhsOrder == 1) {
    // Check dimensions of operands for vector-vector multiplication.
    if (ltype->isColumnVector && rtype->isColumnVector) {
      reportError("cannot multiply two column vectors", expr);
      return;
    } else if (!ltype->isColumnVector && !rtype->isColumnVector) {
      reportError("cannot multiply two row vectors", expr);
      return;
    } else if (ldimensions[0] != rdimensions[0]) {
      std::stringstream errMsg;
      errMsg << "cannot multiply vectors of type " << typeString(lhsType) 
             << " and type " << typeString(rhsType);
      reportError(errMsg.str(), expr);
      return;
    }
    
    std::vector<ir::IndexDomain> dom;
    if (ltype->isColumnVector) {
      dom.push_back(ldimensions[0]);
      dom.push_back(rdimensions[0]);
    }

    retType = std::make_shared<Expr::Type>();
    retType->push_back(ir::TensorType::make(ltype->getComponentType(), dom));
  } else if (lhsOrder == 2 && rhsOrder == 1) {
    // Check dimensions of operands for matrix-vector multiplication.
    if (ldimensions[1] != rdimensions[0]) {
      std::stringstream errMsg;
      errMsg << "cannot multiply a matrix of type " << typeString(lhsType)
             << " by a vector of type " << typeString(rhsType);
      reportError(errMsg.str(), expr);
      return;
    } else if (!rtype->isColumnVector) {
      reportError("Cannot multiply a matrix by a row vector", expr);
    }
    
    const auto tensorType = ir::TensorType::make(ltype->getComponentType(), 
                                                 {ldimensions[0]}, true);
    retType = std::make_shared<Expr::Type>();
    retType->push_back(tensorType);
  } else if (lhsOrder == 1 && rhsOrder == 2) {
    // Check dimensions of operands for vector-matrix multiplication.
    if (ldimensions[0] != rdimensions[0] || 
        ltype->getComponentType() != rtype->getComponentType()) {
      std::stringstream errMsg;
      errMsg << "cannot multiply a vector of type " << typeString(lhsType)
             << " by a matrix of type " << typeString(rhsType);
      reportError(errMsg.str(), expr);
      return;
    } else if (ltype->isColumnVector) {
      reportError("Cannot multiply a column vector by a matrix", expr);
    }

    const auto tensorType = ir::TensorType::make(ltype->getComponentType(), 
                                                 {rdimensions[1]});
    retType = std::make_shared<Expr::Type>();
    retType->push_back(tensorType);
  } else if (lhsOrder == 2 && rhsOrder == 2) {
    // Check dimensions of operands for matrix-matrix multiplication.
    if (ldimensions[1] != rdimensions[0]) {
      std::stringstream errMsg;
      errMsg << "cannot multiply matrices of type " << typeString(lhsType)
             << " and type " << typeString(rhsType);
      reportError(errMsg.str(), expr);
      return;
    }
    
    const std::vector<ir::IndexDomain> dom = {ldimensions[0], rdimensions[1]};
    retType = std::make_shared<Expr::Type>();
    retType->push_back(ir::TensorType::make(ltype->getComponentType(), dom));
  } else {
    reportError("cannot multiply tensors of order 3 or greater using *", expr);
  }
}

void TypeChecker::visit(DivExpr::Ptr expr) {
  const Ptr<Expr::Type> lhsType = inferType(expr->lhs);
  const Ptr<Expr::Type> rhsType = inferType(expr->rhs);
  bool typeChecked = (bool)lhsType && (bool)rhsType;

  // Check that operands of division operation are numeric tensors.
  if (lhsType && (lhsType->size() != 1 || !lhsType->at(0).isTensor() || 
      !lhsType->at(0).toTensor()->getComponentType().isNumeric())) {
    std::stringstream errMsg;
    errMsg << "expected left operand of division operation to be a numeric "
           << "tensor but got an operand of type " << typeString(lhsType);
    reportError(errMsg.str(), expr->lhs);
    typeChecked = false;
  }
  if (rhsType && (rhsType->size() != 1 || !rhsType->at(0).isTensor() || 
      !rhsType->at(0).toTensor()->getComponentType().isNumeric())) {
    std::stringstream errMsg;
    errMsg << "expected right operand of division operation to be a numeric "
           << "tensor but got an operand of type " << typeString(rhsType);
    reportError(errMsg.str(), expr->rhs);
    typeChecked = false;
  }
 
  if (!typeChecked) {
    return;
  }
  
  const ir::TensorType *ltype = lhsType->at(0).toTensor();
  const ir::TensorType *rtype = rhsType->at(0).toTensor();

  // Check that operands of division operation contain elements of same type.
  if (ltype->getComponentType() != rtype->getComponentType()) {
    std::stringstream errMsg;
    errMsg << "cannot divide tensors containing elements of type '"
           << ltype->getComponentType() << "' and type '"
           << rtype->getComponentType() << "'";
    reportError(errMsg.str(), expr);
    return;
  }

  // Check for unsupported division of two non-scalar tensors.
  // Probably want to remove this constraint at some point.
  if (ltype->order() > 0 && rtype->order() > 0) {
    std::stringstream errMsg;
    errMsg << "division of a non-scalar tensor of type " << typeString(lhsType)
           << " by a non-scalar tensor of type " << typeString(rhsType)
           << " is not supported";
    reportError(errMsg.str(), expr);
    return;
  }
  
  retType = (ltype->order() > 0) ? lhsType : rhsType;
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

  // Check that operand of negation operation is a numeric tensor.
  if (opndType->size() != 1 || !opndType->at(0).isTensor() ||
      !opndType->at(0).toTensor()->getComponentType().isNumeric()) {
    std::stringstream errMsg;
    errMsg << "expected operand of tensor negation to be a numeric tensor but "
           << "got an operand of type " << typeString(opndType);
    reportError(errMsg.str(), expr->operand);
    return;
  }
  
  retType = opndType;
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

  // Check that operand of transpose operation is tensor of order 2 or less.
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
      const auto exprType = ir::TensorType::make(tensorType->getComponentType(), 
          dimensions, !tensorType->isColumnVector);
      retType = std::make_shared<Expr::Type>();
      retType->push_back(exprType);
      break;
    }
    case 2:
    {
      const auto exprType = ir::TensorType::make(
          tensorType->getComponentType(), {dimensions[1], dimensions[0]});
      retType = std::make_shared<Expr::Type>();
      retType->push_back(exprType);
      break;
    }
    default:
      unreachable;
      break;
  }
}

void TypeChecker::visit(CallExpr::Ptr expr) {
  std::vector<Ptr<Expr::Type>> argTypes(expr->args.size());
  for (unsigned i = 0; i < expr->args.size(); ++i) {
    if (expr->args[i]) {
      argTypes[i] = inferType(expr->args[i]);
    }
  }
  
  if (!ctx.containsFunction(expr->func->ident)) {
    return;
  }

  const std::string funcName = expr->func->ident;
  const ir::Func func = ctx.getFunction(funcName);
  const std::vector<ir::Var> funcArgs = func.getArguments();

  if (expr->args.size() != funcArgs.size()) {
    std::stringstream errMsg;
    errMsg << "passed in " << expr->args.size() << " arguments "
           << "but function '" << funcName << "' expects " << funcArgs.size();
    reportError(errMsg.str(), expr);
  } else {
    for (unsigned i = 0; i < expr->args.size(); ++i) {
      const Expr::Ptr argument = expr->args[i];
      const Ptr<Expr::Type> argType = argTypes[i];
      
      if (!argType) {
        // Could not infer argument type.
        continue;
      }
  
      // Check that argument is a single non-void value.
      if (argType->size() == 0) {
        reportError("must pass a non-void value as argument", argument);
        continue;
      } else if (argType->size() != 1) {
        std::stringstream errMsg;
        errMsg << "cannot pass multiple values of types "
               << typeString(argType) << " as a single argument";
        reportError(errMsg.str(), argument);
        continue;
      }

      // Check that argument is a tensor.
      // TODO: Remove once support for passing non-tensor arguments is added.
      if (!argType->at(0).isTensor()) {
        std::stringstream errMsg;
        errMsg << "expected argument to be a tensor but got an argument "
               << "of type " << typeString(argType->at(0));
        reportError(errMsg.str(), argument);
        continue;
      }

      if (funcName == "norm") {
        // Check that argument to norm intrinsic is a numeric vector.
        if (!argType->at(0).isTensor() || 
            argType->at(0).toTensor()->getDimensions().size() != 1 ||
            !argType->at(0).toTensor()->getComponentType().isNumeric()) {
          reportError("argument must be a numeric vector", argument);
        }
      } else if (funcName == "dot") {
        if (i == 0) {
          // Check that first argument to dot intrinsic is a numeric vector.
          if (!argType->at(0).isTensor() || 
              argType->at(0).toTensor()->getDimensions().size() != 1 ||
              !argType->at(0).toTensor()->getComponentType().isNumeric()) {
            reportError("argument must be a numeric vector", argument);
          }
        } else {
          // Check that second argument to dot intrinsic is a numeric vector of 
          // same length as first argument.
          if (argTypes[0] && argTypes[0]->size() == 1 && 
              argType->at(0) != argTypes[0]->at(0)) {
            std::stringstream errMsg;
            errMsg << "cannot call function 'dot' on arguments of type "
                   << typeString(argTypes[0]->at(0)) << " and type "
                   << typeString(argType->at(0));
            reportError(errMsg.str(), expr);
          }
        }
      } else if (funcName == "solve") {
        // TODO: Should check dimensions of arguments.
        switch (i) {
          case 0:
          case 1:
            break;
          case 2:
            // Check that third argument is writable.
            if (!argument->isWritable()) {
              reportError("inout argument must be writable", argument);
            }
            break;
          default:
            unreachable;
        }
      } else {
        // Check that argument is of type expected by callee.
        if (!compareTypes(argType->at(0), funcArgs[i].getType())) {
          std::stringstream errMsg;
          errMsg << "expected argument of type " 
                 << typeString(funcArgs[i].getType()) << " but got an "
                 << "argument of type " << typeString(argType);
          reportError(errMsg.str(), argument);
        }

        // Check that inout argument is writable.
        if (writableArgs.find(funcArgs[i]) != writableArgs.end() && 
            !argument->isWritable()) {
          reportError("inout argument must be writable", argument);
        }
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
  expr->access = expr->tensor->access;

  if (!lhsType) {
    return;
  }

  // Check that program does not attempt to read from multiple values 
  // simultaneously (e.g. output of function call returning two tensors).
  if (lhsType->size() != 1) {
    const auto msg = "can only access elements of a single tensor or tuple";
    reportError(msg, expr->tensor);
    return;
  }

  // Check that program only ever attempts to read from tensors or tuples.
  if (lhsType->at(0).isTensor()) {
    const ir::TensorType *tensorType = lhsType->at(0).toTensor();
    const auto dimensions = tensorType->getDimensions();
    const auto outerDims = tensorType->getOuterDimensions();

    // Check that right number of indices is passed to tensor read.
    if (tensorType->getDimensions().size() != expr->indices.size()) {
      std::stringstream errMsg;
      errMsg << "tensor access expected " << dimensions.size()
             << " indices but got " << expr->indices.size();
      reportError(errMsg.str(), expr);
      return;
    }

    std::vector<ir::IndexDomain> dims;
    for (unsigned i = 0; i < expr->indices.size(); ++i) {
      const ReadParam::Ptr index = expr->indices[i];

      if (index->isSlice()) {
        dims.push_back(tensorType->getDimensions()[i]);
        continue;
      }

      const Expr::Ptr indexExpr = to<ExprParam>(index)->expr;
      const Ptr<Expr::Type> indexType = inferType(indexExpr);

      if (!indexType) {
        continue;
      }
      
      // Check that index is a single value.
      if (indexType->size() == 0) {
        reportError("must pass a non-void value as index" , index);
        continue;
      } else if (indexType->size() != 1) {
        std::stringstream errMsg;
        errMsg << "cannot pass multiple values of types " 
               << typeString(indexType) << " as a single index";
        reportError(errMsg.str(), index);
        continue;
      }

      // Check that index is of right type.
      switch (outerDims[i].getKind()) {
        case ir::IndexSet::Range:
          if (!isInt(indexType->at(0))) {
            std::stringstream errMsg;
            errMsg << "expected an integral index but got an index of type " 
                   << typeString(indexType);
            reportError(errMsg.str(), index);
          }
          break;
        case ir::IndexSet::Set:
        {
          const auto setType = outerDims[i].getSet().type().toSet();
          if (!compareTypes(setType->elementType, indexType->at(0))) {
            std::stringstream errMsg;
            errMsg << "expected an index of type "
                   << typeString(setType->elementType) << " but got an "
                   << "index of type " << typeString(indexType);
            reportError(errMsg.str(), index);
          }
          break;
        }
        default:
          break;
      }
    }

    retType = std::make_shared<Expr::Type>();
    if (dims.empty()) {
      retType->push_back(tensorType->getBlockType());
    } else {
      const bool isColumnVector = (dims.size() == 1 && 
                                  !expr->indices.back()->isSlice());
      const auto retTensorType = ir::TensorType::make(
          tensorType->getComponentType(), dims, isColumnVector);
      retType->push_back(retTensorType);
    }
  } else if (lhsType->at(0).isTuple()) {
    // Check that tuple read is indexed by an integral index.
    if (expr->indices.size() != 1) {
      std::stringstream errMsg;
      errMsg << "tuple access expects exactly one index but got " 
             << expr->indices.size();
      reportError(errMsg.str(), expr);
    } else if (expr->indices[0]->isSlice()) {
      reportError("tuple access expects an integral index", expr->indices[0]);
    } else {
      const Expr::Ptr indexExpr = to<ExprParam>(expr->indices[0])->expr;
      const Ptr<Expr::Type> indexType = inferType(indexExpr);
      
      if (indexType->size() != 1 || !isInt(indexType->at(0))) {
        std::stringstream errMsg;
        errMsg << "tuple access expects an integral index but got an index " 
               << "of type " << typeString(indexType);
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
  expr->access = expr->setOrElem->access;

  if (!lhsType) {
    return;
  }

  // Check that program does not attempt to read from multiple values 
  // simultaneously (e.g. output of function call returning two tensors).
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
 
  // Check that program only reads fields from sets and elements.
  if (elemType == nullptr) {
    const auto msg = "field accesses are only valid for sets and elements";
    reportError(msg, expr->setOrElem);
    return;
  }

  const std::string fieldName = expr->field->ident;

  // Check that field is defined for set/element being read.
  if (!elemType->hasField(fieldName)) {
    std::stringstream errMsg;
    errMsg << "undefined field '" << fieldName << "'";
    reportError(errMsg.str(), expr->field);
    return;
  }

  if (type.isElement()) {
    retType = std::make_shared<Expr::Type>();
    retType->push_back(elemType->field(fieldName).type);
    return;
  }
  
  const std::string varName = to<VarExpr>(expr->setOrElem)->ident;
  const ir::Expr setExpr = ctx.getSymbol(varName).getExpr();
  const ir::Type fieldType = getFieldType(setExpr, fieldName);

  // Check that set field is a scalar or vector.
  if (fieldType.toTensor()->order() > 1) {
    const auto msg = "cannot read from non-scalar and non-vector set fields";
    reportError(msg, expr);
    return;
  }

  retType = std::make_shared<Expr::Type>();
  retType->push_back(fieldType);
}

void TypeChecker::visit(VarExpr::Ptr expr) {
  // Check that variable has been declared.
  if (!ctx.hasSymbol(expr->ident)) {
    if (!skipCheckDeclared) {
      reportUndeclared("variable or constant", expr->ident, expr);
    }

    expr->access = internal::Symbol::ReadWrite;
    return;
  }
  
  const internal::Symbol varSym = ctx.getSymbol(expr->ident);
  const ir::Type varType = varSym.getExpr().type();
  expr->access = varSym.getAccess();

  if (!varType.defined()) {
    return;
  }

  retType = std::make_shared<Expr::Type>();
  retType->push_back(varType);
}

void TypeChecker::visit(IntLiteral::Ptr lit) {
  retType = std::make_shared<Expr::Type>();
  retType->push_back(ir::Int);
}

void TypeChecker::visit(FloatLiteral::Ptr lit) {
  retType = std::make_shared<Expr::Type>();
  retType->push_back(ir::Float);
}

void TypeChecker::visit(BoolLiteral::Ptr lit) {
  retType = std::make_shared<Expr::Type>();
  retType->push_back(ir::Boolean);
}

void TypeChecker::visit(ComplexLiteral::Ptr lit) {
  retType = std::make_shared<Expr::Type>();
  retType->push_back(ir::Complex);
}

void TypeChecker::visit(StringLiteral::Ptr lit) {
  retType = std::make_shared<Expr::Type>();
  retType->push_back(ir::String);
}

void TypeChecker::visit(IntVectorLiteral::Ptr lit) {
  typeCheckDenseTensorLiteral(lit);
}

void TypeChecker::visit(FloatVectorLiteral::Ptr lit) {
  typeCheckDenseTensorLiteral(lit);
}

void TypeChecker::visit(ComplexVectorLiteral::Ptr lit) {
  typeCheckDenseTensorLiteral(lit);
}

void TypeChecker::visit(NDTensorLiteral::Ptr lit) {
  typeCheckDenseTensorLiteral(lit);
}

void TypeChecker::visit(ApplyStmt::Ptr stmt) {
  typeCheckMapOrApply(stmt->map, true);
}

void TypeChecker::typeCheckDenseTensorLiteral(DenseTensorLiteral::Ptr lit) {
  try {
    const DenseTensorType tensorType = getDenseTensorType(lit);
    const std::vector<ir::IndexDomain> idoms(tensorType.dimSizes.rbegin(), 
                                             tensorType.dimSizes.rend());
    ir::ScalarType::Kind elemType;
    switch (tensorType.type) {
      case DenseTensorType::Type::INT:
        elemType = ir::ScalarType::Int;
        break;
      case DenseTensorType::Type::FLOAT:
        elemType = ir::ScalarType::Float;
        break;
      case DenseTensorType::Type::COMPLEX:
        elemType = ir::ScalarType::Complex;
        break;
      default:
        elemType = ir::ScalarType::Float;
    }
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
  } else if (isa<ComplexVectorLiteral>(lit)) {
    tensorType.addComplexValues(to<ComplexVectorLiteral>(lit)->vals.size());
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

void TypeChecker::typeCheckVarOrConstDecl(VarDecl::Ptr decl, bool isConst, 
                                          bool isGlobal) {
  const std::string varName = decl->name->ident;
  const std::string varDeclType = isConst ? "constant" : "variable";
  
  ir::Type varType;
  if (decl->type) {
    varType = getIRType(decl->type);
  }
  
  Ptr<Expr::Type> initType;
  if (decl->initVal) {
    initType = inferType(decl->initVal);

    // Infer variable type from initial value if not explicitly specified.
    if (!decl->type && initType && initType->size() == 1) {
      varType = initType->at(0);
    }
  }

  // Check that variable/constant hasn't already been declared in current scope.
  if (ctx.hasSymbol(varName, true) && 
      ctx.getSymbol(varName).getExpr().type().defined()) {
    reportMultipleDefs(varDeclType, varName, decl);
    return;
  }
  
  // Record declaration of variable/constant in symbol table.
  const auto access = isConst ? internal::Symbol::Read : 
                      internal::Symbol::ReadWrite;
  const auto var = ir::Var(varName, varType);
  ctx.addSymbol(varName, var, access);

  // If type signature is invalid or type signature is unspecified and we 
  // cannot infer type, then we are done. 
  if (!varType.defined()) {
    return;
  }

  // Check that variable is a tensor.
  if (!varType.isTensor()) {
    std::stringstream errMsg;
    errMsg << "cannot declare a non-tensor " << varDeclType;
    reportError(errMsg.str(), decl);
    return;
  }

  // Check that initial value type matches declared variable/constant type.
  // If this check completes successfully, then we are done. 
  if (!initType || (initType->size() == 1 && 
      compareTypes(varType, initType->at(0)))) {
    return;
  }

  std::stringstream errMsg;
  errMsg << "cannot initialize a " << varDeclType << " of type "
         << typeString(var.getType()) << " with an expression of type "
         << typeString(initType);

  // Check that initial value is of tensor type. If it is not, then there must 
  // be an error since we require variables to be tensors (at least for the 
  // time being). Otherwise, proceed to check legal special cases.
  if (initType->size() != 1 || !initType->at(0).isTensor()) {
    reportError(errMsg.str(), decl);
    return;
  }

  const ir::Type initIRType = initType->at(0);
  const ir::TensorType *varTensorType = varType.toTensor();
  const ir::TensorType *initTensorType = initIRType.toTensor();
  const ir::ScalarType varComponentType = varTensorType->getComponentType();
  const ir::ScalarType initComponentType = initTensorType->getComponentType();
  
  // Check if attempting to initialize a local tensor with a scalar.
  if (isScalar(initIRType) && varComponentType == initComponentType) {
    // TODO: It might be useful to be able to initialize non-scalar global 
    //       tensors by scalar values. We prohibit this for now because it 
    //       is not supported by the backend.
    if (isGlobal) {
      const auto msg = "cannot initialize a non-scalar global constant "
                       "by a scalar value";
      reportError(msg, decl);
    }
    return;
  }

  const ir::Type varBlockType = varTensorType->getBlockType();
  const ir::Type initBlockType = initTensorType->getBlockType();
  
  // Check if initial value type is equivalent to declared constant type.
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

  // Initialization must be illegal, so report error.
  reportError(errMsg.str(), decl);
}

void TypeChecker::typeCheckMapOrApply(MapExpr::Ptr expr, const bool isApply) {
  const std::string opString = isApply ? "apply" : "map";

  std::vector<ir::Type> actualsType(expr->partialActuals.size());
  for (unsigned i = 0; i < expr->partialActuals.size(); ++i) {
    const Expr::Ptr param = expr->partialActuals[i];
    const Ptr<Expr::Type> paramType = inferType(param);

    if (!paramType) {
      continue;
    }

    // Check that argument is a single non-void value.
    if (paramType->size() == 0) {
      reportError("must pass a non-void value as argument", param);
    } else if (paramType->size() != 1) {
      std::stringstream errMsg;
      errMsg << "cannot pass multiple values of types " 
             << typeString(paramType) << " as a single argument";
      reportError(errMsg.str(), param);
    } else {
      actualsType[i] = paramType->at(0);
    }
  }
  
  const std::string funcName = expr->func->ident;
  const std::string targetName = expr->target->ident;
  
  ir::Func func;
  ir::Expr target;
  bool typeChecked = true;

  // Check that assembly function has been declared.
  if (!ctx.containsFunction(funcName)) {
    reportUndeclared("function", funcName, expr->func);
    typeChecked = false;
  } else {
    func = ctx.getFunction(funcName);
    
    if (isApply && func.getResults().size() > 0) {
      reportError("cannot apply a non-void function", expr->func);
      typeChecked = false;
    }

    // Infer map operation return type.
    retType = std::make_shared<Expr::Type>();
    for (const auto &res : func.getResults()) {
      retType->push_back(res.getType());
    }
  }

  // Check that target set has been declared.
  if (!ctx.hasSymbol(targetName)) {
    reportUndeclared("set", targetName, expr->target);
    typeChecked = false;
  } else {
    target = ctx.getSymbol(expr->target->ident).getExpr();

    // Check that map operation is applied to set.
    if (!target.type().isSet()) {
      std::stringstream errMsg;
      errMsg << opString << " operation can only be applied to sets";
      reportError(errMsg.str(), expr->target);
      typeChecked = false;
    }
  }

  if (!typeChecked) {
    return;
  }

  // Infer assembly function's required argument types.
  const ir::SetType *targetSetType = target.type().toSet();
  actualsType.push_back(targetSetType->elementType);
  
  const auto funcArgs = func.getArguments();
  
  if (targetSetType->endpointSets.size() > 0) {
    const auto neighborSetType = 
        targetSetType->endpointSets[0]->type().toSet();
    
    // Check for heterogeneous edge sets.
    for (unsigned i = 1; i < targetSetType->endpointSets.size(); ++i) {
      if (targetSetType->endpointSets[i]->type().toSet() != neighborSetType) {
        std::stringstream errMsg;
        errMsg << opString << " operation is currently unsupported for "
               << "heterogeneous edge sets";
        reportError(errMsg.str(), expr->target);
        return;
      }
    }
    
    if (actualsType.size() != funcArgs.size()) {
      // TODO: Should eventually support heterogeneous edge sets.
      const auto neighborsType = ir::TupleType::make(
          neighborSetType->elementType, targetSetType->endpointSets.size());
      actualsType.push_back(neighborsType);
    }
  }
 
  // Check that assembly function accepts right number of arguments.
  if (actualsType.size() != funcArgs.size()) {
    std::stringstream errMsg;
    errMsg << opString << " operation passes " << actualsType.size() 
           << " arguments to assembly function but function '" << func.getName()
           << "' expects " << funcArgs.size() << " arguments";
    reportError(errMsg.str(), expr);
    return;
  }

  for (unsigned i = 0; i < actualsType.size(); ++i) {
    if (!actualsType[i].defined() || !funcArgs[i].getType().defined()) {
      continue;
    }
    
    // Check that type of argument that will be passed to assembly function 
    // is type expected by function.
    if (!compareTypes(actualsType[i], funcArgs[i].getType())) {
      std::stringstream errMsg;
      errMsg << opString << " operation passes argument of type "
             << typeString(actualsType[i]) << " to assembly function but "
             << "function '" << func.getName() << "' expects argument of "
             << "type " << typeString(funcArgs[i].getType());
      if (i < expr->partialActuals.size()) {
        reportError(errMsg.str(), expr->partialActuals[i]);
      } else {
        reportError(errMsg.str(), expr->target);
      }
    }
    
    if (i < expr->partialActuals.size()) { 
      const Expr::Ptr param = expr->partialActuals[i];

      // Check that inout argument is writable.
      if (writableArgs.find(funcArgs[i]) != writableArgs.end() && 
          !param->isWritable()) {
        reportError("inout argument must be writable", param);
      }

      // Check that additional argument is a tensor.
      // TODO: Remove once support for passing non-tensor arguments is added.
      if (!actualsType[i].isTensor()) {
        std::stringstream errMsg;
        errMsg << "expected argument to be a tensor but got an argument "
               << "of type " << typeString(actualsType[i]);
        reportError(errMsg.str(), param);
      }
    }
  }
}

void TypeChecker::typeCheckBinaryElwise(BinaryExpr::Ptr expr, 
                                        bool allowStringOperands) {
  const Ptr<Expr::Type> lhsType = inferType(expr->lhs);
  const Ptr<Expr::Type> rhsType = inferType(expr->rhs);
  bool typeChecked = (bool)lhsType && (bool)rhsType;

  // Check that operands of element-wise operation are numeric tensors.
  if (lhsType && (lhsType->size() != 1 || !lhsType->at(0).isTensor() || 
      (!lhsType->at(0).toTensor()->getComponentType().isNumeric() && 
      (!allowStringOperands || !isString(lhsType->at(0)))))) {
    std::stringstream errMsg;
    errMsg << "expected left operand of element-wise operation to be a "
           << "numeric tensor " << (allowStringOperands ? "or string " : "")
           << "but got an operand of type " << typeString(lhsType);
    reportError(errMsg.str(), expr->lhs);
    typeChecked = false;
  }
  if (rhsType && (rhsType->size() != 1 || !rhsType->at(0).isTensor() || 
      (!rhsType->at(0).toTensor()->getComponentType().isNumeric() && 
      (!allowStringOperands || !isString(rhsType->at(0)))))) {
    std::stringstream errMsg;
    errMsg << "expected right operand of element-wise operation to be a "
           << "numeric tensor " << (allowStringOperands ? "or string " : "")
           << "but got an operand of type " << typeString(rhsType);
    reportError(errMsg.str(), expr->rhs);
    typeChecked = false;
  }
 
  if (!typeChecked) {
    return;
  }

  const ir::TensorType *ltype = lhsType->at(0).toTensor();
  const ir::TensorType *rtype = rhsType->at(0).toTensor();
  const auto lComponentType = ltype->getComponentType();
  const auto rComponentType = rtype->getComponentType();
  const bool hasScalarOperand = (ltype->order() == 0 || rtype->order() == 0);

  // Check that operands are compatible (i.e. contain elements of same type 
  // if one operand is scalar, or also have same dimensions otherwise).
  if (hasScalarOperand ? (lComponentType != rComponentType) : 
      !compareTypes(lhsType->at(0), rhsType->at(0))) {
    std::stringstream errMsg;
    errMsg << "cannot perform element-wise operation on tensors of type "
           << typeString(lhsType) << " and type " << typeString(rhsType);
    reportError(errMsg.str(), expr);
    return;
  }
  
  retType = (ltype->order() > 0) ? lhsType : rhsType;
}

void TypeChecker::typeCheckBinaryBoolean(BinaryExpr::Ptr expr) {
  const Ptr<Expr::Type> lhsType = inferType(expr->lhs);
  const Ptr<Expr::Type> rhsType = inferType(expr->rhs);

  // Check that operands of boolean operation are of boolean type.
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
  retType->push_back(ir::Boolean);
}

void TypeChecker::DenseTensorType::addIntValues(unsigned len) {
  if (type != Type::INT && type != Type::UNKNOWN) {
    throw TypeError();
  }

  type = Type::INT;
  dimSizes[dimSizes.size() - 1] += len;
}

void TypeChecker::DenseTensorType::addFloatValues(unsigned len) {
  if (type != Type::FLOAT && type != Type::UNKNOWN) {
    throw TypeError();
  }

  type = Type::FLOAT;
  dimSizes[dimSizes.size() - 1] += len;
}

void TypeChecker::DenseTensorType::addComplexValues(unsigned len) {
  if (type != Type::COMPLEX && type != Type::UNKNOWN) {
    throw TypeError();
  }

  type = Type::COMPLEX;
  dimSizes[dimSizes.size() - 1] += len;
}

void TypeChecker::DenseTensorType::merge(const DenseTensorType &other) {
  if (type != other.type) {
    throw TypeError();
  }
  
  if (dimSizes.size() - 1 != other.dimSizes.size()) {
    throw DimError();
  }

  for (unsigned i = 0; i < dimSizes.size() - 1; ++i) {
    if (dimSizes[i] != other.dimSizes[i]) {
      throw DimError();
    }
  }
  
  ++dimSizes[dimSizes.size() - 1];
}

bool TypeChecker::compareTypes(const ir::Type &l, const ir::Type &r) {
  iassert(l.defined() && r.defined());

  if (l.isTensor() && r.isTensor()) {
    const auto ltype = l.toTensor();
    const auto rtype = r.toTensor();
    return (*ltype == *rtype && ltype->isColumnVector == rtype->isColumnVector);
  }

  return (l == r);
}

std::string TypeChecker::typeString(const ir::Type &type) {
  std::stringstream oss;
  oss << "'" << type << "'";
  return oss.str();
}

std::string TypeChecker::typeString(const Ptr<Expr::Type> &type) {
  if (type->size() == 0) {
    return "void";
  }

  std::stringstream oss;
  if (type->size() > 1) {
    oss << "(";
  }
  
  bool printDelimiter = false;
  for (const auto compType : *type) {
    if (printDelimiter) {
      oss << ", ";
    }
    oss << typeString(compType);
    printDelimiter = true;
  }

  if (type->size() > 1) {
    oss << ")";
  }
  return oss.str();
}

void TypeChecker::reportError(std::string msg, HIRNode::Ptr loc) {
  const auto err = ParseError(loc->getLineBegin(), loc->getColBegin(), 
                              loc->getLineEnd(), loc->getColEnd(), msg);
  errors->push_back(err);
}

void TypeChecker::reportUndeclared(std::string type, std::string ident,
                                   HIRNode::Ptr loc) {
  std::stringstream errMsg;
  errMsg << "undeclared " << type << " '" << ident << "'";
  reportError(errMsg.str(), loc);
}

void TypeChecker::reportMultipleDefs(std::string type, std::string ident, 
                                     HIRNode::Ptr loc) {
  std::stringstream errMsg;
  errMsg << "multiple definitions of " << type << " '" << ident << "'";
  reportError(errMsg.str(), loc);
}

}
}
