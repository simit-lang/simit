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

  retType = ir::Type(ir::SetType::make(elementType, endpoints));
}

void IREmitter::visit(TupleType::Ptr type) {
  iassert(type->length > 0);
  const ir::Type elementType = emitType(type->element);
  retType = ir::Type(ir::TupleType::make(elementType, type->length->val));
}

void IREmitter::visit(ScalarTensorType::Ptr type) {
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
      unreachable;
      break;
  }
  
  retType = ir::Type(ir::TensorType::make(componentType));
}

void IREmitter::visit(NonScalarTensorType::Ptr type) {
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
  
    retType = ir::Type(ir::TensorType::make(componentType, dimensions));
  }

  if (type->transposed) {
    const auto tensorType = retType.toTensor();
    const auto dimensions = tensorType->getDimensions();
    const auto componentType = tensorType->componentType;
    retType = ir::Type(ir::TensorType::make(componentType, dimensions, true));
  }
}

void IREmitter::visit(Field::Ptr field) {
  const ir::Type type = emitType(field->type);
  retField = ir::Field(field->name->ident, type);
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

void IREmitter::visit(IdentDecl::Ptr decl) {
  const ir::Type type = emitType(decl->type);
  retVar = ir::Var(decl->name->ident, type);
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
 
  if (callStmts.defined()) {
    ctx->addStatement(callStmts);
  }
  ctx->addStatement(ifStmt);

  retStmt = ifStmt;
}

void IREmitter::visit(IndexSetDomain::Ptr domain) {
  const ir::IndexSet set = emitIndexSet(domain->set);
  retDomain = Domain::make(set);
}

void IREmitter::visit(RangeDomain::Ptr domain) {
  const ir::Expr lower = emitExpr(domain->lower);
  ctx->addStatement(getCallStmts());

  const ir::Expr upper = emitExpr(domain->upper);
  ctx->addStatement(getCallStmts());

  retDomain = Domain::make(lower, upper);
}

void IREmitter::visit(ForStmt::Ptr stmt) {
  ctx->scope();
  
  const Domain domain = emitDomain(stmt->domain);

  // If we need to write to loop variables, then that should be added as a
  // separate loop structure (that can't be vectorized easily)
  const ir::Var loopVar = ir::Var(stmt->loopVar->ident, ir::Int);
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
  for (ir::Expr *endpoint : target.type().toSet()->endpointSets) {
    endpoints.push_back(*endpoint);
  }

  // We assume the edge set is homogeneous for now
  const ir::Expr endpoint = (endpoints.size() > 0) ? endpoints[0] : ir::Expr();
  
  const auto type = (results.size() == 1) ? results[0].getType() : ir::Type();
  const ir::Var tmp = ctx->getBuilder()->temporary(type);
  retExpr = ir::VarExpr::make(tmp);
 
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
    iassert(ltype->isColumnVector != rtype->isColumnVector);
    retExpr = ltype->isColumnVector ? builder->outerProduct(lhs, rhs) :
              builder->innerProduct(lhs, rhs);
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

  iassert(type->order() > 1 || !ir::isa<ir::Literal>(operand));
  switch (type->order()) {
    case 0:
      // OPT: This might lead to redundant code to be removed in later pass
      retExpr = builder->unaryElwiseExpr(ir::IRBuilder::None, operand);
      break;
    case 1:
    {
      // OPT: This might lead to redundant code to be removed in later pass
      retExpr = builder->unaryElwiseExpr(ir::IRBuilder::None, operand);
      const ir::Type transposedVector = ir::TensorType::make(
        type->componentType, type->getDimensions(), !type->isColumnVector);
      const_cast<ir::ExprNodeBase *>(to<ir::ExprNodeBase>(retExpr))->type = 
        transposedVector;
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
  for (auto operand : expr->operands) {
    const ir::Expr arg = emitExpr(operand);
    arguments.push_back(arg);
  }

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

  if (containsSlices) {
    // We will construct an index expression. First, we built IndexVars.
    std::vector<ir::IndexVar> allivars;
    std::vector<ir::IndexVar> freeVars;
    std::vector<ir::IndexDomain> dimensions = 
      tensor.type().toTensor()->getDimensions();

    unsigned i = 0;
    for (auto &arg : indices) {
      if (expr->indices[i]->isSlice()) {
        auto iv = ir::IndexVar("tmpfree" + std::to_string(i), dimensions[i]);
        allivars.push_back(iv);
        freeVars.push_back(iv);
      } else {
        auto iv = ir::IndexVar("tmpfixed" + std::to_string(i),
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

void IREmitter::visit(IntLiteral::Ptr expr) {
  retExpr = ir::Literal::make(expr->val);
}

void IREmitter::visit(FloatLiteral::Ptr expr) {
  retExpr = ir::Literal::make(expr->val);
}

void IREmitter::visit(BoolLiteral::Ptr expr) {
  retExpr = ir::Literal::make(expr->val);
}

void IREmitter::visit(DenseIntVector::Ptr expr) {
  // TODO: Optimize.
  for (auto val : expr->vals) {
    retTensorVals.addIntValue(val);
  }
}

void IREmitter::visit(DenseFloatVector::Ptr expr) {
  // TODO: Optimize.
  for (auto val : expr->vals) {
    retTensorVals.addFloatValue(val);
  }
}

void IREmitter::visit(DenseNDTensor::Ptr tensor) {
  iassert(tensor->elems.size() > 1);
  
  TensorValues tensorVals = emitTensorVals(tensor->elems[0]);
  tensorVals.addDimension();

  for (unsigned i = 1; i < tensor->elems.size(); ++i) {
    const TensorValues right = emitTensorVals(tensor->elems[i]);
    tensorVals.merge(right);
  }

  retTensorVals = tensorVals;
}

void IREmitter::visit(DenseTensorLiteral::Ptr tensor) {
  const TensorValues tensorVals = emitTensorVals(tensor->tensor);
  const std::vector<ir::IndexDomain> idoms(tensorVals.dimSizes.rbegin(),
                                           tensorVals.dimSizes.rend());
  const ir::ScalarType elemType = (tensorVals.type == TensorValues::Type::INT) ?
                                  ir::ScalarType::Int : ir::ScalarType::Float;
  const ir::Type tensorType = ir::TensorType::make(elemType, idoms, 
                                                   tensor->transposed);
  const void *data = (tensorVals.type == TensorValues::Type::INT) ? 
                     static_cast<const void *>(tensorVals.intVals.data()) :
                     static_cast<const void *>(tensorVals.floatVals.data());
  retExpr = ir::Literal::make(tensorType, const_cast<void *>(data));
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

void IREmitter::addFuncOrProc(FuncDecl::Ptr decl, const bool isProc) {
  ctx->scope();

  std::vector<ir::Var> arguments;
  for (auto arg : decl->args) {
    const ir::Var argVar = emitVar(arg);
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

void IREmitter::addVarOrConst(VarDecl::Ptr decl, const bool isConst) {
  const ir::Var var = emitVar(decl->var);
  
  iassert(!ctx->hasSymbol(var.getName(), true));
  const auto access = isConst ? internal::Symbol::Read : 
                      internal::Symbol::ReadWrite;
  ctx->addSymbol(var.getName(), var, access);

  const auto initExpr = decl->initVal ? emitExpr(decl->initVal) : ir::Expr();
  if (isConst && initExpr.defined() && ir::isa<ir::Literal>(initExpr)) {
    ctx->addConstant(var, initExpr);
  } else {
    ctx->addStatement(ir::VarDecl::make(var));
    
    iassert(decl->initVal || !isConst);
    if (decl->initVal) {
      addAssign({ir::VarExpr::make(var)}, initExpr);
      calls.clear();
    }
  }
}

void IREmitter::addWhileOrDoWhile(WhileStmt::Ptr stmt, const bool isDoWhile) {
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
    
    for (unsigned int i = 0; i < lhs.size(); ++i) {
      const ir::Expr tmpVarExpr = ir::VarExpr::make(results[i]);
      if (ir::isa<ir::FieldRead>(lhs[i])) {
        const ir::FieldRead *fieldRead = ir::to<ir::FieldRead>(lhs[i]);
        const ir::Stmt fieldWrite = ir::FieldWrite::make(
          fieldRead->elementOrSet, fieldRead->fieldName, tmpVarExpr);
        ctx->addStatement(fieldWrite);
      } else if (ir::isa<ir::TensorRead>(lhs[i])) {
        const ir::TensorRead *tensorRead = ir::to<ir::TensorRead>(lhs[i]);
        const ir::Stmt tensorWrite = ir::TensorWrite::make(tensorRead->tensor,
          tensorRead->indices, tmpVarExpr);
        ctx->addStatement(tensorWrite);
      }
    }
  } else if (lhs.size() == 1) {
    if (ir::isa<ir::FieldRead>(lhs[0])) {
      const ir::FieldRead *fieldRead = ir::to<ir::FieldRead>(lhs[0]);
      const ir::Stmt fieldWrite = ir::FieldWrite::make(
        fieldRead->elementOrSet, fieldRead->fieldName, expr);
      ctx->addStatement(fieldWrite);
    } else if (ir::isa<ir::TensorRead>(lhs[0])) {
      const ir::TensorRead *tensorRead = ir::to<ir::TensorRead>(lhs[0]);
      const ir::Stmt tensorWrite = ir::TensorWrite::make(tensorRead->tensor, 
        tensorRead->indices, expr);
      ctx->addStatement(tensorWrite);
    } else {
      ir::Var var = ir::to<ir::VarExpr>(lhs[0])->var;
      const std::string varName = var.getName();
  
      if (!ctx->hasSymbol(varName)) {
        var = ir::Var(varName, expr.type());
        ctx->addSymbol(varName, var, internal::Symbol::ReadWrite);
      }
  
      if (ir::isa<ir::VarExpr>(expr) && expr.type().isTensor()) {
        // The statement assign a tensor to a tensor, so we change it to an
        // assignment index expr.
        expr = ctx->getBuilder()->unaryElwiseExpr(ir::IRBuilder::None, expr);
      }
  
      ctx->addStatement(ir::AssignStmt::make(var, expr));
    }
  } else {
    iassert(lhs.size() == 0);
  }
}

}
}

