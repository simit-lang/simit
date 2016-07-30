#include <memory>
#include <vector>
#include <iostream>
#include <algorithm>
#include <exception>

#include "error.h"
#include "hir.h"
#include "intrinsics.h"
#include "type_checker.h"
#include "util/util.h"

namespace simit {
namespace hir {
  
TypeChecker::TypeChecker(std::vector<ParseError> *errors) :
    retTypeChecked(true),
    skipCheckDeclared(false),
    errors(errors) {
  const auto threeDim = std::make_shared<RangeIndexSet>();
  threeDim->range = 3;

  const auto threeByThreeTensorType = std::make_shared<NDTensorType>();
  threeByThreeTensorType->blockType = makeTensorType(ScalarType::Type::FLOAT);
  threeByThreeTensorType->indexSets = {threeDim, threeDim};
  
  // Add type signatures for intrinsic functions.
  addScalarIntrinsic(ir::intrinsics::mod().getName(), 
                     {ScalarType::Type::INT, ScalarType::Type::INT}, 
                     {ScalarType::Type::INT});
  addScalarIntrinsic(ir::intrinsics::sin().getName(), 
                     {ScalarType::Type::FLOAT}, 
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(ir::intrinsics::cos().getName(), 
                     {ScalarType::Type::FLOAT}, 
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(ir::intrinsics::tan().getName(), 
                     {ScalarType::Type::FLOAT}, 
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(ir::intrinsics::asin().getName(), 
                     {ScalarType::Type::FLOAT}, 
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(ir::intrinsics::acos().getName(), 
                     {ScalarType::Type::FLOAT}, 
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(ir::intrinsics::atan2().getName(), 
                     {ScalarType::Type::FLOAT, ScalarType::Type::FLOAT}, 
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(ir::intrinsics::sqrt().getName(), 
                     {ScalarType::Type::FLOAT}, 
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(ir::intrinsics::log().getName(), 
                     {ScalarType::Type::FLOAT}, 
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(ir::intrinsics::exp().getName(), 
                     {ScalarType::Type::FLOAT}, 
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(ir::intrinsics::pow().getName(), 
                     {ScalarType::Type::FLOAT, ScalarType::Type::FLOAT}, 
                     {ScalarType::Type::FLOAT});
  addIntrinsic(ir::intrinsics::norm().getName(), 
               {Type::Ptr()}, 
               {makeTensorType(ScalarType::Type::FLOAT)});
  addIntrinsic(ir::intrinsics::dot().getName(), 
               {Type::Ptr(), Type::Ptr()}, 
               {makeTensorType(ScalarType::Type::FLOAT)});
  addIntrinsic(ir::intrinsics::det().getName(), 
               {threeByThreeTensorType}, 
               {makeTensorType(ScalarType::Type::FLOAT)});
  addIntrinsic(ir::intrinsics::inv().getName(), 
               {threeByThreeTensorType}, 
               {threeByThreeTensorType});
  addScalarIntrinsic(ir::intrinsics::createComplex().getName(), 
                     {ScalarType::Type::FLOAT, ScalarType::Type::FLOAT}, 
                     {ScalarType::Type::COMPLEX});
  addScalarIntrinsic(ir::intrinsics::complexNorm().getName(), 
                     {ScalarType::Type::COMPLEX}, 
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(ir::intrinsics::complexGetReal().getName(), 
                     {ScalarType::Type::COMPLEX}, 
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(ir::intrinsics::complexGetImag().getName(), 
                     {ScalarType::Type::COMPLEX}, 
                     {ScalarType::Type::FLOAT});
  addScalarIntrinsic(ir::intrinsics::complexConj().getName(), 
                     {ScalarType::Type::COMPLEX}, 
                     {ScalarType::Type::COMPLEX});
  addScalarIntrinsic(ir::intrinsics::clock().getName(), 
                     {}, {ScalarType::Type::FLOAT});
  addScalarIntrinsic(ir::intrinsics::storeTime().getName(), 
                     {ScalarType::Type::INT, ScalarType::Type::FLOAT}, 
                     {ScalarType::Type::FLOAT});
}

void TypeChecker::check(Program::Ptr program) {
  ComputeSetDefinitions(env).compute(program);
  typeCheck(program);
}

void TypeChecker::visit(Program::Ptr program) {
  for (auto elem : program->elems) {
    if (isa<ConstDecl>(elem)) {
      const auto decl = to<ConstDecl>(elem);
      typeCheckGlobalConstDecl(decl);
    } else {
      typeCheck(elem);
    }
  }
}

void TypeChecker::visit(StmtBlock::Ptr stmtBlock) {
  for (auto stmt : stmtBlock->stmts) {
    typeCheck(stmt);
  }
}

void TypeChecker::visit(SetIndexSet::Ptr set) {
  // Check that index set has been previously declared.
  if (!env.hasSymbol(set->setName)) {
    reportUndeclared("set", set->setName, set);
    return;
  }

  const Type::Ptr type = env.getSymbolType(set->setName);

  // Check that index set pointed to by identifier is indeed of set type.
  if (!set->setDef && !isa<SetType>(type)) {
    std::stringstream errMsg;
    errMsg << "expected a set but '" << set->setName 
           << "' is of type " << toString(type);
    reportError(errMsg.str(), set);
  }
}

void TypeChecker::visit(ElementType::Ptr type) {
  // Check that element type has been previously declared.
  if (!env.hasElementType(type->ident)) {
    reportUndeclared("element type", type->ident, type);
    return;
  }

  // If source set inference was able to infer element source, check that the 
  // source set actually contains element of the same type.
  if (type->source && !isa<GenericIndexSet>(type->source) && 
      env.hasSetDefinition(type->source)) {
    const SetType::Ptr sourceType = env.getSetDefinition(type->source);
    const std::string sourceElemType = sourceType->element->ident;

    if (sourceElemType != type->ident) {
      std::stringstream errMsg;
      errMsg << "element declared to be of type '" << type->ident 
             << "' but inferred to be of type '" << sourceElemType << "'";
      reportError(errMsg.str(), type);
    }
  }
}

void TypeChecker::visit(Endpoint::Ptr end) {
  retTypeChecked = typeCheck(end->set);
 
  if (retTypeChecked) {
    const auto endSetType = env.getSetDefinition(end->set);
    end->element = endSetType->element;
  }
}

void TypeChecker::visit(UnstructuredSetType::Ptr type) {
  retTypeChecked = typeCheck(type->element);

  for (auto end : type->endpoints) {
    const bool typeChecked = typeCheck(end);
    retTypeChecked = typeChecked && retTypeChecked;
  }
}

void TypeChecker::visit(LatticeLinkSetType::Ptr type) {
  retTypeChecked = typeCheck(type->element);
  
  retTypeChecked = typeCheck(type->latticePointSet) && retTypeChecked;
  // Check lattice point set is an unstructured set
  if (!isa<UnstructuredSetType>(type->latticePointSet->set->setDef)) {
    std::stringstream errMsg;
    errMsg << "expected lattice point set of Unstructured kind but got "
           << type->latticePointSet->set->setName;
    reportError(errMsg.str(), type->latticePointSet);
  }
  // Check lattice point set is cardinality zero
  else if (to<UnstructuredSetType>(type->latticePointSet->set->setDef)
           ->endpoints.size() != 0) {
    std::stringstream errMsg;
    errMsg << "expected lattice point set of 0 cardinality, but got "
           << to<UnstructuredSetType>(type->latticePointSet->set->setDef)
        ->endpoints.size();
    reportError(errMsg.str(), type->latticePointSet);
  }
}

void TypeChecker::visit(TupleType::Ptr type) {
  retTypeChecked = typeCheck(type->element);

  // Check that tuple length is positive.
  if (type->length->val < 1) {
    const auto msg = "tuple must have length greater than or equal to one";
    reportError(msg, type->length);
  }
}

void TypeChecker::visit(NDTensorType::Ptr type) {
  retTypeChecked = typeCheck(type->blockType);

  for (auto indexSet : type->indexSets) {
    retTypeChecked = typeCheck(indexSet) && retTypeChecked;
  }

  if (isa<NDTensorType>(type->blockType) && type->indexSets.size() != 
      to<NDTensorType>(type->blockType)->indexSets.size()) {
    const auto msg = "blocked tensor type must contain same number "
                     "of dimensions as its blocks";
    reportError(msg, type);
  }

  if (type->transposed && type->indexSets.size() != 1) {
    reportError("transposed type must be a vector", type);
  }
}

void TypeChecker::visit(IdentDecl::Ptr decl) {
  retTypeChecked = typeCheck(decl->type);
}

void TypeChecker::visit(ElementTypeDecl::Ptr decl) {
  Environment::ElementMap elemFields;
  for (auto field : decl->fields) {
    const std::string fieldName = field->name->ident;
    const Type::Ptr fieldType = field->type;

    const bool typeChecked = typeCheck(field);

    if (elemFields.find(fieldName) != elemFields.end()) {
      reportMultipleDefs("element field", fieldName, field);
    }

    elemFields[fieldName] = typeChecked ? fieldType : Type::Ptr();
  }

  // Check that element type has not been previously declared.
  if (env.hasElementType(decl->name->ident)) {
    reportMultipleDefs("element type", decl->name->ident, decl);
  }

  env.addElementType(decl->name->ident, elemFields);
}

void TypeChecker::visit(ExternDecl::Ptr decl) {
  retTypeChecked = typeCheck(decl->type);

  const auto externName = decl->name->ident;
  const auto externType = retTypeChecked ? decl->type : Type::Ptr();

  // Check that variable has not been previously declared.
  if (env.hasSymbol(externName, Environment::Scope::CurrentOnly)) {
    reportMultipleDefs("variable or constant", externName, decl);
  }
 
  env.addSymbol(externName, externType, Access::READ_WRITE);
}

void TypeChecker::visit(FuncDecl::Ptr decl) {
  const std::string name = decl->name->ident;
  const bool specialized = !decl->originalName.empty();

  // Skip type checking for specialized versions of generic functions.
  if (specialized && !decl->genericParams.empty()) {
    if (env.hasFunction(decl->originalName)) {
      env.addFunction(name, decl);
    }

    return;
  }

  if (decl->type == FuncDecl::Type::EXPORTED && !decl->genericParams.empty()) {
    reportError("exported function cannot have generic parameters", decl);
  }

  env.scope();

  for (const auto genericParam : decl->genericParams) {
    const std::string name = genericParam->name;

    if (env.hasSymbol(name, Environment::Scope::CurrentOnly)) {
      reportMultipleDefs("function parameter", name, genericParam);
    }

    const Type::Ptr type = (genericParam->type == GenericParam::Type::RANGE) ?
                           to<Type>(makeTensorType(ScalarType::Type::INT)) :
                           to<Type>(env.getSetDefinition(genericParam));
    env.addSymbol(name, type, Access::READ);
  }

  // Check argument types.
  for (const auto arg : decl->args) {
    const bool typeChecked = typeCheck(arg);
    retTypeChecked = typeChecked && retTypeChecked;

    const std::string name = arg->name->ident;
    const Type::Ptr type = typeChecked ? arg->type : Type::Ptr();
    const Access access = arg->isInOut() ? Access::READ_WRITE : Access::READ;
    
    if (env.hasSymbol(name, Environment::Scope::CurrentOnly)) {
      reportMultipleDefs("function parameter", name, arg);
    }

    env.addSymbol(name, type, access);
  }
  
  // Check return value types.
  for (const auto res : decl->results) {
    const bool typeChecked = typeCheck(res);
    retTypeChecked = typeChecked && retTypeChecked;

    const std::string name = res->name->ident;
    const Type::Ptr type = typeChecked ? res->type : Type::Ptr();
    
    if (env.hasSymbol(name, Environment::Scope::CurrentOnly)) {
      reportMultipleDefs("function parameter", name, res);
    }

    env.addSymbol(name, type, Access::READ_WRITE);
  }

  // Type check function body.
  if (decl->body) {
    typeCheck(decl->body);
  }

  env.unscope();

  if (!retTypeChecked) {
    return;
  }

  // Check that function has not been previously declared.
  if (env.hasFunction(name) && !specialized) {
    reportMultipleDefs("function or procedure", name, decl);
  }

  env.addFunction(name, decl);
}

void TypeChecker::visit(VarDecl::Ptr decl) {
  typeCheckVarOrConstDecl(decl);
}

void TypeChecker::visit(ConstDecl::Ptr decl) {
  typeCheckVarOrConstDecl(decl, true);
}

void TypeChecker::visit(WhileStmt::Ptr stmt) {
  const ExprType condType = inferType(stmt->cond);
 
  env.scope();
  typeCheck(stmt->body);
  env.unscope();

  // Check that conditional expression is boolean.
  if (condType.defined && !condType.isScalarBoolean()) {
    std::stringstream errMsg;
    errMsg << "expected a boolean conditional expression but got an "
           << "expression of type " << toString(condType);
    reportError(errMsg.str(), stmt->cond);
  }
}

void TypeChecker::visit(IfStmt::Ptr stmt) {
  const ExprType condType = inferType(stmt->cond);
  
  env.scope();
  typeCheck(stmt->ifBody);
  env.unscope();

  if (stmt->elseBody) {
    env.scope();
    typeCheck(stmt->elseBody);
    env.unscope();
  }

  // Check that conditional expression is boolean.
  if (condType.defined && !condType.isScalarBoolean()) {
    std::stringstream errMsg;
    errMsg << "expected a boolean conditional expression but got an "
           << "expression of type " << toString(condType);
    reportError(errMsg.str(), stmt->cond);
  }
}

void TypeChecker::visit(IndexSetDomain::Ptr domain) {
  retTypeChecked = typeCheck(domain->set);
}

void TypeChecker::visit(RangeDomain::Ptr domain) {
  const ExprType lowerType = inferType(domain->lower);
  const ExprType upperType = inferType(domain->upper);

  // Check that lower and upper bounds of for-loop range are integral.
  if (lowerType.defined && !lowerType.isScalarInt()) {
    std::stringstream errMsg;
    errMsg << "expected lower bound of for-loop range to be integral but got "
           << "an expression of type " << toString(lowerType);
    reportError(errMsg.str(), domain->lower);
  }
  if (upperType.defined && !upperType.isScalarInt()) {
    std::stringstream errMsg;
    errMsg << "expected upper bound of for-loop range to be integral but got "
           << "an expression of type " << toString(upperType);
    reportError(errMsg.str(), domain->upper);
  }
}

void TypeChecker::visit(ForStmt::Ptr stmt) {
  env.scope();
  
  const bool typeChecked = typeCheck(stmt->domain);
 
  Type::Ptr loopVarType;
  if (isa<RangeDomain>(stmt->domain)) {
    loopVarType = makeTensorType(ScalarType::Type::INT);
  } else if (isa<IndexSetDomain>(stmt->domain) && typeChecked) {
    const auto setDomain = to<IndexSetDomain>(stmt->domain);
    const auto setType = env.getSetDefinition(setDomain->set);
    loopVarType = setType->element;
  }

  env.addSymbol(stmt->loopVar->ident, loopVarType, Access::READ);
  typeCheck(stmt->body);

  env.unscope();
}

void TypeChecker::visit(PrintStmt::Ptr stmt) {
  for (const auto arg : stmt->args) {
    const ExprType argType = inferType(arg);

    // Check that print statement is printing a tensor.
    if (argType.defined && !argType.isTensor()) {
      std::stringstream errMsg;
      errMsg << "cannot print an expression of type " << toString(argType);
      reportError(errMsg.str(), arg);
    }
  }
}

void TypeChecker::visit(ExprStmt::Ptr stmt) {
  inferType(stmt->expr);
}

void TypeChecker::visit(AssignStmt::Ptr stmt) {
  const ExprType exprType = inferType(stmt->expr);
  retTypeChecked = exprType.defined;

  std::vector<ExprType> lhsTypes;
  for (auto lhs : stmt->lhs) {
    // If assigning directly to a variable, then the variable does not have to 
    // be declared beforehand, so skip that check later.
    skipCheckDeclared = isa<VarExpr>(lhs);
    lhsTypes.push_back(inferType(lhs));
    skipCheckDeclared = false;
  }

  // Check that number of values returned by expression on right-hand side
  // (may not equal to one if it is a map operation or function call) is equal 
  // to number of assignment targets.
  if (retTypeChecked && stmt->lhs.size() != exprType.type.size()) {
    std::stringstream errMsg;
    errMsg << "cannot assign an expression returning " << exprType.type.size()
           << " values to " << stmt->lhs.size() << " targets";
    reportError(errMsg.str(), stmt);
  }

  if (retTypeChecked) {
    for (unsigned i = 0; i < stmt->lhs.size(); ++i) {
      // Check that type of value returned by expression on right-hand side 
      // is assignable and corresponds to type of target on left-hand side.
      if (lhsTypes[i].defined) {
        if (lhsTypes[i].isVoid()) {
          reportError("cannot assign to a void target", stmt->lhs[i]);
        } else if (!lhsTypes[i].isSingleValue()) {
          const auto msg = "cannot assign a single value to multiple targets";
          reportError(msg, stmt->lhs[i]);
        } else if (!env.compareTypes(lhsTypes[i].type[0], exprType.type[i])) {
          const bool rhsIsScalar = isa<ScalarType>(exprType.type[i]);
          const bool lhsIsTensor = isa<TensorType>(lhsTypes[i].type[0]);
          
          // Check for assignment of a scalar to a non-scalar tensor.
          bool invalidAssign = !rhsIsScalar || !lhsIsTensor;

          if (!invalidAssign) {
            const auto rhsScalarType = to<ScalarType>(exprType.type[i]);
            const auto lhsTensorType = to<TensorType>(lhsTypes[i].type[0]);

            if (rhsScalarType->type != getComponentType(lhsTensorType)) {
              invalidAssign = true;
            }
          }

          if (invalidAssign) {
            std::stringstream errMsg;
            errMsg << "cannot assign a value of type " 
                   << toString(exprType.type[i]) << " to a target of type " 
                   << toString(lhsTypes[i]);
            reportError(errMsg.str(), stmt->lhs[i]);
          }
        }
      }

      // Check that expression on right-hand side returns only tensors.
      // TODO: Remove once support for assignment of non-tensors is added.
      if (!isa<TensorType>(exprType.type[i])) {
        std::stringstream errMsg;
        errMsg << "cannot assign a non-tensor value of type "
               << toString(exprType.type[i]) << " to a variable";
        reportError(errMsg.str(), stmt->expr);
      }

      // Check that target is writable.
      if (!lhsTypes[i].isWritable()) {
        reportError("assignment target is not writable", stmt->lhs[i]);
      }
    }
  }
  
  for (unsigned i = 0; i < stmt->lhs.size(); ++i) {
    // Mark target variable as having been declared if necessary.
    if (isa<VarExpr>(stmt->lhs[i])) {
      const std::string varName = to<VarExpr>(stmt->lhs[i])->ident;
      if (!env.hasSymbol(varName)) {
        const auto varType = exprType.defined ? exprType.type[i] : Type::Ptr();
        env.addSymbol(varName, varType, Access::READ);
      }
    }
  }
}

void TypeChecker::visit(ExprParam::Ptr expr) {
  retType = inferType(expr->expr);
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
  Type::Ptr repType;
  for (const auto operand : expr->operands) {
    const ExprType opndType = inferType(operand);
    
    if (!opndType.defined) {
      continue;
    }
    
    // Check that comparison operation is performed on scalar values.
    if (!opndType.isScalar()) {
      std::stringstream errMsg;
      errMsg << "comparison operations can only be performed on scalar "
             << "values, not values of type " << toString(opndType);
      reportError(errMsg.str(), operand);
      continue;
    }

    // Check that operands of comparison operation are of the same type.
    if (!repType) {
      repType = opndType.type[0];
    } else if (!env.compareTypes(repType, opndType.type[0])) {
      std::stringstream errMsg;
      errMsg << "value of type " << toString(opndType)
             << "cannot be compared to value of type " << toString(repType);
      reportError(errMsg.str(), operand);
    }
  }
 
  retType = ExprType(makeTensorType(ScalarType::Type::BOOL));
}

void TypeChecker::visit(NotExpr::Ptr expr) {
  const ExprType opndType = inferType(expr->operand);

  // Check that operand of boolean not is boolean.
  if (opndType.defined && !opndType.isScalarBoolean()) {
    std::stringstream errMsg;
    errMsg << "expected a boolean operand but got an operand of type "
           << toString(opndType);
    reportError(errMsg.str(), expr->operand);
  }

  retType = ExprType(makeTensorType(ScalarType::Type::BOOL));
}

void TypeChecker::visit(AddExpr::Ptr expr) {
  typeCheckBinaryElwise(expr, true); 
}

void TypeChecker::visit(SubExpr::Ptr expr) {
  typeCheckBinaryElwise(expr); 
}

void TypeChecker::visit(MulExpr::Ptr expr) {
  const ExprType lhsType = inferType(expr->lhs);
  const ExprType rhsType = inferType(expr->rhs);
  retTypeChecked = lhsType.defined && rhsType.defined;

  // Check that operands of multiplication operation are numeric tensors.
  if (lhsType.defined && !lhsType.isNumericTensor()) {
    std::stringstream errMsg;
    errMsg << "expected left operand of multiplication operation to be a "
           << "numeric tensor but got an operand of type " << toString(lhsType);
    reportError(errMsg.str(), expr->lhs);
  }
  if (rhsType.defined && !rhsType.isNumericTensor()) {
    std::stringstream errMsg;
    errMsg << "expected right operand of multiplication operation to be a "
           << "numeric tensor but got an operand of type " << toString(rhsType);
    reportError(errMsg.str(), expr->rhs);
  }

  if (!retTypeChecked) {
    return;
  }

  const auto ltype = to<TensorType>(lhsType.type[0]);
  const auto rtype = to<TensorType>(rhsType.type[0]);
  const ScalarType::Type lhsComponent = getComponentType(ltype);
  const ScalarType::Type rhsComponent = getComponentType(rtype);

  // Check that operands of multiply contain elements of the same type.
  if (lhsComponent != rhsComponent) {
    std::stringstream errMsg;
    errMsg << "cannot multiply a " << toString(lhsComponent) << " tensor by a "
           << toString(rhsComponent) << " tensor";
    reportError(errMsg.str(), expr);
    return;
  }
  
  const TensorDimensions ldimensions = getDimensions(ltype);
  const TensorDimensions rdimensions = getDimensions(rtype);
  const unsigned lhsOrder = ldimensions.size();
  const unsigned rhsOrder = rdimensions.size();
  const bool lhsTransposed = getTransposed(ltype);
  const bool rhsTransposed = getTransposed(rtype);

  if (lhsOrder == 0 || rhsOrder == 0) {
    retType = ExprType(lhsOrder > 0 ? ltype : rtype); 
  } else if (lhsOrder == 1 && rhsOrder == 1) {
    // Check dimensions of operands for vector-vector multiplication.
    if (lhsTransposed && rhsTransposed) {
      reportError("cannot multiply two row vectors", expr);
      return;
    } else if (!lhsTransposed && !rhsTransposed) {
      reportError("cannot multiply two column vectors", expr);
      return;
    } else if (!env.compareDomains(ldimensions[0], rdimensions[0])) {
      std::stringstream errMsg;
      errMsg << "cannot multiply a vector of type " << toString(ltype) 
             << " by a vector of type " << toString(rtype);
      reportError(errMsg.str(), expr);
      return;
    }
    
    TensorDimensions newDimensions;
    if (!lhsTransposed) {
      newDimensions.push_back(ldimensions[0]);
      newDimensions.push_back(rdimensions[0]);
    }

    retType = ExprType(makeTensorType(lhsComponent, newDimensions));
  } else if (lhsOrder == 2 && rhsOrder == 1) {
    // Check dimensions of operands for matrix-vector multiplication.
    if (!env.compareDomains(ldimensions[1], rdimensions[0])) {
      std::stringstream errMsg;
      errMsg << "cannot multiply a matrix of type " << toString(ltype)
             << " by a vector of type " << toString(rtype);
      reportError(errMsg.str(), expr);
      return;
    } else if (rhsTransposed) {
      reportError("cannot multiply a matrix by a row vector", expr);
    }
   
    retType = ExprType(makeTensorType(lhsComponent, {ldimensions[0]}));
  } else if (lhsOrder == 1 && rhsOrder == 2) {
    // Check dimensions of operands for vector-matrix multiplication.
    if (!env.compareDomains(ldimensions[0], rdimensions[0])) { 
      std::stringstream errMsg;
      errMsg << "cannot multiply a vector of type " << toString(ltype)
             << " by a matrix of type " << toString(rtype);
      reportError(errMsg.str(), expr);
      return;
    } else if (!lhsTransposed) {
      reportError("cannot multiply a column vector by a matrix", expr);
    }

    retType = ExprType(makeTensorType(lhsComponent, {rdimensions[1]}, true));
  } else if (lhsOrder == 2 && rhsOrder == 2) {
    // Check dimensions of operands for matrix-matrix multiplication.
    if (!env.compareDomains(ldimensions[1], rdimensions[0])) {
      std::stringstream errMsg;
      errMsg << "cannot multiply a matrix of type " << toString(ltype)
             << " by a matrix of type " << toString(rtype);
      reportError(errMsg.str(), expr);
      return;
    }
   
    const TensorDimensions newDimensions = {ldimensions[0], rdimensions[1]};
    retType = ExprType(makeTensorType(lhsComponent, newDimensions));
  } else {
    reportError("cannot multiply tensors of order 3 or greater using *", expr);
  }
}

void TypeChecker::visit(DivExpr::Ptr expr) {
  const ExprType lhsType = inferType(expr->lhs);
  const ExprType rhsType = inferType(expr->rhs);
  retTypeChecked = lhsType.defined && rhsType.defined;
  
  // Check that operands of division operation are numeric tensors.
  if (lhsType.defined && !lhsType.isNumericTensor()) {
    std::stringstream errMsg;
    errMsg << "expected left operand of division operation to be a numeric "
           << "tensor but got an operand of type " << toString(lhsType);
    reportError(errMsg.str(), expr->lhs);
  }
  if (rhsType.defined && !rhsType.isNumericTensor()) {
    std::stringstream errMsg;
    errMsg << "expected right operand of division operation to be a numeric "
           << "tensor but got an operand of type " << toString(rhsType);
    reportError(errMsg.str(), expr->rhs);
  }

  if (!retTypeChecked) {
    return;
  }
  
  const auto ltype = to<TensorType>(lhsType.type[0]);
  const auto rtype = to<TensorType>(rhsType.type[0]);
  const ScalarType::Type lhsComponent = getComponentType(ltype);
  const ScalarType::Type rhsComponent = getComponentType(rtype);

  // Check that operands of division operation contain elements of same type.
  if (lhsComponent != rhsComponent) {
    std::stringstream errMsg;
    errMsg << "cannot divide a " << toString(lhsComponent) << " tensor by a "
           << toString(rhsComponent) << " tensor";
    reportError(errMsg.str(), expr);
    return;
  }
  
  const unsigned lhsOrder = getDimensions(ltype).size();
  const unsigned rhsOrder = getDimensions(rtype).size();

  // Check for unsupported division of two non-scalar tensors.
  // Probably want to remove this constraint at some point.
  if (lhsOrder > 0 && rhsOrder > 0) {
    std::stringstream errMsg;
    errMsg << "division of a non-scalar tensor of type " << toString(ltype) 
           << " by a non-scalar tensor of type " << toString(rtype)
           << " is not supported";
    reportError(errMsg.str(), expr);
    return;
  }
  
  retType = (lhsOrder > 0) ? ExprType(ltype) : ExprType(rtype);
}

void TypeChecker::visit(ElwiseMulExpr::Ptr expr) {
  typeCheckBinaryElwise(expr); 
}

void TypeChecker::visit(ElwiseDivExpr::Ptr expr) {
  typeCheckBinaryElwise(expr); 
}

void TypeChecker::visit(LeftDivExpr::Ptr expr) {
  const ExprType lhsType = inferType(expr->lhs);
  const ExprType rhsType = inferType(expr->rhs);
  retTypeChecked = lhsType.defined && rhsType.defined;

  // Check that operands of solve operation are numeric tensors.
  if (lhsType.defined && !lhsType.isNumericTensor()) {
    std::stringstream errMsg;
    errMsg << "expected left operand of solve operation to be a numeric "
           << "matrix but got an operand of type " << toString(lhsType);
    reportError(errMsg.str(), expr->lhs);
  }
  if (rhsType.defined && !rhsType.isNumericTensor()) {
    std::stringstream errMsg;
    errMsg << "expected right operand of solve operation to be a "
           << "numeric column vector but got an operand of type " 
           << toString(rhsType);
    reportError(errMsg.str(), expr->rhs);
  }

  if (!retTypeChecked) {
    return;
  }

  const auto ltype = to<TensorType>(lhsType.type[0]);
  const auto rtype = to<TensorType>(rhsType.type[0]);
  const ScalarType::Type lhsComponent = getComponentType(ltype);
  const ScalarType::Type rhsComponent = getComponentType(rtype);

  // Check that operands of solve operation contain elements of the same type.
  if (lhsComponent != rhsComponent) {
    std::stringstream errMsg;
    errMsg << "solve operation cannot be performed on a " 
           << toString(lhsComponent) << " tensor and a " 
           << toString(rhsComponent) << " tensor";
    reportError(errMsg.str(), expr);
  }
  
  const TensorDimensions ldimensions = getDimensions(ltype);
  const TensorDimensions rdimensions = getDimensions(rtype);
  const unsigned lhsOrder = ldimensions.size();
  const unsigned rhsOrder = rdimensions.size();
  const bool rhsTransposed = getTransposed(rtype);

  // Check that left operand is a matrix.
  if (lhsOrder != 2) {
    std::stringstream errMsg;
    errMsg << "expected left operand of solve operation to be a matrix "
           << "but got an operand of type " << toString(lhsType);
    reportError(errMsg.str(), expr->lhs);
  }

  // Check that right operand is a column vector.
  if (rhsOrder != 1 || rhsTransposed) {
    std::stringstream errMsg;
    errMsg << "expected right operand of solve operation to be a column "
           << "vector but got an operand of type " << toString(lhsType);
    reportError(errMsg.str(), expr->lhs);
  }
  
  if (!retTypeChecked) {
    return;
  }

  // Check dimensions of operands.
  if (!env.compareDomains(ldimensions[0], rdimensions[0])) {
    std::stringstream errMsg;
    errMsg << "solve operation cannot be performed on a matrix of type " 
           << toString(ltype) << " and a vector of type " << toString(rtype);
    reportError(errMsg.str(), expr);
    return;
  }
  
  retType = ExprType(makeTensorType(lhsComponent, {ldimensions[1]}));
}

void TypeChecker::visit(NegExpr::Ptr expr) {
  const ExprType opndType = inferType(expr->operand);

  if (!opndType.defined) {
    return;
  }

  // Check that operand of negation operation is a numeric tensor.
  if (!opndType.isNumericTensor()) {
    std::stringstream errMsg;
    errMsg << "expected operand of tensor negation to be a numeric tensor but "
           << "got an operand of type " << toString(opndType);
    reportError(errMsg.str(), expr->operand);
    return;
  }
  
  retType = ExprType(opndType.type[0]);
}

void TypeChecker::visit(ExpExpr::Ptr expr) {
  // TODO: Implement.
  not_supported_yet; 
}

void TypeChecker::visit(TransposeExpr::Ptr expr) {
  const ExprType opndType = inferType(expr->operand);

  if (!opndType.defined) {
    return;
  }
  
  const auto tensorType = to<TensorType>(opndType.type[0]);
  const ScalarType::Type componentType = getComponentType(tensorType);
  const TensorDimensions dimensions = getDimensions(tensorType);
  const unsigned order = dimensions.size();

  // Check that operand of transpose operation is tensor of order 2 or less.
  if (!opndType.isTensor() || order > 2) {
    std::stringstream errMsg;
    errMsg << "operand of tensor transpose must be a tensor of order 2 or "
           << "less, but got an operand of type " << toString(opndType);
    reportError(errMsg.str(), expr->operand);
    return;
  }

  switch (order) {
    case 0:
      retType = ExprType(opndType.type[0]);
      break;
    case 1:
    {
      const bool transposed = !getTransposed(tensorType);
      retType = ExprType(makeTensorType(componentType, dimensions, transposed));
      break;
    }
    case 2:
    {
      const TensorDimensions newDimensions = {dimensions[1], dimensions[0]};
      retType = ExprType(makeTensorType(componentType, newDimensions));
      break;
    }
    default:
      unreachable;
      break;
  }
}

void TypeChecker::visit(CallExpr::Ptr expr) {
  std::vector<ExprType> argTypes(expr->args.size());
  for (unsigned i = 0; i < expr->args.size(); ++i) {
    const Expr::Ptr arg = expr->args[i];

    if (arg) {
      argTypes[i] = inferType(arg);
    }
   
    if (!argTypes[i].defined) {
      continue;
    }

    // Check that argument is a single non-void value.
    if (argTypes[i].isVoid()) {
      reportError("must pass a non-void value as argument", arg);
      continue;
    } else if (!argTypes[i].isSingleValue()) {
      reportError("cannot pass multiple values as a single argument", arg);
      continue;
    }

    // Check that argument is a tensor.
    // TODO: Remove once support for passing non-tensor arguments is added.
    if (!argTypes[i].isTensor()) {
      std::stringstream errMsg;
      errMsg << "expected argument to be a tensor but got an argument "
             << "of type " << toString(argTypes[i]);
      reportError(errMsg.str(), arg);
      continue;
    }
  }
  
  std::string funcName = expr->func->ident;

  // If callee type signature could not be determined, 
  // then there's nothing more to do.
  if (!env.hasFunction(funcName)) {
    if (util::contains(ir::intrinsics::byNames(), funcName)) {
      std::stringstream errMsg;
      errMsg << "cannot call intrinsic function'" << funcName << "'";
      reportError(errMsg.str(), expr);
    }
 
    return;
  }

  FuncDecl::Ptr func = env.getFunction(funcName);

  if (!func->originalName.empty()) {
    funcName = func->originalName;
  }

  if (!func->genericParams.empty()) {
    // Type parameter resolution needs to be done with information that is 
    // stored only with the original version of the generic function (in 
    // particular, input element sources).
    const FuncDecl::Ptr funcSignature = func->originalName.empty() ? func :
                                        env.getFunction(func->originalName);

    GenericCallTypeChecker checker(env);
    const auto numArgs = std::min(argTypes.size(), funcSignature->args.size());

    for (unsigned i = 0; i < numArgs; ++i) {
      const ExprType argType = argTypes[i];
      if (argType.defined && argType.isSingleValue()) {
        checker.unify(funcSignature->args[i]->type, argType.type[0]);
      }
    }

    for (const auto genericParam : funcSignature->genericParams) {
      const auto it = checker.specializedSets.find(genericParam->name);
      if (it == checker.specializedSets.end()) {
        std::stringstream errMsg;
        errMsg << "unable to resolve type parameter '" << genericParam->name 
               << "' for call to function '" << funcName << "'";
        reportError(errMsg.str(), expr);
        return;
      }
    }

    if (func->originalName.empty()) { 
      func = func->clone<FuncDecl>();
      func->body = StmtBlock::Ptr();
    }

    ReplaceTypeParams(checker.specializedSets).rewrite(func);
    
    if (!func->originalName.empty()) {
      const std::string typeSig = getConcretizedTypeSignatureString(func);

      if (env.hasFunctionReplacement(typeSig)) {
        func = env.getFunctionReplacement(typeSig);
        expr->func->ident = func->name->ident;
      } else {
        env.addFunctionReplacement(typeSig, func);
        func->genericParams.clear();

        // Callee might itself call another generic function with arguments of 
        // generic types, in which case the generic function called by the 
        // callee would also have to be concretized.
        if (errors->empty()) {
          typeCheck(func);
        }
      }
    }
  }

  auto getResType = [](IdentDecl::Ptr res) { return res->type; };

  // Infer function call return type.
  ExprType::TypeVector resultTypes(func->results.size());
  std::transform(func->results.begin(), func->results.end(), 
                 resultTypes.begin(), getResType);
  retType = ExprType(resultTypes);

  if (expr->args.size() != func->args.size()) {
    std::stringstream errMsg;
    errMsg << "passed in " << expr->args.size() << " arguments but function '"
           << funcName << "' expects " << func->args.size();
    reportError(errMsg.str(), expr);
    return;
  }

  for (unsigned i = 0; i < expr->args.size(); ++i) {
    const Argument::Ptr funcArg = func->args[i];
    const Expr::Ptr arg = expr->args[i];
    const ExprType argType = argTypes[i];
    
    if (!argType.defined || !argType.isSingleValue()) {
      continue;
    }

    // Check that argument is of type expected by callee.
    if (funcName == ir::intrinsics::norm().getName() || 
        funcName == ir::intrinsics::dot().getName()) {
      // Check that argument is a numeric tensor of appropriate order.
      if (!argType.isNumericTensor() || 
          getOrder(to<TensorType>(argType.type[0])) != 1) {
        std::stringstream errMsg;
        errMsg << "expected a numeric vector as argument but got an "
               << "argument of type " << toString(argType);
        reportError(errMsg.str(), arg);
      }
    } else if (!env.compareTypes(argType.type[0], funcArg->type)) {
      std::stringstream errMsg;
      errMsg << "expected argument of type " << toString(funcArg->type) 
             << " but got an argument of type " << toString(argType);
      reportError(errMsg.str(), arg);
    }

    // Check that inout argument is writable.
    if (funcArg->isInOut() && !argType.isWritable()) {
      reportError("inout argument must be writable", arg);
    }
  }

  if (!retTypeChecked) {
    return;
  }

  // Handle some of the more complex generic functions as special cases.
  if (funcName == ir::intrinsics::dot().getName()) {
    // Check that first and second arguments are vectors of same length 
    // containing elements of same type.
    if (!env.compareTypes(argTypes[0].type[0], argTypes[1].type[0], true)) {
      std::stringstream errMsg;
      errMsg << "cannot call function 'dot' on arguments of type "
             << toString(argTypes[0]) << " and type " 
             << toString(argTypes[1]);
      reportError(errMsg.str(), expr);
    }
  }
}

void TypeChecker::visit(TensorReadExpr::Ptr expr) {
  const ExprType lhsType = inferType(expr->tensor);

  if (!lhsType.defined) {
    retType = ExprType(lhsType.access);
    return;
  }

  // Check that left operand of tensor read is actually a tensor.
  if (!lhsType.isTensor()) {
    std::stringstream errMsg;
    errMsg << "cannot access elements from objects of type " 
           << toString(lhsType);
    reportError(errMsg.str(), expr->tensor);
    retType = ExprType(lhsType.access);
    return;
  }

  const unsigned numIndices = expr->indices.size();
  const auto tensorType = to<TensorType>(lhsType.type[0]);
  const ScalarType::Type componentType = getComponentType(tensorType);
  const TensorDimensions dimensions = getDimensions(tensorType);

  // Check that right number of indices is passed to tensor read.
  if (numIndices != dimensions.size()) {
    std::stringstream errMsg;
    errMsg << "tensor access expected " << dimensions.size() 
           << " indices but got " << numIndices;
    reportError(errMsg.str(), expr);
    retType = ExprType(lhsType.access);
    return;
  }

  TensorDimensions retDimensions;
  for (unsigned i = 0; i < numIndices; ++i) {
    const ReadParam::Ptr index = expr->indices[i];

    if (index->isSlice()) {
      retDimensions.push_back(dimensions[i]);
      continue;
    }

    const Expr::Ptr indexExpr = to<ExprParam>(index)->expr;
    const ExprType indexType = inferType(indexExpr);

    if (!indexType.defined) {
      continue;
    }
    
    // Check that index is a single value.
    if (indexType.isVoid()) {
      reportError("must pass a non-void value as index" , index);
      continue;
    } else if (!indexType.isSingleValue()) {
      std::stringstream errMsg;
      reportError("cannot pass multiple values as a single index", index);
      continue;
    }

    const IndexSet::Ptr indexSet = dimensions[i][0];

    // Check that index is of right type.
    if (isa<RangeIndexSet>(indexSet) || (isa<GenericIndexSet>(indexSet) && 
        to<GenericIndexSet>(indexSet)->type == GenericIndexSet::Type::RANGE)) {
      if (!indexType.isScalarInt()) {
        std::stringstream errMsg;
        errMsg << "expected an integral index but got an index of type " 
               << toString(indexType);
        reportError(errMsg.str(), index);
      }
    } else if (isa<SetIndexSet>(indexSet)) {
      const std::string domain = to<SetIndexSet>(indexSet)->setName;

      if (!isa<ElementType>(indexType.type[0])) {
        std::stringstream errMsg;
        errMsg << "expected an element of set '" << domain << "' as index "
               << "but got an index of type " << toString(indexType);
        reportError(errMsg.str(), index);
        continue;
      }

      const auto elemType = to<ElementType>(indexType.type[0]);
      const auto elemSource = elemType->source;

      if (elemSource && !env.compareIndexSets(indexSet, elemSource)) {
        std::stringstream errMsg;
        errMsg << "expected an element of set '" << domain << "' as index but "
               << "got an element of set '" << elemSource->setName << "'";
        reportError(errMsg.str(), index);
      }
    } else {
      not_supported_yet;
    }
  }

  const auto ndTensorType = to<NDTensorType>(tensorType);

  if (retDimensions.empty()) {
    retType = ExprType(ndTensorType->blockType, lhsType.access);
  } else {
    bool isTransposed = false;
    if (retDimensions.size() == 1) {
      const unsigned blockDims = getDimensions(ndTensorType->blockType).size();
      isTransposed = (blockDims == 1) ? ndTensorType->transposed :
                     !expr->indices[numIndices - 2]->isSlice();
    }

    const auto retTensorType = makeTensorType(componentType, retDimensions, 
                                              isTransposed);
    retType = ExprType(retTensorType, lhsType.access);
  }
}

void TypeChecker::visit(SetReadExpr::Ptr expr) {
  const ExprType lhsType = inferType(expr->set);

  if (!lhsType.defined) {
    retType = ExprType(lhsType.access);
    return;
  }

  // Check that program does not attempt to read from multiple values
  // simultaneously
  if (lhsType.isVoid()) {
    reportError("cannot access field of a void value", expr->set);
    retType = ExprType(lhsType.access);
    return;
  }
  else if (!lhsType.isSingleValue()) {
    reportError("can only access elements of a single set", expr->set);
    retType = ExprType(lhsType.access);
    return;
  }

  const Type::Ptr type = lhsType.type[0];

  if (isa<SetType>(type)) {
    // Case 1: Node set, single tuple index, DONT know dims
    if (isa<UnstructuredSetType>(type)) {
      // TODO: No good way to check indices = #dims
      for (auto index : expr->indices) {
        if (index->isSlice()) {
          reportError("lattice point access expects integral indices", index);
          retTypeChecked = false;
        }
      }

      if (retTypeChecked) {
        for (auto index : expr->indices) {
          const Expr::Ptr indexExpr = to<ExprParam>(index)->expr;
          const ExprType indexType = inferType(indexExpr);

          if (indexType.defined && !indexType.isScalarInt()) {
            std::stringstream errMsg;
            errMsg << "lattice point access expects an integral index but got "
                   << "an index of type " << toString(indexType);
            reportError(errMsg.str(), index);
            retTypeChecked = false;
          }
        }
      }
      if (retTypeChecked &&
          to<UnstructuredSetType>(type)->endpoints.size() != 0) {
        reportError("lattice point set cannot have non-zero cardinality",
                    expr->set);
      }

      retType = ExprType(to<SetType>(type)->element, lhsType.access);
    }
    // Case 2: Lattice edge set, double set of indices (#indices = 2*dim)
    else if (isa<LatticeLinkSetType>(type)) {
      // Check number of indices = 2*dim and all integral
      unsigned dims = to<LatticeLinkSetType>(type)->dimensions;
      if (expr->indices.size() != 2*dims) {
        std::stringstream errMsg;
        errMsg << "lattice edge set access expects number of indices to equal "
               << "2*dimensions, but got " << expr->indices.size() << " vs "
               << 2*dims;
        reportError(errMsg.str(), expr->indices[0]);
      }
      else {
        for (auto index : expr->indices) {
          if (index->isSlice()) {
            reportError("lattice edge set access expects integral indices", index);
          }
        }

        if (retTypeChecked) {
          for (auto index : expr->indices) {
            const Expr::Ptr indexExpr = to<ExprParam>(index)->expr;
            const ExprType indexType = inferType(indexExpr);

            if (indexType.defined && !indexType.isScalarInt()) {
              std::stringstream errMsg;
              errMsg << "lattice point access expects an integral index but got "
                     << "an index of type " << toString(indexType);
              reportError(errMsg.str(), index);
              retTypeChecked = false;
            }
          }
        }
      }

      retType = ExprType(to<SetType>(type)->element, lhsType.access);
    }
    else {
      std::stringstream errMsg;
      errMsg << "set type cannot be indexed: " << toString(lhsType);
      reportError(errMsg.str(), expr->set);
    }
  } else {
    std::stringstream errMsg;
    errMsg << "cannot access elements from objects of type "
           << toString(lhsType);
    reportError(errMsg.str(), expr->set);
  }
}

void TypeChecker::visit(TupleReadExpr::Ptr expr) {
  const ExprType lhsType = inferType(expr->tuple);

  if (!lhsType.defined) {
    retType = ExprType(lhsType.access);
    return;
  }

  iassert(lhsType.isSingleValue());

  if (expr->index) {
    const ExprType indexType = inferType(expr->index);

    if (indexType.defined && !indexType.isScalarInt()) {
      std::stringstream errMsg;
      errMsg << "tuple access expects an integral index but got an index " 
             << "of type " << toString(indexType);
      reportError(errMsg.str(), expr->index);
    }
  }

  retType = ExprType(to<TupleType>(lhsType.type[0])->element, lhsType.access);
}

void TypeChecker::visit(FieldReadExpr::Ptr expr) {
  const ExprType lhsType = inferType(expr->setOrElem);

  if (!lhsType.defined) {
    retType = ExprType(lhsType.access);
    return;
  }

  // Check that program does not attempt to read from multiple values 
  // simultaneously (e.g. output of function call returning two tensors).
  if (lhsType.isVoid()) {
    reportError("cannot access field of a void value", expr->setOrElem);
    retType = ExprType(lhsType.access);
    return;
  } else if (!lhsType.isSingleValue()) {
    reportError("cannot access fields of multiple values", expr->setOrElem);
    retType = ExprType(lhsType.access);
    return;
  }

  const Type::Ptr type = lhsType.type[0];

  ElementType::Ptr elemType;
  if (isa<ElementType>(type)) {
    elemType = to<ElementType>(type);
  } else if (isa<SetType>(type)) {
    elemType = to<SetType>(type)->element;
  }

  // Check that program only reads fields from sets and elements.
  if (!elemType) {
    reportError("can only access fields of sets or elements", expr->setOrElem);
    retType = ExprType(lhsType.access);
    return;
  }

  const std::string fieldName = expr->field->ident;

  // Check that field is defined for set/element being read.
  if (!env.hasElementField(elemType->ident, fieldName)) {
    std::stringstream errMsg;
    errMsg << "undefined field '" << fieldName << "'";
    reportError(errMsg.str(), expr->field);
    retType = ExprType(lhsType.access);
    return;
  }

  TensorType::Ptr retTensorType = 
      to<TensorType>(env.getElementField(elemType->ident, fieldName));

  if (isa<SetType>(type)) {
    if (getDimensions(retTensorType).size() > 1) {
      reportError("can only access scalar or vector set fields", expr);
      retType = ExprType(lhsType.access);
      return;
    }

    const auto indexSet = std::make_shared<SetIndexSet>();
    indexSet->setName = to<VarExpr>(expr->setOrElem)->ident;
    env.addSetDefinition(indexSet, to<SetType>(type));

    const auto setReadTensorType = std::make_shared<NDTensorType>();
    setReadTensorType->indexSets.push_back(indexSet);
    setReadTensorType->transposed = getTransposed(retTensorType);
    setReadTensorType->blockType = retTensorType;

    retTensorType = setReadTensorType;
  }

  retType = ExprType(retTensorType, lhsType.access);
}

void TypeChecker::visit(ParenExpr::Ptr expr) {
  retType = inferType(expr->expr);
}

void TypeChecker::visit(VarExpr::Ptr expr) {
  // Check that variable has been declared.
  if (!env.hasSymbol(expr->ident)) {
    if (!skipCheckDeclared) {
      reportUndeclared("variable or constant", expr->ident, expr);
    }

    retType = ExprType(Access::READ_WRITE);
    return;
  }
 
  const Type::Ptr varType = env.getSymbolType(expr->ident);
  const Access varAccess = env.getSymbolAccess(expr->ident);

  retType = varType ? ExprType(varType, varAccess) : ExprType(varAccess);
}

void TypeChecker::visit(IntLiteral::Ptr lit) {
  retType = ExprType(makeTensorType(ScalarType::Type::INT));
}

void TypeChecker::visit(FloatLiteral::Ptr lit) {
  retType = ExprType(makeTensorType(ScalarType::Type::FLOAT));
}

void TypeChecker::visit(BoolLiteral::Ptr lit) {
  retType = ExprType(makeTensorType(ScalarType::Type::BOOL));
}

void TypeChecker::visit(ComplexLiteral::Ptr lit) {
  retType = ExprType(makeTensorType(ScalarType::Type::COMPLEX));
}

void TypeChecker::visit(StringLiteral::Ptr lit) {
  retType = ExprType(makeTensorType(ScalarType::Type::STRING));
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

void TypeChecker::visit(Test::Ptr test) {
  for (auto arg : test->args) {
    inferType(arg);
  }

  inferType(test->expected);
}

void TypeChecker::typeCheckVarOrConstDecl(VarDecl::Ptr decl, bool isConst, 
                                          bool isGlobal) {
  const std::string varName = decl->name->ident;
  const std::string varDeclType = isConst ? "constant" : "variable";

  const bool typeChecked = decl->type ? typeCheck(decl->type) : false;
  Type::Ptr varType = typeChecked ? decl->type : Type::Ptr();
  
  ExprType initType;
  if (decl->initVal) {
    initType = inferType(decl->initVal);

    // Infer variable type from initial value if not explicitly specified.
    if (!decl->type && initType.defined && initType.isSingleValue()) {
      varType = initType.type[0];
    }
  }

  // Check that variable/constant hasn't already been declared in current scope.
  if (env.hasSymbol(varName, Environment::Scope::CurrentOnly)) {
    reportMultipleDefs(varDeclType, varName, decl);
  }
  
  // Record declaration of variable/constant in symbol table.
  const Access access = isConst ? Access::READ : Access::READ_WRITE;
  env.addSymbol(varName, varType, access);

  // If type signature is invalid or type signature is unspecified and we 
  // cannot infer type from initial value, then there is nothing more to do. 
  if (!varType) {
    return;
  }

  // Check that variable/constant is a tensor.
  if (!isa<TensorType>(varType)) {
    std::stringstream errMsg;
    errMsg << "cannot declare a non-tensor " << varDeclType;
    reportError(errMsg.str(), decl);
    return;
  }

  // Check that initial value type matches declared variable/constant type.
  // If this check completes successfully, then we are done. 
  if (!initType.defined || (initType.isTensor() && 
      env.compareTypes(varType, initType.type[0]))) {
    return;
  }

  std::stringstream errMsg;
  errMsg << "cannot initialize a " << varDeclType << " of type "
         << toString(varType) << " with an expression of type "
         << toString(initType);

  // Check that initial value is of tensor type. If it is not, then there must 
  // be an error since we require variables to be tensors (at least for the 
  // time being). Otherwise, proceed to check legal special cases.
  if (!initType.isTensor()) {
    reportError(errMsg.str(), decl);
    return;
  }

  const TensorType::Ptr varTensorType = to<TensorType>(varType);
  const TensorType::Ptr initTensorType = to<TensorType>(initType.type[0]);
  const ScalarType::Type varComponentType = getComponentType(varTensorType);
  const ScalarType::Type initComponentType = getComponentType(initTensorType);

  // Check if attempting to initialize a local tensor with a scalar.
  if (isa<ScalarType>(initTensorType) && varComponentType == initComponentType) {
    // TODO: It might be useful to be able to initialize non-scalar global 
    //       tensors by scalar values. We prohibit this for now because it 
    //       is not supported by the backend.
    if (isGlobal) {
      const auto msg = "cannot initialize a non-scalar global "
                       "constant by a scalar value";
      reportError(msg, decl);
    }

    return;
  }

  if (isa<NDTensorType>(varTensorType) && isa<NDTensorType>(initTensorType)) {
    const auto varNDTensorType = to<NDTensorType>(varTensorType);
    const auto initNDTensorType = to<NDTensorType>(initTensorType);
    const auto varBlockType = varNDTensorType->blockType;
    const auto initBlockType = initNDTensorType->blockType;
    
    // Check if initial value type is equivalent to declared constant type.
    if (isConst && env.compareTypes(varBlockType, initBlockType)) {
      const auto &varDimensions = varNDTensorType->indexSets;
      const auto &initDimensions = initNDTensorType->indexSets;
    
      // Search for first "non-trivial" dimensions of both types.
      auto varDimsIt = varDimensions.begin();
      while (varDimsIt != varDimensions.end()) {
        if (!isa<RangeIndexSet>(*varDimsIt) || 
            (to<RangeIndexSet>(*varDimsIt)->range != 1)) {
          break;
        }
        ++varDimsIt;
      }
      
      auto initDimsIt = initDimensions.begin();
      while (initDimsIt != initDimensions.end()) {
        if (!isa<RangeIndexSet>(*initDimsIt) || 
            (to<RangeIndexSet>(*initDimsIt)->range != 1)) {
          break;
        }
        ++initDimsIt;
      }

      const IndexDomain varNonTrivialDims(varDimsIt, varDimensions.end());
      const IndexDomain initNonTrivialDims(initDimsIt, initDimensions.end());
      
      if (env.compareDomains(varNonTrivialDims, initNonTrivialDims)) {
        return;
      }
    }
  }

  // Initialization must be illegal, so report error.
  reportError(errMsg.str(), decl);
}

void TypeChecker::typeCheckMapOrApply(MapExpr::Ptr expr, const bool isApply) {
  const std::string opString = isApply ? "apply" : "map";

  std::vector<ExprType> actualsType(expr->partialActuals.size());
  for (unsigned i = 0; i < expr->partialActuals.size(); ++i) {
    const Expr::Ptr param = expr->partialActuals[i];
   
    // Infer argument type.
    actualsType[i] = inferType(param);

    if (!actualsType[i].defined) {
      continue;
    }

    // Check that argument is a single non-void value.
    if (actualsType[i].isVoid()) {
      reportError("must pass a non-void value as argument", param);
      continue;
    } else if (!actualsType[i].isSingleValue()) {
      reportError("cannot pass multiple values as a single argument", param);
      continue;
    }
      
    // Check that additional argument is a tensor.
    // TODO: Remove once support for passing non-tensor arguments is added.
    if (!actualsType[i].isTensor()) {
      std::stringstream errMsg;
      errMsg << "expected argument to be a tensor but got an argument "
             << "of type " << toString(actualsType[i].type[0]);
      reportError(errMsg.str(), param);
    }
  }

  FuncDecl::Ptr func;
  SetType::Ptr targetSetType;
  SetType::Ptr throughSetType;
  
  std::string funcName = expr->func->ident;
  
  // Check that assembly function has been declared.
  if (!env.hasFunction(funcName)) {
    reportUndeclared("function", funcName, expr->func);
    return;
  } else {
    func = env.getFunction(funcName);
    
    if (isApply && !func->results.empty()) {
      reportError("cannot apply a non-void function", expr->func);
    }
  }

  if (!func->originalName.empty()) {
    funcName = func->originalName;
  }

  // Check that target has been declared and is a (non-generic) set.
  if (!typeCheck(expr->target)) {
    retTypeChecked = false;
  } else if (isa<GenericIndexSet>(expr->target)) {
    std::stringstream errMsg;
    errMsg << opString << " operation cannot be applied to generic sets";
    reportError(errMsg.str(), expr->target);
  } else {
    targetSetType = env.getSetDefinition(expr->target);
  }
  
  auto getResType = [](IdentDecl::Ptr res) { return res->type; };

  // If through is declared, check that it is a lattice link set,
  if (expr->through) {
    if (!typeCheck(expr->through)) {
      retTypeChecked = false;
    } else if (isa<GenericIndexSet>(expr->through)) {
      std::stringstream errMsg;
      errMsg << opString << " operation cannot be applied through generic sets";
      reportError(errMsg.str(), expr->through);
    } else {
      throughSetType = env.getSetDefinition(expr->through);

      if (!isa<LatticeLinkSetType>(throughSetType)) {
        std::stringstream errMsg;
        errMsg << opString << " operation can only be applied through lattice "
               << "link sets";
        reportError(errMsg.str(), expr->through);
      }
      else if (to<LatticeLinkSetType>(throughSetType)
               ->latticePointSet->set->setName !=
               expr->target->setName) {
        std::stringstream errMsg;
        errMsg << opString << " operation can only be mapped through lattice "
               << "link set with target " << expr->target->setName
               << " as an endpoint";
        reportError(errMsg.str(), expr->through);
        retTypeChecked = false;
      }
    }
  }

  if (!retTypeChecked) {
    // Infer map operation return type if possible.
    if (func && func->genericParams.empty()) {
      ExprType::TypeVector resultTypes(func->results.size());
      std::transform(func->results.begin(), func->results.end(), 
                     resultTypes.begin(), getResType);
      retType = ExprType(resultTypes);
    }

    return;
  }

  // Infer assembly function's required argument types.
  actualsType.push_back(ExprType(targetSetType->element, Access::READ_WRITE));
  
  if (isa<UnstructuredSetType>(targetSetType) &&
      !to<UnstructuredSetType>(targetSetType)->endpoints.empty()) {
    UnstructuredSetType::Ptr utype = to<UnstructuredSetType>(targetSetType);
    const auto endpoint = utype->endpoints[0];
    const auto neighborSet = endpoint->set->setName;
    
    // Check for heterogeneous edge sets, which are currently unsupported.
    // TODO: Should remove this once support for heterogeneous edge sets 
    //       has been added.
    for (unsigned i = 1; i < utype->endpoints.size(); ++i) {
      if (utype->endpoints[i]->set->setName != neighborSet) {
        std::stringstream errMsg;
        errMsg << opString << " operation is currently unsupported for "
               << "heterogeneous edge sets";
        reportError(errMsg.str(), expr->target);
        return;
      }
    }
    
    if (actualsType.size() != func->args.size()) {
      const auto neighborsLength = std::make_shared<TupleLength>();
      neighborsLength->val = utype->endpoints.size();

      const auto neighborsType = std::make_shared<TupleType>();
      neighborsType->element = endpoint->element;
      neighborsType->length = neighborsLength; 
      actualsType.push_back(ExprType(neighborsType, Access::READ_WRITE));
    }
  }

  // Through declaration adds link set as an argument.
  if (throughSetType) {
    actualsType.push_back(ExprType(throughSetType, Access::READ));
  }
 
  if (!func->genericParams.empty()) {
    // Type parameter resolution needs to be done with information that is 
    // stored only with the original version of the generic function (in 
    // particular, input element sources).
    const FuncDecl::Ptr funcSignature = func->originalName.empty() ? func :
                                        env.getFunction(func->originalName);

    GenericCallTypeChecker checker(env);
    const unsigned numArgs = std::min(actualsType.size(), 
                                      funcSignature->args.size());

    for (unsigned i = 0; i < numArgs; ++i) {
      const ExprType paramType = actualsType[i];
      if (paramType.defined && paramType.isSingleValue()) {
        checker.unify(funcSignature->args[i]->type, paramType.type[0]);
      }
    }

    for (const auto genericParam : funcSignature->genericParams) {
      const auto it = checker.specializedSets.find(genericParam->name);
      if (it == checker.specializedSets.end()) {
        std::stringstream errMsg;
        errMsg << "unable to resolve type parameter '" << genericParam->name 
               << "' for " << opString << " operation with function '" 
               << funcName << "'";
        reportError(errMsg.str(), expr);
        return;
      }
    }

    if (func->originalName.empty()) {
      func = func->clone<FuncDecl>();
      func->body = StmtBlock::Ptr();
    }

    ReplaceTypeParams(checker.specializedSets).rewrite(func);

    if (!func->originalName.empty()) {
      const std::string typeSig = getConcretizedTypeSignatureString(func);

      if (env.hasFunctionReplacement(typeSig)) {
        func = env.getFunctionReplacement(typeSig);
        expr->func->ident = func->name->ident;
      } else {
        env.addFunctionReplacement(typeSig, func);
        func->genericParams.clear();

        // Callee might itself call another generic function with arguments of 
        // generic types, in which case the generic function called by the 
        // callee would also have to be concretized.
        if (errors->empty()) {
          typeCheck(func);
        }
      }
    }
  }
  
  // Infer map operation return type.
  ExprType::TypeVector resultTypes(func->results.size());
  std::transform(func->results.begin(), func->results.end(), 
                 resultTypes.begin(), getResType);
  retType = ExprType(resultTypes);

  // Check that assembly function accepts right number of arguments.
  if (actualsType.size() != func->args.size()) {
    std::stringstream errMsg;
    errMsg << opString << " operation passes " << actualsType.size() 
           << " arguments to assembly function but function '" << funcName
           << "' expects " << func->args.size() << " arguments";
    reportError(errMsg.str(), expr);
    return;
  }

  for (unsigned i = 0; i < actualsType.size(); ++i) {
    const ExprType paramType = actualsType[i];
    const Argument::Ptr funcArg = func->args[i];

    if (!paramType.defined || !paramType.isSingleValue()) {
      continue;
    }

    // Check that argument is of type expected by callee.
    if (!env.compareTypes(paramType.type[0], funcArg->type)) {
      std::stringstream errMsg;
      errMsg << opString << " operation passes argument of type "
             << toString(paramType.type[0]) << " to assembly function '"
             << funcName << "' but function expects argument of type "
             << toString(funcArg->type);
      if (i < expr->partialActuals.size()) {
        reportError(errMsg.str(), expr->partialActuals[i]);
      } else {
        reportError(errMsg.str(), expr->target);
      }
    }

    // Check that inout argument is writable.
    if (funcArg->isInOut() && !paramType.isWritable()) {
      reportError("inout argument must be writable", funcArg);
    }
  }
}

void TypeChecker::typeCheckBinaryElwise(BinaryExpr::Ptr expr, 
                                        bool allowStringOperands) {
  const ExprType lhsType = inferType(expr->lhs);
  const ExprType rhsType = inferType(expr->rhs);
  retTypeChecked = lhsType.defined && rhsType.defined;

  // Check that operands of element-wise operation are numeric tensors 
  // (or strings, if operation can be performed on strings).
  if (lhsType.defined && !lhsType.isNumericTensor() && 
      (!allowStringOperands || !lhsType.isString())) {
    std::stringstream errMsg;
    errMsg << "expected left operand of element-wise operation to be a "
           << "numeric tensor " << (allowStringOperands ? "or string " : "")
           << "but got an operand of type " << toString(lhsType);
    reportError(errMsg.str(), expr->lhs);
  }
  if (rhsType.defined && !rhsType.isNumericTensor() && 
      (!allowStringOperands || !rhsType.isString())) {
    std::stringstream errMsg;
    errMsg << "expected right operand of element-wise operation to be a "
           << "numeric tensor " << (allowStringOperands ? "or string " : "")
           << "but got an operand of type " << toString(rhsType);
    reportError(errMsg.str(), expr->rhs);
  }
 
  if (!retTypeChecked) {
    return;
  }

  const auto ltype = to<TensorType>(lhsType.type[0]);
  const auto rtype = to<TensorType>(rhsType.type[0]);
  const ScalarType::Type lhsComponent = getComponentType(ltype);
  const ScalarType::Type rhsComponent = getComponentType(rtype);
  const unsigned lhsOrder = getDimensions(ltype).size();
  const unsigned rhsOrder = getDimensions(rtype).size();
  const bool hasScalarOperand = (lhsOrder == 0 || rhsOrder == 0);

  // Check that operands are compatible (i.e. contain elements of same type 
  // if one operand is scalar, or also have same dimensions otherwise).
  if (hasScalarOperand ? (lhsComponent != rhsComponent) : 
      !env.compareTypes(ltype, rtype)) {
    std::stringstream errMsg;
    errMsg << "cannot perform element-wise operation on tensors of type "
           << toString(ltype) << " and type " << toString(rtype);
    reportError(errMsg.str(), expr);
    return;
  }
  
  retType = (lhsOrder > 0) ? ExprType(ltype) : ExprType(rtype);
}

void TypeChecker::typeCheckBinaryBoolean(BinaryExpr::Ptr expr) {
  const ExprType lhsType = inferType(expr->lhs);
  const ExprType rhsType = inferType(expr->rhs);

  // Check that operands of boolean operation are of boolean type.
  if (lhsType.defined && !lhsType.isScalarBoolean()) {
    std::stringstream errMsg;
    errMsg << "expected left operand of boolean operation to be a boolean "
           << "but got an operand of type " << toString(lhsType);
    reportError(errMsg.str(), expr->lhs);
  }
  if (rhsType.defined && !rhsType.isScalarBoolean()) {
    std::stringstream errMsg;
    errMsg << "expected right operand of boolean operation to be a boolean "
           << "but got an operand of type " << toString(rhsType);
    reportError(errMsg.str(), expr->rhs);
  }

  retType = ExprType(makeTensorType(ScalarType::Type::BOOL));
}

void TypeChecker::typeCheckDenseTensorLiteral(DenseTensorLiteral::Ptr lit) {
  try {
    const DenseTensorType tensor = getDenseTensorType(lit);
    const auto &dimSizes = tensor.dimSizes;

    const NDTensorType::Ptr tensorType = std::make_shared<NDTensorType>();
    tensorType->transposed = (tensor.dimSizes.size() == 1) && !lit->transposed;

    for (auto it = dimSizes.rbegin(); it != dimSizes.rend(); ++it) {
      const auto indexSet = std::make_shared<RangeIndexSet>();
      indexSet->range = *it;
      tensorType->indexSets.push_back(indexSet);
    }

    switch (tensor.type) {
      case DenseTensorType::Type::INT:
        tensorType->blockType = makeTensorType(ScalarType::Type::INT);
        break;
      case DenseTensorType::Type::FLOAT:
        tensorType->blockType = makeTensorType(ScalarType::Type::FLOAT);
        break;
      case DenseTensorType::Type::COMPLEX:
        tensorType->blockType = makeTensorType(ScalarType::Type::COMPLEX);
        break;
      default:
        unreachable;
        break;
    }

    retType = ExprType(tensorType);
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

bool TypeChecker::ExprType::isNumericType(ScalarType::Type type) const {
  switch (type) {
    case ScalarType::Type::INT:
    case ScalarType::Type::FLOAT:
    case ScalarType::Type::COMPLEX:
      return true;
    default:
      return false;
  }
}

bool TypeChecker::ExprType::isNumericTensor() const {
  if (!isTensor()) {
    return false;
  }

  TensorType::Ptr tensorType = to<TensorType>(type[0]);
  while (isa<NDTensorType>(tensorType)) {
    tensorType = to<NDTensorType>(tensorType)->blockType;
  }

  return isNumericType(to<ScalarType>(tensorType)->type);
}

bool TypeChecker::ExprType::isReadable() const {
  switch (access) {
    case Access::READ:
    case Access::READ_WRITE:
      return true;
    default:
      return false;
  }
}

bool TypeChecker::ExprType::isWritable() const {
  switch (access) {
    case Access::WRITE:
    case Access::READ_WRITE:
      return true;
    default:
      return false;
  }
}

void TypeChecker::ComputeSetDefinitions::visit(SetIndexSet::Ptr set) {
  if (!decls.contains(set->setName)) {
    return;
  }
  
  const SetType::Ptr type = decls.get(set->setName);
  
  if (type) {
    env.addSetDefinition(set, type);
  }
}

void TypeChecker::ComputeSetDefinitions::visit(IdentDecl::Ptr decl) {
  HIRVisitor::visit(decl);

  const auto type = isa<SetType>(decl->type) ? 
                    to<SetType>(decl->type) : SetType::Ptr();
  decls.insert(decl->name->ident, type);
}

void TypeChecker::ComputeSetDefinitions::visit(FuncDecl::Ptr decl) {
  decls.insert(decl->name->ident, SetType::Ptr());
  decls.scope();
  
  for (const auto genericParam : decl->genericParams) {
    const auto setType = SetType::getUndefinedSetType();
    decls.insert(genericParam->name, setType);
    env.addSetDefinition(genericParam, setType);
  }
  
  HIRVisitor::visit(decl);
  decls.unscope();
}

void TypeChecker::ComputeSetDefinitions::visit(VarDecl::Ptr decl) {
  HIRVisitor::visit(decl);

  const auto type = isa<SetType>(decl->type) ? 
                    to<SetType>(decl->type) : SetType::Ptr();
  decls.insert(decl->name->ident, type);
}

void TypeChecker::ComputeSetDefinitions::visit(WhileStmt::Ptr stmt) {
  stmt->cond->accept(this);
  
  decls.scope();
  stmt->body->accept(this);
  decls.unscope();
}
  
void TypeChecker::ComputeSetDefinitions::visit(IfStmt::Ptr stmt) {
  stmt->cond->accept(this);

  decls.scope();
  stmt->ifBody->accept(this);
  decls.unscope();

  if (stmt->elseBody) {
    decls.scope();
    stmt->elseBody->accept(this);
    decls.unscope();
  }
}

void TypeChecker::ComputeSetDefinitions::visit(ForStmt::Ptr stmt) {
  decls.scope();
  stmt->domain->accept(this);
  decls.insert(stmt->loopVar->ident, SetType::Ptr()); 
  
  stmt->body->accept(this);
  decls.unscope();
}

void TypeChecker::ComputeSetDefinitions::visit(AssignStmt::Ptr stmt) {
  stmt->expr->accept(this);
  
  for (auto lhs : stmt->lhs) {
    lhs->accept(this);
    
    if (!isa<VarExpr>(lhs)) {
      continue;
    }
    
    const std::string varName = to<VarExpr>(lhs)->ident;
    
    if (!decls.contains(varName)) {
      decls.insert(varName, SetType::Ptr());
    }
  }
}

void TypeChecker::GenericCallTypeChecker::unify(Type::Ptr paramType, 
                                                Type::Ptr argType) {
  if (isa<NDTensorType>(paramType) && isa<NDTensorType>(argType)) {
    const auto paramTensorType = to<NDTensorType>(paramType);
    const auto argTensorType = to<NDTensorType>(argType);
    const auto paramDomain = paramTensorType->indexSets;
    const auto argDomain = argTensorType->indexSets;
    
    const unsigned domainSize = std::min(paramDomain.size(), argDomain.size());

    for (unsigned i = 0; i < domainSize; ++i) {
      if (isa<GenericIndexSet>(paramDomain[i])) {
        const auto genericName = to<GenericIndexSet>(paramDomain[i])->setName;
        
        if (specializedSets.find(genericName) == specializedSets.end()) {
          if (isa<SetIndexSet>(argDomain[i])) {
            const auto indexSet = argDomain[i]->clone<SetIndexSet>();
            const auto argIndexSet = to<SetIndexSet>(argDomain[i]);
            env.addSetDefinition(indexSet, env.getSetDefinition(argIndexSet));

            const auto paramGenericType = isa<GenericIndexSet>(paramDomain[i]) ?
                to<GenericIndexSet>(paramDomain[i])->type : 
                GenericIndexSet::Type::UNKNOWN;
            const auto argGenericType = isa<GenericIndexSet>(argDomain[i]) ?
                to<GenericIndexSet>(argDomain[i])->type : 
                GenericIndexSet::Type::UNKNOWN;
            
            if (paramGenericType != GenericIndexSet::Type::RANGE ||
                argGenericType == GenericIndexSet::Type::RANGE) {
              specializedSets[genericName] = indexSet;
            }
          } else if (isa<RangeIndexSet>(argDomain[i])) {
            specializedSets[genericName] = argDomain[i]->clone<RangeIndexSet>();
          } else {
            not_supported_yet;
          }
        }
      }
    }

    unify(paramTensorType->blockType, argTensorType->blockType);
  } else if (isa<ElementType>(paramType) && isa<ElementType>(argType)) {
    const auto paramElemType = to<ElementType>(paramType);
    const auto argElemType = to<ElementType>(argType);

    if (argElemType->source && isa<GenericIndexSet>(paramElemType->source)) {
      const auto genericName = paramElemType->source->setName;

      if (specializedSets.find(genericName) == specializedSets.end()) {
        specializedSets[genericName] = argElemType->source;
      }
    }
  } else if (isa<TupleType>(paramType) && isa<TupleType>(argType)) {
    const auto paramTupleType = to<TupleType>(paramType);
    const auto argTupleType = to<TupleType>(argType);
    unify(paramTupleType->element, argTupleType->element);
  }
}

std::string TypeChecker::getConcretizedTypeSignatureString(FuncDecl::Ptr decl) {
  class FuncTypeSignaturePrinter : public HIRPrinter {
    public:
      FuncTypeSignaturePrinter(std::ostream &oss) : HIRPrinter(oss) {}

    private:
      virtual void visit(SetIndexSet::Ptr set) {
        iassert((bool)set->setDef);
        oss << set->setDef;
      }

      virtual void visit(FuncDecl::Ptr decl) {
        switch (decl->type) {
          case FuncDecl::Type::EXPORTED:
            oss << "export ";
            break;
          case FuncDecl::Type::EXTERNAL:
            oss << "extern ";
            break;
          default:
            break;
        }
        
        iassert(!decl->originalName.empty());
        oss << "func " << decl->originalName << "(";
        
        for (auto arg : decl->args) {
          arg->accept(this);
          oss << ",";
        }
        
        oss << ") -> (";
          
        for (auto result : decl->results) {
          result->accept(this);
          oss << ",";
        }
        
        oss << ")";
      }
  };

  std::stringstream oss;
  FuncTypeSignaturePrinter printer(oss);
  decl->accept(&printer);

  return oss.str();
}

void TypeChecker::ReplaceTypeParams::visit(GenericIndexSet::Ptr set) {
  node = specializedSets.at(set->setName);
}

void TypeChecker::ReplaceTypeParams::visit(IndexSetDomain::Ptr domain) {
  HIRRewriter::visit(domain);

  if (isa<RangeIndexSet>(domain->set)) {
    const auto lowerBound = std::make_shared<IntLiteral>();
    lowerBound->val = 0;

    const auto upperBound = std::make_shared<IntLiteral>();
    upperBound->val = to<RangeIndexSet>(domain->set)->range;

    const auto rangeDomain = std::make_shared<RangeDomain>();
    rangeDomain->lower = lowerBound;
    rangeDomain->upper = upperBound;

    node = rangeDomain;
  }
}

void TypeChecker::ReplaceTypeParams::visit(RangeConst::Ptr expr) {
  const auto literal = std::make_shared<IntLiteral>();
  literal->val = to<RangeIndexSet>(specializedSets.at(expr->ident))->range;

  node = literal;
}

void TypeChecker::Environment::addSymbol(const std::string& name, 
                                         Type::Ptr type, Access access) {
  if (isa<SetType>(type)) {
    const auto setType = to<SetType>(type);
    
    const SetIndexSet::Ptr set = (setType->element->ident == "") ? 
                                 std::make_shared<GenericIndexSet>() : 
                                 std::make_shared<SetIndexSet>();
    set->setName = name;

    setType->element->source = set;
    addSetDefinition(set, setType);
  }

  symbolTable.insert(name, SymbolType(type, access));
}

bool TypeChecker::Environment::compareIndexSets(IndexSet::Ptr l, 
                                                IndexSet::Ptr r) {
  if (isa<SetIndexSet>(l)) {
    if (!isa<SetIndexSet>(r)) {
      return false;
    }

    const auto lSet = to<SetIndexSet>(l);
    const auto rSet = to<SetIndexSet>(r);

    if (!hasSetDefinition(lSet) || !hasSetDefinition(rSet)) {
      return false;
    }
    
    SetType::Ptr lType = getSetDefinition(lSet);
    SetType::Ptr rType = getSetDefinition(rSet);
    if (isa<UnstructuredSetType>(lType)) {
      return lType == rType;
    }
    else if (isa<LatticeLinkSetType>(lType)) {
      // Pointers may not be identical if LatticeLinkSetType
      if (!isa<LatticeLinkSetType>(rType)) {
        return false;
      }
      auto lLatType = to<LatticeLinkSetType>(lType);
      auto rLatType = to<LatticeLinkSetType>(rType);
      if (lLatType->dimensions != rLatType->dimensions) {
        return false;
      }
      return compareIndexSets(lLatType->latticePointSet->set,
                              rLatType->latticePointSet->set);
    }
    else {
      return false;
    }
  } else if (isa<RangeIndexSet>(l)) {
    return isa<RangeIndexSet>(r) &&
           (to<RangeIndexSet>(l)->range == to<RangeIndexSet>(r)->range);
  }

  return false;
}

bool TypeChecker::Environment::compareDomains(const IndexDomain &l,
                                              const IndexDomain &r) {
  if (l.size() != r.size()) {
    return false;
  }

  for (unsigned i = 0; i < l.size(); ++i) {
    if (!compareIndexSets(l[i], r[i])) {
      return false;
    }
  }

  return true;
}

bool TypeChecker::Environment::compareTypes(Type::Ptr l, Type::Ptr r, 
                                            bool ignoreTranspose) {
  if (isa<ScalarType>(l)) {
    return isa<ScalarType>(r) && 
           (to<ScalarType>(l)->type == to<ScalarType>(r)->type);
  } else if (isa<NDTensorType>(l)) {
    if (!isa<NDTensorType>(r)) {
      return false;
    }

    const auto ltype = to<NDTensorType>(l);
    const auto rtype = to<NDTensorType>(r);

    return ((ltype->transposed == rtype->transposed) || ignoreTranspose) && 
           compareDomains(ltype->indexSets, rtype->indexSets) && 
           compareTypes(ltype->blockType, rtype->blockType);
  } else if (isa<ElementType>(l)) {
    if (!isa<ElementType>(r)) {
      return false;
    }

    const auto ltype = to<ElementType>(l);
    const auto rtype = to<ElementType>(r);

    return (!ltype->source || !rtype->source) ? (ltype->ident == rtype->ident) :
           compareIndexSets(ltype->source, rtype->source);
  } else if (isa<SetType>(l)) {
    if (!isa<SetType>(r)) {
      return false;
    }

    const auto lgentype = to<SetType>(l);
    const auto rgentype = to<SetType>(r);

    if (!compareTypes(lgentype->element, rgentype->element)) {
      return false;
    }

    if (isa<UnstructuredSetType>(lgentype)) {
      if (!isa<UnstructuredSetType>(rgentype)) {
        return false;
      }

      const auto ltype = to<UnstructuredSetType>(lgentype);
      const auto rtype = to<UnstructuredSetType>(rgentype);
      
      if (ltype->endpoints.size() != rtype->endpoints.size()) {
        return false;
      }

      for (unsigned i = 0; i < ltype->endpoints.size(); ++i) {
        const auto lEndpointElem = ltype->endpoints[i]->element;
        const auto rEndpointElem = rtype->endpoints[i]->element;
      
        if (!compareTypes(lEndpointElem, rEndpointElem)) {
          return false;
        }
      }

      return true;
    }
    else if (isa<LatticeLinkSetType>(lgentype)) {
      if (!isa<LatticeLinkSetType>(rgentype)) {
        return false;
      }

      const auto ltype = to<LatticeLinkSetType>(lgentype);
      const auto rtype = to<LatticeLinkSetType>(rgentype);

      if (ltype->dimensions != rtype->dimensions) {
        return false;
      }

      if (!compareTypes(ltype->latticePointSet->element,
                        rtype->latticePointSet->element)) {
        return false;
      }
      
      if (ltype->latticePointSet->set->setName !=
          rtype->latticePointSet->set->setName) {
        return false;
      }

      return compareTypes(ltype->latticePointSet->set->setDef,
                          rtype->latticePointSet->set->setDef);
    }

    return false;
  } else if (isa<TupleType>(l)) {
    if (!isa<TupleType>(r)) {
      return false;
    }

    const auto ltype = to<TupleType>(l);
    const auto rtype = to<TupleType>(r);

    return (ltype->length->val == rtype->length->val) && 
           compareTypes(ltype->element, rtype->element);
  }

  return false;
}

TypeChecker::ExprType TypeChecker::inferType(Expr::Ptr ptr) {
  const auto tmp = retType;
  retType = ExprType();
  
  ptr->accept(this);
  const auto ret = retType;
  
  retType = tmp;
  return ret;
}

bool TypeChecker::typeCheck(HIRNode::Ptr ptr) {
  iassert(!isa<Expr>(ptr));
  
  const bool tmp = retTypeChecked;
  retTypeChecked = true;
  
  ptr->accept(this);
  const bool ret = retTypeChecked;
  
  retTypeChecked = tmp;
  return ret;
}

bool TypeChecker::typeCheckGlobalConstDecl(ConstDecl::Ptr ptr) {
  const bool tmp = retTypeChecked;
  retTypeChecked = true;
  
  typeCheckVarOrConstDecl(ptr, true, true);
  const bool ret = retTypeChecked;
  
  retTypeChecked = tmp;
  return ret;
}

void TypeChecker::getDimensions(TensorType::Ptr type, TensorDimensions &dims) {
  if (isa<ScalarType>(type)) {
    return;
  }

  const auto tensorType = to<NDTensorType>(type);

  if (dims.empty()) {
    dims.resize(tensorType->indexSets.size());
  }
  
  iassert(dims.size() == tensorType->indexSets.size());

  for (unsigned i = 0; i < dims.size(); ++i) {
    dims[i].push_back(tensorType->indexSets[i]);
  }

  getDimensions(tensorType->blockType, dims);
}

ScalarType::Type TypeChecker::getComponentType(TensorType::Ptr type) {
  while (isa<NDTensorType>(type)) {
    type = to<NDTensorType>(type)->blockType;
  }

  return to<ScalarType>(type)->type;
}

TypeChecker::TensorDimensions TypeChecker::getDimensions(TensorType::Ptr type) {
  TensorDimensions dimensions;
  getDimensions(type, dimensions);
  return dimensions;
}

unsigned TypeChecker::getOrder(TensorType::Ptr type) {
  return isa<ScalarType>(type) ? 0 : to<NDTensorType>(type)->indexSets.size();
}

bool TypeChecker::getTransposed(TensorType::Ptr type) {
  return isa<NDTensorType>(type) && to<NDTensorType>(type)->transposed;
}

TensorType::Ptr TypeChecker::makeTensorType(ScalarType::Type componentType,
                                            const TensorDimensions &dimensions,
                                            bool transposed) {
  const auto scalarType = std::make_shared<ScalarType>();
  scalarType->type = componentType;

  if (dimensions.empty()) {
    return scalarType;
  }

  TensorType::Ptr retType = scalarType;
  for (unsigned i = 0; i < dimensions[0].size(); ++i) {
    const auto tensorType = std::make_shared<NDTensorType>();
    tensorType->blockType = retType;
    
    const unsigned idx = dimensions[0].size() - i - 1; 
    for (unsigned j = 0; j < dimensions.size(); ++j) {
      tensorType->indexSets.push_back(dimensions[j][idx]);
    }

    retType = tensorType;
  }
  to<NDTensorType>(retType)->transposed = transposed;

  return retType;
}

void TypeChecker::addScalarIntrinsic(const std::string& name,
    const std::vector<ScalarType::Type> &args,
    const std::vector<ScalarType::Type> &results) {
  std::vector<Type::Ptr> argTypes(args.size());
  std::vector<Type::Ptr> resultTypes(results.size());

  for (unsigned i = 0; i < args.size(); ++i) {
    argTypes[i] = makeTensorType(args[i]);
  }
  for (unsigned i = 0; i < results.size(); ++i) {
    resultTypes[i] = makeTensorType(results[i]);
  }
  
  addIntrinsic(name, argTypes, resultTypes);
}

void TypeChecker::addIntrinsic(const std::string& name,
    const std::vector<Type::Ptr> &args,
    const std::vector<Type::Ptr> &results) {
  const auto decl = std::make_shared<FuncDecl>();
  decl->name = std::make_shared<Identifier>();

  decl->name->ident = name;
  decl->type = FuncDecl::Type::INTERNAL;
  
  for (unsigned i = 0; i < args.size(); ++i) {
    const auto arg = std::make_shared<Argument>();
    
    arg->type = args[i];
    decl->args.push_back(arg);
  }

  for (const auto resType : results) {
    const auto res = std::make_shared<IdentDecl>();
    res->type = resType;
    decl->results.push_back(res);
  }

  env.addFunction(name, decl);
}

std::string TypeChecker::toString(ExprType type) {
  if (type.isVoid()) {
    return "'void'";
  } else if (type.isSingleValue()) {
    return toString(type.type[0]);
  }

  std::stringstream oss;
  oss << "'(";
  
  bool printDelimiter = false;
  for (const auto compType : type.type) {
    if (printDelimiter) {
      oss << ", ";
    }
    oss << toString(compType, false);
    printDelimiter = true;
  }

  oss << ")'";
  return oss.str();
}

std::string TypeChecker::toString(Type::Ptr type, bool printQuotes) {
  const std::string quote = (printQuotes ? "'" : "");
 
  std::stringstream typeOut;
  typeOut << *type;

  std::stringstream oss;
  if (typeOut.str() == "") {
    oss << "unknown";
  } else {
    oss << quote << typeOut.str() << quote;
  }
  
  if (isa<ElementType>(type)) {
    const auto elemType = to<ElementType>(type);
    
    if (elemType->source) {
      oss << " from set '" << *elemType->source << "'";
    }
  } else if (isa<TupleType>(type)) {
    const auto tupleType = to<TupleType>(type);

    if (tupleType->element->source) {
      oss << " from set '(" << *tupleType->element->source << " * " 
          << tupleType->length->val << ")'";
    }
  }

  return oss.str();
}

std::string TypeChecker::toString(ScalarType::Type type) {
  switch (type) {
    case ScalarType::Type::INT:
      return "integer";
    case ScalarType::Type::FLOAT:
      return "floating-point";
    case ScalarType::Type::BOOL:
      return "boolean";
    case ScalarType::Type::COMPLEX:
      return "complex";
    case ScalarType::Type::STRING:
      return "string";
    default:
      unreachable;
      return "";
  }
}

void TypeChecker::reportError(const std::string &msg, HIRNode::Ptr loc) {
  const auto err = ParseError(loc->getLineBegin(), loc->getColBegin(), 
                              loc->getLineEnd(), loc->getColEnd(), msg);
  errors->push_back(err);
  retTypeChecked = false;
}

void TypeChecker::reportUndeclared(const std::string &type, 
                                   const std::string &ident,
                                   HIRNode::Ptr loc) {
  std::stringstream errMsg;
  errMsg << "undeclared " << type << " '" << ident << "'";
  reportError(errMsg.str(), loc);
}

void TypeChecker::reportMultipleDefs(const std::string &type, 
                                     const std::string &ident, 
                                     HIRNode::Ptr loc) {
  std::stringstream errMsg;
  errMsg << "multiple definitions of " << type << " '" << ident << "'";
  reportError(errMsg.str(), loc);
}

}
}

