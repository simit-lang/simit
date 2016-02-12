#include <vector>

#include "ir_emitter.h"
#include "types.h"
#include "domain.h"
#include "error.h"
#include "ir.h"

namespace simit {
namespace hir {

void IREmitter::visit(StmtBlock::Ptr stmtBlock) {
  for (auto stmt : stmtBlock->stmts) {
    stmt->accept(this);
  }
  
  const std::vector<ir::Stmt> *stmts = ctx->getStatements();
  retStmt = (stmts->size() == 0) ? ir::Pass::make() : ir::Block::make(*stmts);
}

void IREmitter::visit(RangeIndexSet::Ptr set) {
  retIndexSet = ir::IndexSet(set->range);
}

void IREmitter::visit(SetIndexSet::Ptr set) {
  const ir::Expr setExpr = ctx->getSymbol(set->setName).getExpr();
  
  iassert(setExpr.type().isSet());
  retIndexSet = ir::IndexSet(setExpr);
}

void IREmitter::visit(DynamicIndexSet::Ptr set) {
  retIndexSet = ir::IndexSet();
}

void IREmitter::visit(ElementType::Ptr set) {
  iassert(ctx->containsElementType(set->ident));
  retType = ctx->getElementType(set->ident);
}

void IREmitter::visit(Endpoint::Ptr end) {
  retExpr = ctx->getSymbol(end->setName).getExpr();
}

void IREmitter::visit(SetType::Ptr type) {
  const ir::Type elementType = emitType(type->element);

  std::vector<ir::Expr> endpoints;
  for (auto end : type->endpoints) {
    const ir::Expr endpoint = emitExpr(end);
    endpoints.push_back(endpoint);
  }

  retType = ir::SetType::make(elementType, endpoints);
}

void IREmitter::visit(TupleType::Ptr type) {
  const ir::Type elementType = emitType(type->element);
  
  iassert(type->length > 0);
  retType = ir::TupleType::make(elementType, type->length->val);
}

void IREmitter::visit(ScalarType::Ptr type) {
  switch (type->type) {
    case ScalarType::Type::INT:
      retType = ir::Int;
      break;
    case ScalarType::Type::FLOAT:
      retType = ir::Float;
      break;
    case ScalarType::Type::BOOL:
      retType = ir::Boolean;
      break;
    default:
      unreachable;
      break;
  }
}

void IREmitter::visit(NDTensorType::Ptr type) {
  const ir::Type blockType = emitType(type->blockType);

  std::vector<ir::IndexSet> indexSets;
  for (auto is : type->indexSets) {
    const ir::IndexSet indexSet = emitIndexSet(is);
    indexSets.push_back(indexSet);
  }

  if (indexSets.empty()) {
    retType = blockType;
  } else {
    const auto blockTensorType = blockType.toTensor();
    const auto componentType = blockTensorType->getComponentType();
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
  
    retType = ir::TensorType::make(componentType, dimensions, 
                                   dimensions.size() == 1);
  }

  if (type->transposed) {
    const auto tensorType = retType.toTensor();
    const auto dimensions = tensorType->getDimensions();
    const auto componentType = tensorType->getComponentType();
    retType = ir::TensorType::make(componentType, dimensions);
  }
}

void IREmitter::visit(IdentDecl::Ptr decl) {
  const ir::Type type = emitType(decl->type);
  retVar = ir::Var(decl->name->ident, type);
  retField = ir::Field(decl->name->ident, type);
}

void IREmitter::visit(ElementTypeDecl::Ptr decl) {
  iassert(!ctx->containsElementType(decl->name->ident));
  
  std::vector<ir::Field> fields;
  for (auto f : decl->fields) {
    const ir::Field field = emitField(f);
    fields.push_back(field);
  }

  ctx->addElementType(ir::ElementType::make(decl->name->ident, fields));
}

void IREmitter::visit(ExternDecl::Ptr decl) {
  const ir::Var externVar = emitVar(decl->var);
  ctx->addExtern(externVar);
  ctx->addSymbol(externVar);
}

void IREmitter::visit(FuncDecl::Ptr decl) {
  addFuncOrProc(decl);
}

void IREmitter::visit(ProcDecl::Ptr decl) {
  addFuncOrProc(decl, true);
}

void IREmitter::visit(VarDecl::Ptr decl) {
  addVarOrConst(decl);
}

void IREmitter::visit(ConstDecl::Ptr decl) {
  addVarOrConst(decl, true);
}

void IREmitter::visit(WhileStmt::Ptr stmt) {
  addWhileOrDoWhile(stmt);
}

void IREmitter::visit(DoWhileStmt::Ptr stmt) {
  addWhileOrDoWhile(stmt, true);
}

void IREmitter::visit(IfStmt::Ptr stmt) {
  const ir::Expr cond = emitExpr(stmt->cond);
  const ir::Stmt callStmts = getCallStmts();
  
  ctx->scope();
  const ir::Stmt ifBody = emitStmt(stmt->ifBody);
  ctx->unscope();

  ir::Stmt elseBody;
  if (stmt->elseBody) {
    ctx->scope();
    elseBody = emitStmt(stmt->elseBody);
    ctx->unscope();
  }

  const ir::Stmt ifStmt = elseBody.defined() ? 
                          ir::IfThenElse::make(cond, ifBody, elseBody) : 
                          ir::IfThenElse::make(cond, ifBody);

  // Function calls and maps in if-statement condition need to be evaluated 
  // before condition expression is evaluated.
  if (callStmts.defined()) {
    ctx->addStatement(callStmts);
  }
  ctx->addStatement(ifStmt);

  retStmt = ifStmt;
}

void IREmitter::visit(IndexSetDomain::Ptr domain) {
  const ir::IndexSet set = emitIndexSet(domain->set);
  retDomain = Domain(set);
}

void IREmitter::visit(RangeDomain::Ptr domain) {
  ir::Stmt callStmts;

  const ir::Expr lower = emitExpr(domain->lower);
  callStmts = getCallStmts();
  
  if (callStmts.defined()) {
    ctx->addStatement(callStmts);
  }

  const ir::Expr upper = emitExpr(domain->upper);
  callStmts = getCallStmts();

  if (callStmts.defined()) {
    ctx->addStatement(callStmts);
  }

  retDomain = Domain(lower, upper);
}

void IREmitter::visit(ForStmt::Ptr stmt) {
  ctx->scope();
  const Domain domain = emitDomain(stmt->domain);

  ir::Var loopVar;
  switch (domain.type) {
    case Domain::Type::SET:
    {
      const ir::Type elemType = domain.set.getSet().type().toSet()->elementType;
      loopVar = ir::Var(stmt->loopVar->ident, elemType);
      break;
    }
    case Domain::Type::RANGE:
      loopVar = ir::Var(stmt->loopVar->ident, ir::Int);
      break;
    default:
      unreachable;
      break;
  }

  // If we need to write to loop variables, then that should be added as a
  // separate loop structure (that can't be vectorized easily)
  ctx->addSymbol(stmt->loopVar->ident, loopVar, internal::Symbol::Read);
 
  const ir::Stmt body = emitStmt(stmt->body);
  ctx->unscope();
  
  ir::Stmt forStmt;
  switch (domain.type) {
    case Domain::Type::SET:
      forStmt = ir::For::make(loopVar, domain.set, body);
      break;
    case Domain::Type::RANGE:
      forStmt = ir::ForRange::make(loopVar, domain.lower, domain.upper, body);
      break;
    default:
      unreachable;
      break;
  }
  ctx->addStatement(forStmt);
}

void IREmitter::visit(PrintStmt::Ptr stmt) {
  const ir::Expr expr = emitExpr(stmt->expr);
  const ir::Stmt callStmts = getCallStmts();
 
  if (callStmts.defined()) {
    ctx->addStatement(callStmts);
  }
  ctx->addStatement(ir::Print::make(expr));
}

void IREmitter::visit(ExprStmt::Ptr stmt) {
  const ir::Expr expr = emitExpr(stmt->expr);
  addAssign({}, expr);
}

void IREmitter::visit(AssignStmt::Ptr stmt) {
  std::vector<ir::Expr> targets;
  for (auto lhs : stmt->lhs) {
    const ir::Expr target = emitExpr(lhs);
    targets.push_back(target);
  }

  const ir::Expr expr = emitExpr(stmt->expr);
  addAssign(targets, expr);
}

void IREmitter::visit(ExprParam::Ptr param) {
  retExpr = emitExpr(param->expr);
}

void IREmitter::visit(MapExpr::Ptr expr) {
  const ir::Func func = ctx->getFunction(expr->func->ident);
  const std::vector<ir::Var> results = func.getResults();

  const ir::Expr target = ctx->getSymbol(expr->target->ident).getExpr();
 
  std::vector<ir::Expr> partialActuals;
  for (auto actual : expr->partialActuals) {
    const ir::Expr param = emitExpr(actual);
    partialActuals.push_back(param);
  }
  
  ir::ReductionOperator reduction;
  switch (expr->op) {
    case MapExpr::ReductionOp::SUM:
      reduction = ir::ReductionOperator::Sum;
      break;
    default:
      reduction = ir::ReductionOperator::Undefined;
      break;
  }
  
  std::vector<ir::Expr> endpoints;
  iassert(target.type().isSet());
  for (const ir::Expr *endpoint : target.type().toSet()->endpointSets) {
    endpoints.push_back(*endpoint);
  }

  // Map expressions are translated to map statements whose values are stored 
  // in temporary variables. Within the original expression in which the map 
  // appeared, the map is replaced with a read of the temporary variable.
  const auto type = (results.size() == 1) ? results[0].getType() : ir::Type();
  const ir::Var tmp = ctx->getBuilder()->temporary(type);
  retExpr = ir::VarExpr::make(tmp);

  // TODO: Should eventually support heterogeneous edge sets.
  const ir::Expr endpoint = (endpoints.size() > 0) ? endpoints[0] : ir::Expr();
  const ir::Stmt mapStmt = ir::Map::make({tmp}, func, partialActuals, target,
                                         endpoint, reduction);
  calls.push_back(mapStmt);
}

void IREmitter::visit(OrExpr::Ptr expr) {
  const ir::Expr lhs = emitExpr(expr->lhs);
  const ir::Expr rhs = emitExpr(expr->rhs);
  retExpr = ir::Or::make(lhs, rhs);
}

void IREmitter::visit(AndExpr::Ptr expr) {
  const ir::Expr lhs = emitExpr(expr->lhs);
  const ir::Expr rhs = emitExpr(expr->rhs);
  retExpr = ir::And::make(lhs, rhs);
}

void IREmitter::visit(XorExpr::Ptr expr) {
  const ir::Expr lhs = emitExpr(expr->lhs);
  const ir::Expr rhs = emitExpr(expr->rhs);
  retExpr = ir::Xor::make(lhs, rhs);
}

void IREmitter::visit(EqExpr::Ptr expr) {
  iassert(expr->operands.size() > 1);
  ir::Expr lhs = emitExpr(expr->operands[0]);

  // Chained (n-ary) comparison operations are translated to conjunction of 
  // binary comparison operations. Since function calls are lifted up, the 
  // emitted program would behave as if each operand is evaluated only once.
  ir::Expr eqExpr;
  for (unsigned i = 0; i < expr->ops.size(); ++i) {
    const ir::Expr rhs = emitExpr(expr->operands[i + 1]);
    
    ir::Expr cmpExpr;
    switch (expr->ops[i]) {
      case EqExpr::Op::LT:
        cmpExpr = ir::Lt::make(lhs, rhs);
        break;
      case EqExpr::Op::LE:
        cmpExpr = ir::Le::make(lhs, rhs);
        break;
      case EqExpr::Op::GT:
        cmpExpr = ir::Gt::make(lhs, rhs);
        break;
      case EqExpr::Op::GE:
        cmpExpr = ir::Ge::make(lhs, rhs);
        break;
      case EqExpr::Op::EQ:
        cmpExpr = ir::Eq::make(lhs, rhs);
        break;
      case EqExpr::Op::NE:
        cmpExpr = ir::Ne::make(lhs, rhs);
        break;
      default:
        unreachable;
        break;
    }

    eqExpr = (i == 0) ? cmpExpr : ir::And::make(eqExpr, cmpExpr);
    lhs = rhs;
  }

  retExpr = eqExpr;
}

void IREmitter::visit(NotExpr::Ptr expr) {
  const ir::Expr operand = emitExpr(expr->operand);
  retExpr = ir::Not::make(operand);
}

void IREmitter::visit(AddExpr::Ptr expr) {
  const ir::Expr lhs = emitExpr(expr->lhs);
  const ir::Expr rhs = emitExpr(expr->rhs);
  retExpr = ctx->getBuilder()->binaryElwiseExpr(lhs, ir::IRBuilder::Add, rhs);
}

void IREmitter::visit(SubExpr::Ptr expr) {
  const ir::Expr lhs = emitExpr(expr->lhs);
  const ir::Expr rhs = emitExpr(expr->rhs);
  retExpr = ctx->getBuilder()->binaryElwiseExpr(lhs, ir::IRBuilder::Sub, rhs);
}

void IREmitter::visit(MulExpr::Ptr expr) {
  ir::IRBuilder *builder = ctx->getBuilder();

  const ir::Expr lhs = emitExpr(expr->lhs);
  const ir::Expr rhs = emitExpr(expr->rhs);
  const auto ltype = lhs.type().toTensor();
  const auto rtype = rhs.type().toTensor();
  const auto ldimensions = ltype->getDimensions();
  const auto rdimensions = rtype->getDimensions();

  iassert(ltype->order() <= 2 && rtype->order() <= 2);
  if (ltype->order() == 0 || rtype->order() == 0) {
    // Scale
    retExpr = builder->binaryElwiseExpr(lhs, ir::IRBuilder::Mul, rhs);
  } else if (ltype->order() == 1 && rtype->order() == 1) {
    // Vector-Vector Multiplication (inner and outer product)
    iassert(lhs.type() == rhs.type());
    if (expr->lhs->type.at(0).toTensor()->isColumnVector) {
      retExpr = builder->outerProduct(lhs, rhs);
    } else {
      retExpr = builder->innerProduct(lhs, rhs);
    }
  } else if (ltype->order() == 2 && rtype->order() == 1) {
    // Matrix-Vector
    iassert(ldimensions[1] == rdimensions[0]);
    retExpr = builder->gemv(lhs, rhs);
  } else if (ltype->order() == 1 && rtype->order() == 2) {
    // Vector-Matrix
    iassert(ldimensions[0] == rdimensions[0]);
    retExpr = builder->gevm(lhs, rhs);
  } else if (ltype->order() == 2 && rtype->order() == 2) {
    // Matrix-Matrix
    iassert(ldimensions[1] == rdimensions[0]);
    retExpr = builder->gemm(lhs, rhs);
  }
}

void IREmitter::visit(DivExpr::Ptr expr) {
  const ir::Expr lhs = emitExpr(expr->lhs);
  const ir::Expr rhs = emitExpr(expr->rhs);
  retExpr = ctx->getBuilder()->binaryElwiseExpr(lhs, ir::IRBuilder::Div, rhs);
}

void IREmitter::visit(ElwiseMulExpr::Ptr expr) {
  const ir::Expr lhs = emitExpr(expr->lhs);
  const ir::Expr rhs = emitExpr(expr->rhs);
  retExpr = ctx->getBuilder()->binaryElwiseExpr(lhs, ir::IRBuilder::Mul, rhs);
}

void IREmitter::visit(ElwiseDivExpr::Ptr expr) {
  const ir::Expr lhs = emitExpr(expr->lhs);
  const ir::Expr rhs = emitExpr(expr->rhs);
  retExpr = ctx->getBuilder()->binaryElwiseExpr(lhs, ir::IRBuilder::Div, rhs);
}

void IREmitter::visit(NegExpr::Ptr expr) {
  const ir::Expr operand = emitExpr(expr->operand);
  iassert(!ir::isa<ir::Literal>(operand));
  retExpr = !expr->negate ? operand : 
            ctx->getBuilder()->unaryElwiseExpr(ir::IRBuilder::Neg, operand);
}

void IREmitter::visit(ExpExpr::Ptr expr) {
  // TODO: Implement.
  not_supported_yet;
}

void IREmitter::visit(TransposeExpr::Ptr expr) {
  ir::IRBuilder *builder = ctx->getBuilder();

  ir::Expr operand = emitExpr(expr->operand);
  const ir::TensorType *type = operand.type().toTensor();

  switch (type->order()) {
    case 0:
      // OPT: This might lead to redundant code to be removed in later pass
      retExpr = builder->unaryElwiseExpr(ir::IRBuilder::None, operand);
      break;
    case 1:
    {
      // OPT: This might lead to redundant code to be removed in later pass
      retExpr = builder->unaryElwiseExpr(ir::IRBuilder::None, operand);
      auto retExprNode = const_cast<ir::ExprNode *>(to<ir::ExprNode>(retExpr));
      
      const bool isColumnVector = expr->type.at(0).toTensor()->isColumnVector;
      retExprNode->type = ir::TensorType::make(type->getComponentType(), 
                                               type->getDimensions(), 
                                               isColumnVector);
      break;
    }
    case 2:
      retExpr = builder->transposedMatrix(operand);
      break;
    default:
      unreachable;
      break;
  }
}

void IREmitter::visit(CallExpr::Ptr expr) {
  const ir::Func func = ctx->getFunction(expr->func->ident);
  const std::vector<ir::Var> results = func.getResults();

  std::vector<ir::Expr> arguments;
  for (auto argument : expr->arguments) {
    iassert((bool)argument);
    const ir::Expr arg = emitExpr(argument);
    arguments.push_back(arg);
  }

  // Function calls are translated to call statements whose values are stored 
  // in temporary variables. Within the original expression in which the call 
  // appeared, the call is replaced with a read of the temporary variable.
  const auto type = (results.size() == 1) ? results[0].getType() : ir::Type();
  const ir::Var tmp = ctx->getBuilder()->temporary(type);
  retExpr = ir::VarExpr::make(tmp);

  const ir::Stmt callStmt = ir::CallStmt::make({tmp}, func, arguments);
  calls.push_back(callStmt);
}

void IREmitter::visit(TensorReadExpr::Ptr expr) {
  const ir::Expr tensor = emitExpr(expr->tensor);
  iassert(tensor.type().isTensor());

  std::vector<ir::Expr> indices;
  bool containsSlices = false;
  for (auto param : expr->indices) {
    const ir::Expr arg = emitExpr(param);
    indices.push_back(arg);
    if (param->isSlice()) {
      containsSlices = true;
    }
  }

  // If expression is a tensor slice, then translate to index expression.
  if (containsSlices) {
    // We will construct an index expression. First, we built IndexVars.
    std::vector<ir::IndexVar> allivars;
    std::vector<ir::IndexVar> freeVars;
    std::vector<ir::IndexDomain> dimensions = 
      tensor.type().toTensor()->getDimensions();

    unsigned i = 0;
    for (auto &arg : indices) {
      if (expr->indices[i]->isSlice()) {
        auto iv = ir::IndexVar("@tmpfree" + std::to_string(i), dimensions[i]);
        allivars.push_back(iv);
        freeVars.push_back(iv);
      } else {
        auto iv = ir::IndexVar("@tmpfixed" + std::to_string(i),
                               dimensions[i], new ir::Expr(arg));
        allivars.push_back(iv);
      }
      ++i;
    }

    // Now construct an index expression.
    retExpr = ir::IndexExpr::make(freeVars,
                                  ir::IndexedTensor::make(tensor, allivars));
  } else {
    retExpr = ir::TensorRead::make(tensor, indices);
  }
}

void IREmitter::visit(TupleReadExpr::Ptr expr) {
  const ir::Expr tuple = emitExpr(expr->tuple);
  const ir::Expr index = emitExpr(expr->index);

  iassert(tuple.type().isTuple());
  retExpr = ir::TupleRead::make(tuple, index);
}

void IREmitter::visit(FieldReadExpr::Ptr expr) {
  const ir::Expr lhs = emitExpr(expr->setOrElem);
  const ir::Type type = lhs.type();

  iassert(type.isElement() || type.isSet());
  const ir::ElementType *elemType = type.isElement() ? type.toElement() :
                                    type.toSet()->elementType.toElement();
 
  iassert(elemType->hasField(expr->field->ident));
  retExpr = ir::FieldRead::make(lhs, expr->field->ident);
}

void IREmitter::visit(VarExpr::Ptr expr) {
  retExpr = ctx->hasSymbol(expr->ident) ? 
            ctx->getSymbol(expr->ident).getExpr() :
            ir::VarExpr::make(ir::Var(expr->ident, ir::Type()));
}

void IREmitter::visit(IntLiteral::Ptr lit) {
  retExpr = ir::Literal::make(lit->val);
}

void IREmitter::visit(FloatLiteral::Ptr lit) {
  retExpr = ir::Literal::make(lit->val);
}

void IREmitter::visit(BoolLiteral::Ptr lit) {
  retExpr = ir::Literal::make(lit->val);
}

void IREmitter::visit(IntVectorLiteral::Ptr lit) {
  emitDenseTensorLiteral(lit);
}

void IREmitter::visit(FloatVectorLiteral::Ptr lit) {
  emitDenseTensorLiteral(lit);
}

void IREmitter::visit(NDTensorLiteral::Ptr lit) {
  emitDenseTensorLiteral(lit);
}

void IREmitter::emitDenseTensorLiteral(DenseTensorLiteral::Ptr tensor) {
  const DenseTensorValues tensorVals = emitTensorValues(tensor);
  const std::vector<ir::IndexDomain> idoms(tensorVals.dimSizes.rbegin(),
                                           tensorVals.dimSizes.rend());
  const auto elemType = (tensorVals.type == DenseTensorValues::Type::INT) ?
                        ir::ScalarType::Int : ir::ScalarType::Float;
  
  const ir::Type tensorType = ir::TensorType::make(elemType, idoms, 
                                                   tensor->transposed);
  const void *data = (tensorVals.type == DenseTensorValues::Type::INT) ? 
                     static_cast<const void *>(tensorVals.intVals.data()) :
                     static_cast<const void *>(tensorVals.floatVals.data());
  retExpr = ir::Literal::make(tensorType, const_cast<void *>(data));
}

IREmitter::DenseTensorValues 
    IREmitter::emitTensorValues(DenseTensorLiteral::Ptr lit) {
  DenseTensorValues tensorVals;
  
  if (isa<IntVectorLiteral>(lit)) {
    tensorVals.addIntValues(to<IntVectorLiteral>(lit)->vals);
  } else if (isa<FloatVectorLiteral>(lit)) {
    tensorVals.addFloatValues(to<FloatVectorLiteral>(lit)->vals);
  } else {
    const auto ndTensorLit = to<NDTensorLiteral>(lit);
  
    tensorVals = emitTensorValues(ndTensorLit->elems[0]);
    tensorVals.addDimension();
  
    for (unsigned i = 1; i < ndTensorLit->elems.size(); ++i) {
      const DenseTensorValues right = emitTensorValues(ndTensorLit->elems[i]);
      tensorVals.merge(right);
    }
  }

  return tensorVals;
}

void IREmitter::visit(Test::Ptr test) {
  std::vector<ir::Expr> args;
  for (auto operand : test->args) {
    const ir::Expr arg = emitExpr(operand);
    args.push_back(arg);
  }
  
  const ir::Expr expected = emitExpr(test->expected);

  iassert(calls.empty());
  ctx->addTest(new internal::FunctionTest(test->func->ident, args, {expected}));
}

void IREmitter::DenseTensorValues::addIntValues(const std::vector<int> &vals) {
  iassert(type != Type::FLOAT);
  type = Type::INT;
  
  intVals.insert(intVals.end(), vals.begin(), vals.end());
  dimSizes[dimSizes.size() - 1] += vals.size();
}

void IREmitter::DenseTensorValues::addFloatValues(
    const std::vector<double> &vals) {
  iassert(type != Type::INT);
  type = Type::FLOAT;
  
  floatVals.insert(floatVals.end(), vals.begin(), vals.end());
  dimSizes[dimSizes.size() - 1] += vals.size();
}

void IREmitter::DenseTensorValues::merge(
    const IREmitter::DenseTensorValues &other) {
  iassert(type == other.type);
  iassert(dimSizes.size() - 1 == other.dimSizes.size());

  switch (type) {
    case Type::INT:
      intVals.insert(intVals.end(), other.intVals.begin(), 
                     other.intVals.end());
      break;
    case Type::FLOAT:
      floatVals.insert(floatVals.end(), other.floatVals.begin(), 
                       other.floatVals.end());
      break;
    default:
      unreachable;
      break;
  }
  ++dimSizes[dimSizes.size() - 1];
}

void IREmitter::addFuncOrProc(FuncDecl::Ptr decl, bool isProc) {
  ctx->scope();

  std::vector<ir::Var> arguments;
  for (auto arg : decl->args) {
    const ir::Var argVar = emitVar(arg->arg);
    const auto access = arg->inout ? internal::Symbol::ReadWrite : 
                        internal::Symbol::Read;
    ctx->addSymbol(argVar.getName(), argVar, access);
    arguments.push_back(argVar);
  }
  
  std::vector<ir::Var> results;
  for (auto res : decl->results) {
    const ir::Var result = emitVar(res);
    ctx->addSymbol(result);
    results.push_back(result);
  }

  const ir::Stmt body = emitStmt(decl->body);
  ctx->unscope();

  if (isProc) {
    for (auto &extPair : ctx->getExterns()) {
      const ir::Var ext = ctx->getExtern(extPair.first);
      arguments.push_back(ext);
    }
  }

  iassert(!ctx->containsFunction(decl->name->ident));
  ctx->addFunction(ir::Func(decl->name->ident, arguments, results, body));
}

void IREmitter::addVarOrConst(VarDecl::Ptr decl, bool isConst) {
  const ir::Var var = emitVar(decl->var);
  
  iassert(!ctx->hasSymbol(var.getName(), true));
  const auto access = isConst ? internal::Symbol::Read : 
                      internal::Symbol::ReadWrite;
  ctx->addSymbol(var.getName(), var, access);

  iassert(decl->initVal || !isConst);
  const auto initExpr = decl->initVal ? emitExpr(decl->initVal) : ir::Expr();
  
  if (isConst && initExpr.defined() && ir::isa<ir::Literal>(initExpr) && 
      (isScalar(var.getType()) || !isScalar(initExpr.type()))) {
    // Optimization to avoid having to initialize constant multiple times. This 
    // can be performed as long as constant is a literal, assuming that the 
    // constant is scalar or that it is initialized to a non-scalar (i.e. as 
    // long as a non-scalar constant is not initialized to a constant).
    ctx->addConstant(var, initExpr);
  } else {
    ctx->addStatement(ir::VarDecl::make(var));
    
    if (decl->initVal) {
      addAssign({ir::VarExpr::make(var)}, initExpr);
      calls.clear();
    }
  }
}

void IREmitter::addWhileOrDoWhile(WhileStmt::Ptr stmt, bool isDoWhile) {
  const ir::Expr cond = emitExpr(stmt->cond);
  const ir::Stmt callStmts = getCallStmts();
  
  ctx->scope();
  ir::Stmt body = emitStmt(stmt->body);
  ctx->unscope();
 
  if (isDoWhile) {
    ctx->addStatement(body);
  }
  if (callStmts.defined()) {
    ctx->addStatement(callStmts);
    body = ir::Block::make(body, callStmts);
  }
  ctx->addStatement(ir::While::make(cond, body));
}

void IREmitter::addAssign(const std::vector<ir::Expr> &lhs, ir::Expr expr) {
  ir::Stmt topLevelStmt;
  if (ir::isa<ir::VarExpr>(expr) && !calls.empty()) {
    const ir::Var var = ir::to<ir::VarExpr>(expr)->var;
    if (ir::isa<ir::CallStmt>(calls.back())) {
      const auto callStmt = ir::to<ir::CallStmt>(calls.back());
      if (var == callStmt->results[0]) {
        topLevelStmt = callStmt;
        calls.pop_back();
      }
    } else {
      const auto mapStmt = ir::to<ir::Map>(calls.back());
      if (var == mapStmt->vars[0]) {
        topLevelStmt = mapStmt;
        calls.pop_back();
      }
    }
  }

  const ir::Stmt callStmts = getCallStmts();
  if (callStmts.defined()) {
    ctx->addStatement(callStmts);
  }

  if (topLevelStmt.defined()) {
    const bool isCallStmt = ir::isa<ir::CallStmt>(topLevelStmt);
    const std::vector<ir::Var> retVals = isCallStmt ? 
      ir::to<ir::CallStmt>(topLevelStmt)->callee.getResults() :
      ir::to<ir::Map>(topLevelStmt)->function.getResults();
    
    iassert(lhs.size() <= retVals.size());

    // If target of assignment is a variable, then have function call or map 
    // statement directly store into that variable. If target of assignment is 
    // not a variable (e.g. field element) or if statement is not an 
    // assignment, then store the result into a temporary variable.
    std::vector<ir::Var> results;
    for (unsigned i = 0; i < retVals.size(); ++i) {
      if (i < lhs.size() && ir::isa<ir::VarExpr>(lhs[i])) {
        ir::Var var = ir::to<ir::VarExpr>(lhs[i])->var;
        const std::string varName = var.getName();

        if (!ctx->hasSymbol(varName)) {
          var = ir::Var(varName, retVals[i].getType());
          ctx->addSymbol(var);
          ctx->addStatement(ir::VarDecl::make(var));
        }

        results.push_back(var);
      } else {
        const ir::Var tmp = ctx->getBuilder()->temporary(retVals[i].getType());
        ctx->addSymbol(tmp);
        ctx->addStatement(ir::VarDecl::make(tmp));
        results.push_back(tmp);
      }
    }
    
    // Emit call or map statement.
    if (isCallStmt) {
      auto callStmt = const_cast<ir::CallStmt *>(
          ir::to<ir::CallStmt>(topLevelStmt));
      callStmt->results = results;
      ctx->addStatement(callStmt);
    } else {
      auto mapStmt = const_cast<ir::Map *>(ir::to<ir::Map>(topLevelStmt));
      mapStmt->vars = results;
      ctx->addStatement(mapStmt);
    }
    
    // If target of assignment is a field or tensor element, then have output 
    // of function call or map statement copied over from temporary variable.
    for (unsigned int i = 0; i < lhs.size(); ++i) {
      ir::Expr tmpExpr = ir::VarExpr::make(results[i]);
      if (!isScalar(tmpExpr.type())) {
        // Needed to handle assignment of non-scalar tensors.
        tmpExpr = ctx->getBuilder()->unaryElwiseExpr(ir::IRBuilder::None, 
                                                     tmpExpr);
      }
      
      if (ir::isa<ir::FieldRead>(lhs[i])) {
        const ir::FieldRead *fieldRead = ir::to<ir::FieldRead>(lhs[i]);
        const ir::Stmt fieldWrite = ir::FieldWrite::make(
            fieldRead->elementOrSet, fieldRead->fieldName, tmpExpr);
        ctx->addStatement(fieldWrite);
      } else if (ir::isa<ir::TensorRead>(lhs[i])) {
        const ir::TensorRead *tensorRead = ir::to<ir::TensorRead>(lhs[i]);
        const ir::Stmt tensorWrite = ir::TensorWrite::make(
            tensorRead->tensor, tensorRead->indices, tmpExpr);
        ctx->addStatement(tensorWrite);
      }
    }
  } else if (lhs.size() == 1) {
    if (!ir::isa<ir::IndexExpr>(expr) && !isScalar(expr.type())) {
      // Needed to handle assignment of non-scalar tensors.
      expr = ctx->getBuilder()->unaryElwiseExpr(ir::IRBuilder::None, expr);
    }

    if (ir::isa<ir::FieldRead>(lhs[0])) {
      const ir::FieldRead *fieldRead = ir::to<ir::FieldRead>(lhs[0]);
      const ir::Stmt fieldWrite = ir::FieldWrite::make(
          fieldRead->elementOrSet, fieldRead->fieldName, expr);
      ctx->addStatement(fieldWrite);
    } else if (ir::isa<ir::TensorRead>(lhs[0])) {
      const ir::TensorRead *tensorRead = ir::to<ir::TensorRead>(lhs[0]);
      const ir::Stmt tensorWrite = ir::TensorWrite::make(
          tensorRead->tensor, tensorRead->indices, expr);
      ctx->addStatement(tensorWrite);
    } else {
      ir::Var var = ir::to<ir::VarExpr>(lhs[0])->var;
      const std::string varName = var.getName();

      // Target variable might not have been declared.
      if (!ctx->hasSymbol(varName)) {
        var = ir::Var(varName, expr.type());
        ctx->addSymbol(varName, var, internal::Symbol::ReadWrite);
      }
  
      ctx->addStatement(ir::AssignStmt::make(var, expr));
    }
  } else {
    iassert(lhs.size() == 0);
  }
}

ir::Stmt IREmitter::getCallStmts() {
  if (calls.empty()) {
    return ir::Stmt();
  }

  const ir::Stmt callStmts = ir::Block::make(calls);
  
  calls.clear();
  return callStmts;
}

}
}

