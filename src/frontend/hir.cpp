#include <memory>

#include "hir.h"
#include "token.h"
#include "program_context.h"

namespace simit {
namespace hir {

void HIRNode::setBeginLoc(const internal::Token &token) {
  lineBegin = token.lineBegin;
  colBegin = token.colBegin;
}

void HIRNode::setEndLoc(const internal::Token &token) {
  lineEnd = token.lineEnd;
  colEnd = token.colEnd;
}

void HIRNode::setLoc(const internal::Token &token) {
  setBeginLoc(token);
  setEndLoc(token);
}

void HIRNode::setLoc(HIRNode::Ptr node) {
  lineBegin = node->lineBegin;
  colBegin = node->colBegin;
  lineEnd = node->lineEnd;
  colEnd = node->colEnd;
}

void Program::copy(HIRNode::Ptr node) {
  const auto program = to<Program>(node);
  HIRNode::copy(program);
  for (const auto &elem : program->elems) {
    elems.push_back(elem->clone());
  }
}

HIRNode::Ptr Program::cloneImpl() {
  const auto node = std::make_shared<Program>();
  node->copy(shared_from_this());
  return node;
}

void StmtBlock::copy(HIRNode::Ptr node) {
  const auto stmtBlock = to<StmtBlock>(node);
  Stmt::copy(stmtBlock);
  for (const auto &stmt : stmtBlock->stmts) {
    stmts.push_back(stmt->clone<Stmt>());
  }
}

HIRNode::Ptr StmtBlock::cloneImpl() {
  const auto node = std::make_shared<StmtBlock>();
  node->copy(shared_from_this());
  return node;
}

void RangeIndexSet::copy(HIRNode::Ptr node) {
  const auto indexSet = to<RangeIndexSet>(node);
  IndexSet::copy(indexSet);
  range = indexSet->range;
}

HIRNode::Ptr RangeIndexSet::cloneImpl() {
  const auto node = std::make_shared<RangeIndexSet>();
  node->copy(shared_from_this());
  return node;
}

void SetIndexSet::copy(HIRNode::Ptr node) {
  const auto indexSet = to<SetIndexSet>(node);
  IndexSet::copy(indexSet);
  setName = indexSet->setName;
}

HIRNode::Ptr SetIndexSet::cloneImpl() {
  const auto node = std::make_shared<SetIndexSet>();
  node->copy(shared_from_this());
  return node;
}

void GenericIndexSet::copy(HIRNode::Ptr node) {
  const auto indexSet = to<GenericIndexSet>(node);
  SetIndexSet::copy(indexSet);
  type = indexSet->type;
}

HIRNode::Ptr GenericIndexSet::cloneImpl() {
  const auto node = std::make_shared<GenericIndexSet>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr DynamicIndexSet::cloneImpl() {
  const auto node = std::make_shared<DynamicIndexSet>();
  node->copy(shared_from_this());
  return node;
}

void ElementType::copy(HIRNode::Ptr node) {
  // TODO: Note that we don't bother copying source index set node since it is 
  // not needed for type checking, and doing so would only complicate things.
  const auto elementType = to<ElementType>(node);
  Type::copy(elementType);
  ident = elementType->ident;
}

HIRNode::Ptr ElementType::cloneImpl() {
  const auto node = std::make_shared<ElementType>();
  node->copy(shared_from_this());
  return node;
}

void Endpoint::copy(HIRNode::Ptr node) {
  const auto endpoint = to<Endpoint>(node);
  HIRNode::copy(endpoint);
  set = endpoint->set->clone<SetIndexSet>();
  element = endpoint->element->clone<ElementType>();
}

HIRNode::Ptr Endpoint::cloneImpl() {
  const auto node = std::make_shared<Endpoint>();
  node->copy(shared_from_this());
  return node;
}

void SetType::copy(HIRNode::Ptr node) {
  const auto setType = to<SetType>(node);
  Type::copy(setType);
  element = setType->element->clone<ElementType>();
  for (const auto &endpoint : setType->endpoints) {
    endpoints.push_back(endpoint->clone<Endpoint>());
  }
}

HIRNode::Ptr SetType::cloneImpl() {
  const auto node = std::make_shared<SetType>();
  node->copy(shared_from_this());
  return node;
}

void TupleLength::copy(HIRNode::Ptr node) {
  const auto length = to<TupleLength>(node);
  HIRNode::copy(length);
  val = length->val;
}

HIRNode::Ptr TupleLength::cloneImpl() {
  const auto node = std::make_shared<TupleLength>();
  node->copy(shared_from_this());
  return node;
}

void TupleType::copy(HIRNode::Ptr node) {
  const auto tupleType = to<TupleType>(node);
  Type::copy(tupleType);
  element = tupleType->element->clone<ElementType>();
  length = tupleType->length->clone<TupleLength>();
}

HIRNode::Ptr TupleType::cloneImpl() {
  const auto node = std::make_shared<TupleType>();
  node->copy(shared_from_this());
  return node;
}

ScalarType::Ptr make(ScalarType::Type type) {
  auto ret = std::make_shared<ScalarType>();
  ret->type = type;
  return ret;
}

void ScalarType::copy(HIRNode::Ptr node) {
  const auto scalarType = to<ScalarType>(node);
  TensorType::copy(scalarType);
  type = scalarType->type;
}

HIRNode::Ptr ScalarType::cloneImpl() {
  const auto node = std::make_shared<ScalarType>();
  node->copy(shared_from_this());
  return node;
}

void NDTensorType::copy(HIRNode::Ptr node) {
  const auto ndTensorType = to<NDTensorType>(node);
  TensorType::copy(ndTensorType);
  for (const auto &indexSet : ndTensorType->indexSets) {
    indexSets.push_back(indexSet->clone<IndexSet>());
  }
  blockType = ndTensorType->blockType;
  transposed = ndTensorType->transposed;
}

HIRNode::Ptr NDTensorType::cloneImpl() {
  const auto node = std::make_shared<NDTensorType>();
  node->copy(shared_from_this());
  return node;
}

void Identifier::copy(HIRNode::Ptr node) {
  const auto identifier = to<Identifier>(node);
  HIRNode::copy(identifier);
  ident = identifier->ident;
}

HIRNode::Ptr Identifier::cloneImpl() {
  const auto node = std::make_shared<Identifier>();
  node->copy(shared_from_this());
  return node;
}

IdentDecl::Ptr make(Identifier::Ptr name, Type::Ptr type) {
  const auto ret = std::make_shared<IdentDecl>();
  ret->name = name;
  ret->type = type;
  return ret;
}

void IdentDecl::copy(HIRNode::Ptr node) {
  const auto identDecl = to<IdentDecl>(node);
  HIRNode::copy(identDecl);
  name = identDecl->name->clone<Identifier>();
  type = identDecl->type->clone<Type>();
}

HIRNode::Ptr IdentDecl::cloneImpl() {
  const auto node = std::make_shared<IdentDecl>();
  node->copy(shared_from_this());
  return node;
}

void FieldDecl::copy(HIRNode::Ptr node) {
  const auto fieldDecl = to<FieldDecl>(node);
  HIRNode::copy(fieldDecl);
  field = fieldDecl->field->clone<IdentDecl>();
}

HIRNode::Ptr FieldDecl::cloneImpl() {
  const auto node = std::make_shared<FieldDecl>();
  node->copy(shared_from_this());
  return node;
}

void ElementTypeDecl::copy(HIRNode::Ptr node) {
  const auto elementTypeDecl = to<ElementTypeDecl>(node);
  HIRNode::copy(elementTypeDecl);
  name = elementTypeDecl->name->clone<Identifier>();
  for (const auto &field : elementTypeDecl->fields) {
    fields.push_back(field->clone<FieldDecl>());
  }
}

HIRNode::Ptr ElementTypeDecl::cloneImpl() {
  const auto node = std::make_shared<ElementTypeDecl>();
  node->copy(shared_from_this());
  return node;
}

Argument::Ptr make(IdentDecl::Ptr arg) {
  const auto ret = std::make_shared<Argument>();
  ret->arg = arg;
  return ret;
}

void Argument::copy(HIRNode::Ptr node) {
  const auto argument = to<Argument>(node);
  HIRNode::copy(argument);
  arg = argument->arg->clone<IdentDecl>();
}

HIRNode::Ptr Argument::cloneImpl() {
  const auto node = std::make_shared<Argument>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr InOutArgument::cloneImpl() {
  const auto node = std::make_shared<InOutArgument>();
  node->copy(shared_from_this());
  return node;
}

void ExternDecl::copy(HIRNode::Ptr node) {
  const auto externDecl = to<ExternDecl>(node);
  HIRNode::copy(externDecl);
  var = externDecl->var->clone<IdentDecl>();
}

HIRNode::Ptr ExternDecl::cloneImpl() {
  const auto node = std::make_shared<ExternDecl>();
  node->copy(shared_from_this());
  return node;
}

void GenericParam::copy(HIRNode::Ptr node) {
  const auto genericParam = to<GenericParam>(node);
  HIRNode::copy(genericParam);
  name = genericParam->name;
  type = genericParam->type;
}

HIRNode::Ptr GenericParam::cloneImpl() {
  const auto node = std::make_shared<GenericParam>();
  node->copy(shared_from_this());
  return node;
}

void FuncDecl::copy(HIRNode::Ptr node) {
  const auto funcDecl = to<FuncDecl>(node);
  HIRNode::copy(funcDecl);
  name = funcDecl->name->clone<Identifier>();
  for (const auto &genericParam : funcDecl->genericParams) {
    genericParams.push_back(genericParam->clone<GenericParam>());
  }
  for (const auto &arg : funcDecl->args) {
    args.push_back(arg->clone<Argument>());
  }
  for (const auto &result : funcDecl->results) {
    results.push_back(result->clone<IdentDecl>());
  }
  body = funcDecl->body->clone<StmtBlock>();
  exported = funcDecl->exported;
  originalName = funcDecl->originalName;
}

HIRNode::Ptr FuncDecl::cloneImpl() {
  const auto node = std::make_shared<FuncDecl>();
  node->copy(shared_from_this());
  return node;
}

void VarDecl::copy(HIRNode::Ptr node) {
  const auto varDecl = to<VarDecl>(node);
  Stmt::copy(varDecl);
  name = varDecl->name->clone<Identifier>();
  if (varDecl->type) {
    type = varDecl->type->clone<Type>();
  }
  if (varDecl->initVal) {
    initVal = varDecl->initVal->clone<Expr>();
  }
}

HIRNode::Ptr VarDecl::cloneImpl() {
  const auto node = std::make_shared<VarDecl>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr ConstDecl::cloneImpl() {
  const auto node = std::make_shared<ConstDecl>();
  node->copy(shared_from_this());
  return node;
}

void WhileStmt::copy(HIRNode::Ptr node) {
  const auto whileStmt = to<WhileStmt>(node);
  Stmt::copy(whileStmt);
  cond = whileStmt->cond->clone<Expr>(); 
  body = whileStmt->body->clone<StmtBlock>();
}

HIRNode::Ptr WhileStmt::cloneImpl() {
  const auto node = std::make_shared<WhileStmt>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr DoWhileStmt::cloneImpl() {
  const auto node = std::make_shared<DoWhileStmt>();
  node->copy(shared_from_this());
  return node;
}

void IfStmt::copy(HIRNode::Ptr node) {
  const auto ifStmt = to<IfStmt>(node);
  Stmt::copy(ifStmt);
  cond = ifStmt->cond->clone<Expr>(); 
  ifBody = ifStmt->ifBody->clone<Stmt>();
  if (ifStmt->elseBody) {
    elseBody = ifStmt->elseBody->clone<Stmt>();
  }
}

HIRNode::Ptr IfStmt::cloneImpl() {
  const auto node = std::make_shared<IfStmt>();
  node->copy(shared_from_this());
  return node;
}

void IndexSetDomain::copy(HIRNode::Ptr node) {
  const auto indexSetDomain = to<IndexSetDomain>(node);
  ForDomain::copy(indexSetDomain);
  set = indexSetDomain->set->clone<SetIndexSet>();
}

HIRNode::Ptr IndexSetDomain::cloneImpl() {
  const auto node = std::make_shared<IndexSetDomain>();
  node->copy(shared_from_this());
  return node;
}

void RangeDomain::copy(HIRNode::Ptr node) {
  const auto rangeDomain = to<RangeDomain>(node);
  ForDomain::copy(rangeDomain);
  lower = rangeDomain->lower->clone<Expr>();
  upper = rangeDomain->upper->clone<Expr>();
}

HIRNode::Ptr RangeDomain::cloneImpl() {
  const auto node = std::make_shared<RangeDomain>();
  node->copy(shared_from_this());
  return node;
}

void ForStmt::copy(HIRNode::Ptr node) {
  const auto forStmt = to<ForStmt>(node);
  Stmt::copy(forStmt);
  loopVar = forStmt->loopVar->clone<Identifier>();
  domain = forStmt->domain->clone<ForDomain>();
  body = forStmt->body->clone<StmtBlock>();
}

HIRNode::Ptr ForStmt::cloneImpl() {
  const auto node = std::make_shared<ForStmt>();
  node->copy(shared_from_this());
  return node;
}

void PrintStmt::copy(HIRNode::Ptr node) {
  const auto printStmt = to<PrintStmt>(node);
  Stmt::copy(printStmt);
  for (const auto &arg : printStmt->args) {
    args.push_back(arg->clone<Expr>());
  }
  printNewline = printStmt->printNewline;
}

HIRNode::Ptr PrintStmt::cloneImpl() {
  const auto node = std::make_shared<PrintStmt>();
  node->copy(shared_from_this());
  return node;
}

void ExprStmt::copy(HIRNode::Ptr node) {
  const auto exprStmt = to<ExprStmt>(node);
  Stmt::copy(exprStmt);
  expr = exprStmt->expr->clone<Expr>();
}

HIRNode::Ptr ExprStmt::cloneImpl() {
  const auto node = std::make_shared<ExprStmt>();
  node->copy(shared_from_this());
  return node;
}

void AssignStmt::copy(HIRNode::Ptr node) {
  const auto assignStmt = to<AssignStmt>(node);
  ExprStmt::copy(assignStmt);
  for (const auto &left : assignStmt->lhs) {
    lhs.push_back(left->clone<Expr>());
  }
}

HIRNode::Ptr AssignStmt::cloneImpl() {
  const auto node = std::make_shared<AssignStmt>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr Slice::cloneImpl() {
  const auto node = std::make_shared<Slice>();
  node->copy(shared_from_this());
  return node;
}

void ExprParam::copy(HIRNode::Ptr node) {
  const auto exprParam = to<ExprParam>(node);
  ReadParam::copy(exprParam);
  expr = exprParam->expr->clone<Expr>();
}

HIRNode::Ptr ExprParam::cloneImpl() {
  const auto node = std::make_shared<ExprParam>();
  node->copy(shared_from_this());
  return node;
}

void MapExpr::copy(HIRNode::Ptr node) {
  const auto mapExpr = to<MapExpr>(node);
  Expr::copy(mapExpr);
  func = mapExpr->func->clone<Identifier>();
  for (const auto &arg : mapExpr->partialActuals) {
    partialActuals.push_back(arg->clone<Expr>());
  }
  target = mapExpr->target->clone<SetIndexSet>();
}

void ReducedMapExpr::copy(HIRNode::Ptr node) {
  const auto reducedMapExpr = to<ReducedMapExpr>(node);
  MapExpr::copy(reducedMapExpr);
  op = reducedMapExpr->op;
}

HIRNode::Ptr ReducedMapExpr::cloneImpl() {
  const auto node = std::make_shared<ReducedMapExpr>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr UnreducedMapExpr::cloneImpl() {
  const auto node = std::make_shared<UnreducedMapExpr>();
  node->copy(shared_from_this());
  return node;
}

void UnaryExpr::copy(HIRNode::Ptr node) {
  const auto unaryExpr = to<UnaryExpr>(node);
  Expr::copy(unaryExpr);
  operand = unaryExpr->operand->clone<Expr>();
}

void BinaryExpr::copy(HIRNode::Ptr node) {
  const auto binaryExpr = to<BinaryExpr>(node);
  Expr::copy(binaryExpr);
  lhs = binaryExpr->lhs->clone<Expr>();
  rhs = binaryExpr->rhs->clone<Expr>();
}

void NaryExpr::copy(HIRNode::Ptr node) {
  const auto naryExpr = to<NaryExpr>(node);
  Expr::copy(naryExpr);
  for (const auto &operand : naryExpr->operands) {
    operands.push_back(operand->clone<Expr>());
  }
}

HIRNode::Ptr OrExpr::cloneImpl() {
  const auto node = std::make_shared<OrExpr>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr AndExpr::cloneImpl() {
  const auto node = std::make_shared<AndExpr>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr XorExpr::cloneImpl() {
  const auto node = std::make_shared<XorExpr>();
  node->copy(shared_from_this());
  return node;
}

void EqExpr::copy(HIRNode::Ptr node) {
  const auto eqExpr = to<EqExpr>(node);
  NaryExpr::copy(eqExpr);
  ops = eqExpr->ops;
}

HIRNode::Ptr EqExpr::cloneImpl() {
  const auto node = std::make_shared<EqExpr>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr NotExpr::cloneImpl() {
  const auto node = std::make_shared<NotExpr>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr AddExpr::cloneImpl() {
  const auto node = std::make_shared<AddExpr>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr SubExpr::cloneImpl() {
  const auto node = std::make_shared<SubExpr>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr MulExpr::cloneImpl() {
  const auto node = std::make_shared<MulExpr>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr DivExpr::cloneImpl() {
  const auto node = std::make_shared<DivExpr>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr ElwiseMulExpr::cloneImpl() {
  const auto node = std::make_shared<ElwiseMulExpr>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr ElwiseDivExpr::cloneImpl() {
  const auto node = std::make_shared<ElwiseDivExpr>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr LeftDivExpr::cloneImpl() {
  const auto node = std::make_shared<LeftDivExpr>();
  node->copy(shared_from_this());
  return node;
}

void NegExpr::copy(HIRNode::Ptr node) {
  const auto negExpr = to<NegExpr>(node);
  UnaryExpr::copy(negExpr);
  negate = negExpr->negate;
}

HIRNode::Ptr NegExpr::cloneImpl() {
  const auto node = std::make_shared<NegExpr>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr ExpExpr::cloneImpl() {
  const auto node = std::make_shared<ExpExpr>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr TransposeExpr::cloneImpl() {
  const auto node = std::make_shared<TransposeExpr>();
  node->copy(shared_from_this());
  return node;
}

void CallExpr::copy(HIRNode::Ptr node) {
  const auto callExpr = to<CallExpr>(node);
  Expr::copy(callExpr);
  func = callExpr->func->clone<Identifier>();
  for (const auto &genericArg : callExpr->genericArgs) {
    genericArgs.push_back(genericArg->clone<Expr>());
  }
  for (const auto &arg : callExpr->args) {
    args.push_back(arg ? arg->clone<Expr>() : Expr::Ptr());
  }
}

HIRNode::Ptr CallExpr::cloneImpl() {
  const auto node = std::make_shared<CallExpr>();
  node->copy(shared_from_this());
  return node;
}

void TensorReadExpr::copy(HIRNode::Ptr node) {
  const auto tensorReadExpr = to<TensorReadExpr>(node);
  Expr::copy(tensorReadExpr);
  tensor = tensorReadExpr->tensor->clone<Expr>();
  for (const auto &index : tensorReadExpr->indices) {
    indices.push_back(index->clone<ReadParam>());
  }
}

HIRNode::Ptr TensorReadExpr::cloneImpl() {
  const auto node = std::make_shared<TensorReadExpr>();
  node->copy(shared_from_this());
  return node;
}

void TupleReadExpr::copy(HIRNode::Ptr node) {
  const auto tupleReadExpr = to<TupleReadExpr>(node);
  Expr::copy(tupleReadExpr);
  tuple = tupleReadExpr->tuple->clone<Expr>();
  if (tupleReadExpr->index) {
    index = tupleReadExpr->index->clone<Expr>();
  }
}

HIRNode::Ptr TupleReadExpr::cloneImpl() {
  const auto node = std::make_shared<TupleReadExpr>();
  node->copy(shared_from_this());
  return node;
}

void FieldReadExpr::copy(HIRNode::Ptr node) {
  const auto fieldReadExpr = to<FieldReadExpr>(node);
  Expr::copy(fieldReadExpr);
  setOrElem = fieldReadExpr->setOrElem->clone<Expr>();
  field = fieldReadExpr->field->clone<Identifier>();
}

HIRNode::Ptr FieldReadExpr::cloneImpl() {
  const auto node = std::make_shared<FieldReadExpr>();
  node->copy(shared_from_this());
  return node;
}

void ParenExpr::copy(HIRNode::Ptr node) {
  const auto parenExpr = to<ParenExpr>(node);
  Expr::copy(parenExpr);
  expr = parenExpr->expr->clone<Expr>();
}

HIRNode::Ptr ParenExpr::cloneImpl() {
  const auto node = std::make_shared<ParenExpr>();
  node->copy(shared_from_this());
  return node;
}

void VarExpr::copy(HIRNode::Ptr node) {
  const auto varExpr = to<VarExpr>(node);
  Expr::copy(varExpr);
  ident = varExpr->ident;
}

HIRNode::Ptr VarExpr::cloneImpl() {
  const auto node = std::make_shared<VarExpr>();
  node->copy(shared_from_this());
  return node;
}

HIRNode::Ptr RangeConst::cloneImpl() {
  const auto node = std::make_shared<RangeConst>();
  node->copy(shared_from_this());
  return node;
}

void IntLiteral::copy(HIRNode::Ptr node) {
  const auto intLiteral = to<IntLiteral>(node);
  TensorLiteral::copy(intLiteral);
  val = intLiteral->val;
}

HIRNode::Ptr IntLiteral::cloneImpl() {
  const auto node = std::make_shared<IntLiteral>();
  node->copy(shared_from_this());
  return node;
}

void FloatLiteral::copy(HIRNode::Ptr node) {
  const auto floatLiteral = to<FloatLiteral>(node);
  TensorLiteral::copy(floatLiteral);
  val = floatLiteral->val;
}

HIRNode::Ptr FloatLiteral::cloneImpl() {
  const auto node = std::make_shared<FloatLiteral>();
  node->copy(shared_from_this());
  return node;
}

void BoolLiteral::copy(HIRNode::Ptr node) {
  const auto boolLiteral = to<BoolLiteral>(node);
  TensorLiteral::copy(boolLiteral);
  val = boolLiteral->val;
}

HIRNode::Ptr BoolLiteral::cloneImpl() {
  const auto node = std::make_shared<BoolLiteral>();
  node->copy(shared_from_this());
  return node;
}

void ComplexLiteral::copy(HIRNode::Ptr node) {
  const auto complexLiteral = to<ComplexLiteral>(node);
  TensorLiteral::copy(complexLiteral);
  val = complexLiteral->val;
}

HIRNode::Ptr ComplexLiteral::cloneImpl() {
  const auto node = std::make_shared<ComplexLiteral>();
  node->copy(shared_from_this());
  return node;
}

void StringLiteral::copy(HIRNode::Ptr node) {
  const auto stringLiteral = to<StringLiteral>(node);
  TensorLiteral::copy(stringLiteral);
  val = stringLiteral->val;
}

HIRNode::Ptr StringLiteral::cloneImpl() {
  const auto node = std::make_shared<StringLiteral>();
  node->copy(shared_from_this());
  return node;
}

void DenseTensorLiteral::copy(HIRNode::Ptr node) {
  const auto denseTensorLiteral = to<DenseTensorLiteral>(node);
  TensorLiteral::copy(denseTensorLiteral);
  transposed = denseTensorLiteral->transposed;
}

void IntVectorLiteral::copy(HIRNode::Ptr node) {
  const auto intVectorLiteral = to<IntVectorLiteral>(node);
  DenseTensorLiteral::copy(intVectorLiteral);
  vals = intVectorLiteral->vals;
}

HIRNode::Ptr IntVectorLiteral::cloneImpl() {
  const auto node = std::make_shared<IntVectorLiteral>();
  node->copy(shared_from_this());
  return node;
}

void FloatVectorLiteral::copy(HIRNode::Ptr node) {
  const auto floatVectorLiteral = to<FloatVectorLiteral>(node);
  DenseTensorLiteral::copy(floatVectorLiteral);
  vals = floatVectorLiteral->vals;
}

HIRNode::Ptr FloatVectorLiteral::cloneImpl() {
  const auto node = std::make_shared<FloatVectorLiteral>();
  node->copy(shared_from_this());
  return node;
}

void ComplexVectorLiteral::copy(HIRNode::Ptr node) {
  const auto complexVectorLiteral = to<ComplexVectorLiteral>(node);
  DenseTensorLiteral::copy(complexVectorLiteral);
  vals = complexVectorLiteral->vals;
}

HIRNode::Ptr ComplexVectorLiteral::cloneImpl() {
  const auto node = std::make_shared<ComplexVectorLiteral>();
  node->copy(shared_from_this());
  return node;
}

void NDTensorLiteral::copy(HIRNode::Ptr node) {
  const auto ndTensorLiteral = to<NDTensorLiteral>(node);
  DenseTensorLiteral::copy(ndTensorLiteral);
  for (const auto &elem : ndTensorLiteral->elems) {
    elems.push_back(elem->clone<DenseTensorLiteral>());
  }
}

HIRNode::Ptr NDTensorLiteral::cloneImpl() {
  const auto node = std::make_shared<NDTensorLiteral>();
  node->copy(shared_from_this());
  return node;
}

void ApplyStmt::copy(HIRNode::Ptr node) {
  const auto applyStmt = to<ApplyStmt>(node);
  Stmt::copy(applyStmt);
  map = applyStmt->map->clone<UnreducedMapExpr>();
}

HIRNode::Ptr ApplyStmt::cloneImpl() {
  const auto node = std::make_shared<ApplyStmt>();
  node->copy(shared_from_this());
  return node;
}

void Test::copy(HIRNode::Ptr node) {
  const auto test = to<Test>(node);
  HIRNode::copy(test);
  func = test->func->clone<Identifier>();
  for (const auto &arg : test->args) {
    args.push_back(arg->clone<Expr>());
  }
  expected = test->expected->clone<Expr>();
}

HIRNode::Ptr Test::cloneImpl() {
  const auto node = std::make_shared<Test>();
  node->copy(shared_from_this());
  return node;
}

}
}

