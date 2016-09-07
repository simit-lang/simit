#include <memory>

#include "fir.h"
#include "token.h"
#include "program_context.h"

namespace simit {
namespace fir {

void FIRNode::copy(FIRNode::Ptr node) {
  lineBegin = node->lineBegin;
  colBegin = node->colBegin;
  lineEnd = node->lineEnd;
  colEnd = node->colEnd;
}

void FIRNode::setBeginLoc(const internal::Token &token) {
  lineBegin = token.lineBegin;
  colBegin = token.colBegin;
}

void FIRNode::setEndLoc(const internal::Token &token) {
  lineEnd = token.lineEnd;
  colEnd = token.colEnd;
}

void FIRNode::setLoc(const internal::Token &token) {
  setBeginLoc(token);
  setEndLoc(token);
}

void Program::copy(FIRNode::Ptr node) {
  const auto program = to<Program>(node);
  FIRNode::copy(program);
  for (const auto &elem : program->elems) {
    elems.push_back(elem->clone());
  }
}

FIRNode::Ptr Program::cloneNode() {
  const auto node = std::make_shared<Program>();
  node->copy(shared_from_this());
  return node;
}

void StmtBlock::copy(FIRNode::Ptr node) {
  const auto stmtBlock = to<StmtBlock>(node);
  Stmt::copy(stmtBlock);
  for (const auto &stmt : stmtBlock->stmts) {
    stmts.push_back(stmt->clone<Stmt>());
  }
}

FIRNode::Ptr StmtBlock::cloneNode() {
  const auto node = std::make_shared<StmtBlock>();
  node->copy(shared_from_this());
  return node;
}

void RangeIndexSet::copy(FIRNode::Ptr node) {
  const auto indexSet = to<RangeIndexSet>(node);
  IndexSet::copy(indexSet);
  range = indexSet->range;
}

FIRNode::Ptr RangeIndexSet::cloneNode() {
  const auto node = std::make_shared<RangeIndexSet>();
  node->copy(shared_from_this());
  return node;
}

void SetIndexSet::copy(FIRNode::Ptr node) {
  const auto indexSet = to<SetIndexSet>(node);
  IndexSet::copy(indexSet);
  setName = indexSet->setName;
}

FIRNode::Ptr SetIndexSet::cloneNode() {
  const auto node = std::make_shared<SetIndexSet>();
  node->copy(shared_from_this());
  return node;
}

void GenericIndexSet::copy(FIRNode::Ptr node) {
  const auto indexSet = to<GenericIndexSet>(node);
  SetIndexSet::copy(indexSet);
  type = indexSet->type;
}

FIRNode::Ptr GenericIndexSet::cloneNode() {
  const auto node = std::make_shared<GenericIndexSet>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr DynamicIndexSet::cloneNode() {
  const auto node = std::make_shared<DynamicIndexSet>();
  node->copy(shared_from_this());
  return node;
}

void ElementType::copy(FIRNode::Ptr node) {
  const auto elementType = to<ElementType>(node);
  Type::copy(elementType);
  ident = elementType->ident;
}

FIRNode::Ptr ElementType::cloneNode() {
  const auto node = std::make_shared<ElementType>();
  node->copy(shared_from_this());
  return node;
}

void Endpoint::copy(FIRNode::Ptr node) {
  const auto endpoint = to<Endpoint>(node);
  FIRNode::copy(endpoint);
  set = endpoint->set->clone<SetIndexSet>();
  element = endpoint->element->clone<ElementType>();
}

FIRNode::Ptr Endpoint::cloneNode() {
  const auto node = std::make_shared<Endpoint>();
  node->copy(shared_from_this());
  return node;
}

SetType::Ptr SetType::getUndefinedSetType() {
  const auto undefinedSetType = std::make_shared<HeterogeneousEdgeSetType>();
  undefinedSetType->element = std::make_shared<ElementType>();
  return undefinedSetType;
}

void SetType::copy(FIRNode::Ptr node) {
  const auto setType = to<SetType>(node);
  Type::copy(setType);
  element = setType->element->clone<ElementType>();
}

FIRNode::Ptr UnstructuredSetType::cloneNode() {
  const auto node = std::make_shared<UnstructuredSetType>();
  node->copy(shared_from_this());
  return node;
}

void TupleLength::copy(FIRNode::Ptr node) {
  const auto length = to<TupleLength>(node);
  FIRNode::copy(length);
  val = length->val;
}

FIRNode::Ptr TupleLength::cloneNode() {
  const auto node = std::make_shared<TupleLength>();
  node->copy(shared_from_this());
  return node;
}

void HomogeneousEdgeSetType::copy(FIRNode::Ptr node) {
  const auto setType = to<HomogeneousEdgeSetType>(node);
  UnstructuredSetType::copy(setType);
  endpoint = setType->endpoint->clone<Endpoint>();
  arity = setType->arity->clone<TupleLength>();
}

FIRNode::Ptr HomogeneousEdgeSetType::cloneNode() {
  const auto node = std::make_shared<HomogeneousEdgeSetType>();
  node->copy(shared_from_this());
  return node;
}

bool HeterogeneousEdgeSetType::isHomogeneous() const {
  const auto neighborSet = endpoints[0]->set->setName;
  
  for (unsigned i = 1; i < endpoints.size(); ++i) {
    if (endpoints[i]->set->setName != neighborSet) {
      return false;
    }
  }

  return true;
}

void HeterogeneousEdgeSetType::copy(FIRNode::Ptr node) {
  const auto setType = to<HeterogeneousEdgeSetType>(node);
  UnstructuredSetType::copy(setType);
  for (const auto &endpoint : setType->endpoints) {
    endpoints.push_back(endpoint->clone<Endpoint>());
  }
}

FIRNode::Ptr HeterogeneousEdgeSetType::cloneNode() {
  const auto node = std::make_shared<HeterogeneousEdgeSetType>();
  node->copy(shared_from_this());
  return node;
}

void LatticeLinkSetType::copy(FIRNode::Ptr node) {
  const auto setType = to<LatticeLinkSetType>(node);
  SetType::copy(setType);
  latticePointSet = setType->latticePointSet->clone<Endpoint>();
  dimensions = setType->dimensions;
}

FIRNode::Ptr LatticeLinkSetType::cloneNode() {
  const auto node = std::make_shared<LatticeLinkSetType>();
  node->copy(shared_from_this());
  return node;
}

void Identifier::copy(FIRNode::Ptr node) {
  const auto identifier = to<Identifier>(node);
  FIRNode::copy(identifier);
  ident = identifier->ident;
}

FIRNode::Ptr Identifier::cloneNode() {
  const auto node = std::make_shared<Identifier>();
  node->copy(shared_from_this());
  return node;
}

void TupleElement::copy(FIRNode::Ptr node) {
  const auto elem = to<TupleElement>(node);
  FIRNode::copy(elem);
  if (elem->name) {
    name = elem->name->clone<Identifier>();
  }
  element = elem->element->clone<ElementType>();
}

FIRNode::Ptr TupleElement::cloneNode() {
  const auto node = std::make_shared<TupleElement>();
  node->copy(shared_from_this());
  return node;
}

void NamedTupleType::copy(FIRNode::Ptr node) {
  const auto tupleType = to<NamedTupleType>(node);
  Type::copy(tupleType);
  for (const auto &elem : tupleType->elems) {
    elems.push_back(elem->clone<TupleElement>());
  }
}

FIRNode::Ptr NamedTupleType::cloneNode() {
  const auto node = std::make_shared<NamedTupleType>();
  node->copy(shared_from_this());
  return node;
}

void UnnamedTupleType::copy(FIRNode::Ptr node) {
  const auto tupleType = to<UnnamedTupleType>(node);
  Type::copy(tupleType);
  element = tupleType->element->clone<ElementType>();
  length = tupleType->length->clone<TupleLength>();
}

FIRNode::Ptr UnnamedTupleType::cloneNode() {
  const auto node = std::make_shared<UnnamedTupleType>();
  node->copy(shared_from_this());
  return node;
}

void ScalarType::copy(FIRNode::Ptr node) {
  const auto scalarType = to<ScalarType>(node);
  TensorType::copy(scalarType);
  type = scalarType->type;
}

FIRNode::Ptr ScalarType::cloneNode() {
  const auto node = std::make_shared<ScalarType>();
  node->copy(shared_from_this());
  return node;
}

void NDTensorType::copy(FIRNode::Ptr node) {
  const auto ndTensorType = to<NDTensorType>(node);
  TensorType::copy(ndTensorType);
  for (const auto &indexSet : ndTensorType->indexSets) {
    indexSets.push_back(indexSet->clone<IndexSet>());
  }
  blockType = ndTensorType->blockType->clone<TensorType>();
  transposed = ndTensorType->transposed;
}

FIRNode::Ptr NDTensorType::cloneNode() {
  const auto node = std::make_shared<NDTensorType>();
  node->copy(shared_from_this());
  return node;
}

void OpaqueType::copy(FIRNode::Ptr node) {
  const auto opaqueType = to<OpaqueType>(node);
  Type::copy(opaqueType);
}

FIRNode::Ptr OpaqueType::cloneNode() {
  const auto node = std::make_shared<OpaqueType>();
  node->copy(shared_from_this());
  return node;
}

void IdentDecl::copy(FIRNode::Ptr node) {
  const auto identDecl = to<IdentDecl>(node);
  FIRNode::copy(identDecl);
  name = identDecl->name->clone<Identifier>();
  type = identDecl->type->clone<Type>();
}

FIRNode::Ptr IdentDecl::cloneNode() {
  const auto node = std::make_shared<IdentDecl>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr FieldDecl::cloneNode() {
  const auto node = std::make_shared<FieldDecl>();
  node->copy(shared_from_this());
  return node;
}

void ElementTypeDecl::copy(FIRNode::Ptr node) {
  const auto elementTypeDecl = to<ElementTypeDecl>(node);
  FIRNode::copy(elementTypeDecl);
  name = elementTypeDecl->name->clone<Identifier>();
  for (const auto &field : elementTypeDecl->fields) {
    fields.push_back(field->clone<FieldDecl>());
  }
}

FIRNode::Ptr ElementTypeDecl::cloneNode() {
  const auto node = std::make_shared<ElementTypeDecl>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr Argument::cloneNode() {
  const auto node = std::make_shared<Argument>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr InOutArgument::cloneNode() {
  const auto node = std::make_shared<InOutArgument>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr ExternDecl::cloneNode() {
  const auto node = std::make_shared<ExternDecl>();
  node->copy(shared_from_this());
  return node;
}

void GenericParam::copy(FIRNode::Ptr node) {
  const auto genericParam = to<GenericParam>(node);
  FIRNode::copy(genericParam);
  name = genericParam->name;
  type = genericParam->type;
}

FIRNode::Ptr GenericParam::cloneNode() {
  const auto node = std::make_shared<GenericParam>();
  node->copy(shared_from_this());
  return node;
}

void FuncDecl::copy(FIRNode::Ptr node) {
  const auto funcDecl = to<FuncDecl>(node);
  FIRNode::copy(funcDecl);
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
  if (funcDecl->body) {
    body = funcDecl->body->clone<StmtBlock>();
  }
  type = funcDecl->type;
  originalName = funcDecl->originalName;
}

FIRNode::Ptr FuncDecl::cloneNode() {
  const auto node = std::make_shared<FuncDecl>();
  node->copy(shared_from_this());
  return node;
}

void VarDecl::copy(FIRNode::Ptr node) {
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

FIRNode::Ptr VarDecl::cloneNode() {
  const auto node = std::make_shared<VarDecl>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr ConstDecl::cloneNode() {
  const auto node = std::make_shared<ConstDecl>();
  node->copy(shared_from_this());
  return node;
}

void WhileStmt::copy(FIRNode::Ptr node) {
  const auto whileStmt = to<WhileStmt>(node);
  Stmt::copy(whileStmt);
  cond = whileStmt->cond->clone<Expr>(); 
  body = whileStmt->body->clone<StmtBlock>();
}

FIRNode::Ptr WhileStmt::cloneNode() {
  const auto node = std::make_shared<WhileStmt>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr DoWhileStmt::cloneNode() {
  const auto node = std::make_shared<DoWhileStmt>();
  node->copy(shared_from_this());
  return node;
}

void IfStmt::copy(FIRNode::Ptr node) {
  const auto ifStmt = to<IfStmt>(node);
  Stmt::copy(ifStmt);
  cond = ifStmt->cond->clone<Expr>(); 
  ifBody = ifStmt->ifBody->clone<Stmt>();
  if (ifStmt->elseBody) {
    elseBody = ifStmt->elseBody->clone<Stmt>();
  }
}

FIRNode::Ptr IfStmt::cloneNode() {
  const auto node = std::make_shared<IfStmt>();
  node->copy(shared_from_this());
  return node;
}

void IndexSetDomain::copy(FIRNode::Ptr node) {
  const auto indexSetDomain = to<IndexSetDomain>(node);
  ForDomain::copy(indexSetDomain);
  set = indexSetDomain->set->clone<SetIndexSet>();
}

FIRNode::Ptr IndexSetDomain::cloneNode() {
  const auto node = std::make_shared<IndexSetDomain>();
  node->copy(shared_from_this());
  return node;
}

void RangeDomain::copy(FIRNode::Ptr node) {
  const auto rangeDomain = to<RangeDomain>(node);
  ForDomain::copy(rangeDomain);
  lower = rangeDomain->lower->clone<Expr>();
  upper = rangeDomain->upper->clone<Expr>();
}

FIRNode::Ptr RangeDomain::cloneNode() {
  const auto node = std::make_shared<RangeDomain>();
  node->copy(shared_from_this());
  return node;
}

void ForStmt::copy(FIRNode::Ptr node) {
  const auto forStmt = to<ForStmt>(node);
  Stmt::copy(forStmt);
  loopVar = forStmt->loopVar->clone<Identifier>();
  domain = forStmt->domain->clone<ForDomain>();
  body = forStmt->body->clone<StmtBlock>();
}

FIRNode::Ptr ForStmt::cloneNode() {
  const auto node = std::make_shared<ForStmt>();
  node->copy(shared_from_this());
  return node;
}

void PrintStmt::copy(FIRNode::Ptr node) {
  const auto printStmt = to<PrintStmt>(node);
  Stmt::copy(printStmt);
  for (const auto &arg : printStmt->args) {
    args.push_back(arg->clone<Expr>());
  }
  printNewline = printStmt->printNewline;
}

FIRNode::Ptr PrintStmt::cloneNode() {
  const auto node = std::make_shared<PrintStmt>();
  node->copy(shared_from_this());
  return node;
}

void ExprStmt::copy(FIRNode::Ptr node) {
  const auto exprStmt = to<ExprStmt>(node);
  Stmt::copy(exprStmt);
  expr = exprStmt->expr->clone<Expr>();
}

FIRNode::Ptr ExprStmt::cloneNode() {
  const auto node = std::make_shared<ExprStmt>();
  node->copy(shared_from_this());
  return node;
}

void AssignStmt::copy(FIRNode::Ptr node) {
  const auto assignStmt = to<AssignStmt>(node);
  ExprStmt::copy(assignStmt);
  for (const auto &left : assignStmt->lhs) {
    lhs.push_back(left->clone<Expr>());
  }
}

FIRNode::Ptr AssignStmt::cloneNode() {
  const auto node = std::make_shared<AssignStmt>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr Slice::cloneNode() {
  const auto node = std::make_shared<Slice>();
  node->copy(shared_from_this());
  return node;
}

void ExprParam::copy(FIRNode::Ptr node) {
  const auto exprParam = to<ExprParam>(node);
  ReadParam::copy(exprParam);
  expr = exprParam->expr->clone<Expr>();
}

FIRNode::Ptr ExprParam::cloneNode() {
  const auto node = std::make_shared<ExprParam>();
  node->copy(shared_from_this());
  return node;
}

void MapExpr::copy(FIRNode::Ptr node) {
  const auto mapExpr = to<MapExpr>(node);
  Expr::copy(mapExpr);
  func = mapExpr->func->clone<Identifier>();
  for (const auto &arg : mapExpr->partialActuals) {
    partialActuals.push_back(arg->clone<Expr>());
  }
  target = mapExpr->target->clone<SetIndexSet>();
}

void ReducedMapExpr::copy(FIRNode::Ptr node) {
  const auto reducedMapExpr = to<ReducedMapExpr>(node);
  MapExpr::copy(reducedMapExpr);
  op = reducedMapExpr->op;
}

FIRNode::Ptr ReducedMapExpr::cloneNode() {
  const auto node = std::make_shared<ReducedMapExpr>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr UnreducedMapExpr::cloneNode() {
  const auto node = std::make_shared<UnreducedMapExpr>();
  node->copy(shared_from_this());
  return node;
}

void UnaryExpr::copy(FIRNode::Ptr node) {
  const auto unaryExpr = to<UnaryExpr>(node);
  Expr::copy(unaryExpr);
  operand = unaryExpr->operand->clone<Expr>();
}

void BinaryExpr::copy(FIRNode::Ptr node) {
  const auto binaryExpr = to<BinaryExpr>(node);
  Expr::copy(binaryExpr);
  lhs = binaryExpr->lhs->clone<Expr>();
  rhs = binaryExpr->rhs->clone<Expr>();
}

void NaryExpr::copy(FIRNode::Ptr node) {
  const auto naryExpr = to<NaryExpr>(node);
  Expr::copy(naryExpr);
  for (const auto &operand : naryExpr->operands) {
    operands.push_back(operand->clone<Expr>());
  }
}

FIRNode::Ptr OrExpr::cloneNode() {
  const auto node = std::make_shared<OrExpr>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr AndExpr::cloneNode() {
  const auto node = std::make_shared<AndExpr>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr XorExpr::cloneNode() {
  const auto node = std::make_shared<XorExpr>();
  node->copy(shared_from_this());
  return node;
}

void EqExpr::copy(FIRNode::Ptr node) {
  const auto eqExpr = to<EqExpr>(node);
  NaryExpr::copy(eqExpr);
  ops = eqExpr->ops;
}

FIRNode::Ptr EqExpr::cloneNode() {
  const auto node = std::make_shared<EqExpr>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr NotExpr::cloneNode() {
  const auto node = std::make_shared<NotExpr>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr AddExpr::cloneNode() {
  const auto node = std::make_shared<AddExpr>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr SubExpr::cloneNode() {
  const auto node = std::make_shared<SubExpr>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr MulExpr::cloneNode() {
  const auto node = std::make_shared<MulExpr>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr DivExpr::cloneNode() {
  const auto node = std::make_shared<DivExpr>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr ElwiseMulExpr::cloneNode() {
  const auto node = std::make_shared<ElwiseMulExpr>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr ElwiseDivExpr::cloneNode() {
  const auto node = std::make_shared<ElwiseDivExpr>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr LeftDivExpr::cloneNode() {
  const auto node = std::make_shared<LeftDivExpr>();
  node->copy(shared_from_this());
  return node;
}

void NegExpr::copy(FIRNode::Ptr node) {
  const auto negExpr = to<NegExpr>(node);
  UnaryExpr::copy(negExpr);
  negate = negExpr->negate;
}

FIRNode::Ptr NegExpr::cloneNode() {
  const auto node = std::make_shared<NegExpr>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr ExpExpr::cloneNode() {
  const auto node = std::make_shared<ExpExpr>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr TransposeExpr::cloneNode() {
  const auto node = std::make_shared<TransposeExpr>();
  node->copy(shared_from_this());
  return node;
}

void CallExpr::copy(FIRNode::Ptr node) {
  const auto callExpr = to<CallExpr>(node);
  Expr::copy(callExpr);
  func = callExpr->func->clone<Identifier>();
  for (const auto &genericArg : callExpr->genericArgs) {
    genericArgs.push_back(genericArg->clone<IndexSet>());
  }
  for (const auto &arg : callExpr->args) {
    args.push_back(arg ? arg->clone<Expr>() : Expr::Ptr());
  }
}

FIRNode::Ptr CallExpr::cloneNode() {
  const auto node = std::make_shared<CallExpr>();
  node->copy(shared_from_this());
  return node;
}

void TensorReadExpr::copy(FIRNode::Ptr node) {
  const auto tensorReadExpr = to<TensorReadExpr>(node);
  Expr::copy(tensorReadExpr);
  tensor = tensorReadExpr->tensor->clone<Expr>();
  for (const auto &index : tensorReadExpr->indices) {
    indices.push_back(index->clone<ReadParam>());
  }
}

FIRNode::Ptr TensorReadExpr::cloneNode() {
  const auto node = std::make_shared<TensorReadExpr>();
  node->copy(shared_from_this());
  return node;
}

void SetReadExpr::copy(FIRNode::Ptr node) {
  const auto setReadExpr = to<SetReadExpr>(node);
  Expr::copy(setReadExpr);
  set = setReadExpr->set->clone<Expr>();
  for (const auto &index : setReadExpr->indices) {
    indices.push_back(index->clone<Expr>());
  }
}

FIRNode::Ptr SetReadExpr::cloneNode() {
  const auto node = std::make_shared<SetReadExpr>();
  node->copy(shared_from_this());
  return node;
}

void TupleReadExpr::copy(FIRNode::Ptr node) {
  const auto tupleReadExpr = to<TupleReadExpr>(node);
  Expr::copy(tupleReadExpr);
  tuple = tupleReadExpr->tuple->clone<Expr>();
}

void NamedTupleReadExpr::copy(FIRNode::Ptr node) {
  const auto tupleReadExpr = to<NamedTupleReadExpr>(node);
  TupleReadExpr::copy(tupleReadExpr);
  elem = tupleReadExpr->elem->clone<Identifier>();
}

FIRNode::Ptr NamedTupleReadExpr::cloneNode() {
  const auto node = std::make_shared<NamedTupleReadExpr>();
  node->copy(shared_from_this());
  return node;
}

void UnnamedTupleReadExpr::copy(FIRNode::Ptr node) {
  const auto tupleReadExpr = to<UnnamedTupleReadExpr>(node);
  TupleReadExpr::copy(tupleReadExpr);
  index = tupleReadExpr->index->clone<Expr>();
}

FIRNode::Ptr UnnamedTupleReadExpr::cloneNode() {
  const auto node = std::make_shared<UnnamedTupleReadExpr>();
  node->copy(shared_from_this());
  return node;
}

void FieldReadExpr::copy(FIRNode::Ptr node) {
  const auto fieldReadExpr = to<FieldReadExpr>(node);
  Expr::copy(fieldReadExpr);
  setOrElem = fieldReadExpr->setOrElem->clone<Expr>();
  field = fieldReadExpr->field->clone<Identifier>();
}

FIRNode::Ptr FieldReadExpr::cloneNode() {
  const auto node = std::make_shared<FieldReadExpr>();
  node->copy(shared_from_this());
  return node;
}

void ParenExpr::copy(FIRNode::Ptr node) {
  const auto parenExpr = to<ParenExpr>(node);
  Expr::copy(parenExpr);
  expr = parenExpr->expr->clone<Expr>();
}

FIRNode::Ptr ParenExpr::cloneNode() {
  const auto node = std::make_shared<ParenExpr>();
  node->copy(shared_from_this());
  return node;
}

void VarExpr::copy(FIRNode::Ptr node) {
  const auto varExpr = to<VarExpr>(node);
  Expr::copy(varExpr);
  ident = varExpr->ident;
}

FIRNode::Ptr VarExpr::cloneNode() {
  const auto node = std::make_shared<VarExpr>();
  node->copy(shared_from_this());
  return node;
}

FIRNode::Ptr RangeConst::cloneNode() {
  const auto node = std::make_shared<RangeConst>();
  node->copy(shared_from_this());
  return node;
}

void IntLiteral::copy(FIRNode::Ptr node) {
  const auto intLiteral = to<IntLiteral>(node);
  TensorLiteral::copy(intLiteral);
  val = intLiteral->val;
}

FIRNode::Ptr IntLiteral::cloneNode() {
  const auto node = std::make_shared<IntLiteral>();
  node->copy(shared_from_this());
  return node;
}

void FloatLiteral::copy(FIRNode::Ptr node) {
  const auto floatLiteral = to<FloatLiteral>(node);
  TensorLiteral::copy(floatLiteral);
  val = floatLiteral->val;
}

FIRNode::Ptr FloatLiteral::cloneNode() {
  const auto node = std::make_shared<FloatLiteral>();
  node->copy(shared_from_this());
  return node;
}

void BoolLiteral::copy(FIRNode::Ptr node) {
  const auto boolLiteral = to<BoolLiteral>(node);
  TensorLiteral::copy(boolLiteral);
  val = boolLiteral->val;
}

FIRNode::Ptr BoolLiteral::cloneNode() {
  const auto node = std::make_shared<BoolLiteral>();
  node->copy(shared_from_this());
  return node;
}

void ComplexLiteral::copy(FIRNode::Ptr node) {
  const auto complexLiteral = to<ComplexLiteral>(node);
  TensorLiteral::copy(complexLiteral);
  val = complexLiteral->val;
}

FIRNode::Ptr ComplexLiteral::cloneNode() {
  const auto node = std::make_shared<ComplexLiteral>();
  node->copy(shared_from_this());
  return node;
}

void StringLiteral::copy(FIRNode::Ptr node) {
  const auto stringLiteral = to<StringLiteral>(node);
  TensorLiteral::copy(stringLiteral);
  val = stringLiteral->val;
}

FIRNode::Ptr StringLiteral::cloneNode() {
  const auto node = std::make_shared<StringLiteral>();
  node->copy(shared_from_this());
  return node;
}

void DenseTensorLiteral::copy(FIRNode::Ptr node) {
  const auto denseTensorLiteral = to<DenseTensorLiteral>(node);
  TensorLiteral::copy(denseTensorLiteral);
  transposed = denseTensorLiteral->transposed;
}

void IntVectorLiteral::copy(FIRNode::Ptr node) {
  const auto intVectorLiteral = to<IntVectorLiteral>(node);
  DenseTensorLiteral::copy(intVectorLiteral);
  vals = intVectorLiteral->vals;
}

FIRNode::Ptr IntVectorLiteral::cloneNode() {
  const auto node = std::make_shared<IntVectorLiteral>();
  node->copy(shared_from_this());
  return node;
}

void FloatVectorLiteral::copy(FIRNode::Ptr node) {
  const auto floatVectorLiteral = to<FloatVectorLiteral>(node);
  DenseTensorLiteral::copy(floatVectorLiteral);
  vals = floatVectorLiteral->vals;
}

FIRNode::Ptr FloatVectorLiteral::cloneNode() {
  const auto node = std::make_shared<FloatVectorLiteral>();
  node->copy(shared_from_this());
  return node;
}

void ComplexVectorLiteral::copy(FIRNode::Ptr node) {
  const auto complexVectorLiteral = to<ComplexVectorLiteral>(node);
  DenseTensorLiteral::copy(complexVectorLiteral);
  vals = complexVectorLiteral->vals;
}

FIRNode::Ptr ComplexVectorLiteral::cloneNode() {
  const auto node = std::make_shared<ComplexVectorLiteral>();
  node->copy(shared_from_this());
  return node;
}

void NDTensorLiteral::copy(FIRNode::Ptr node) {
  const auto ndTensorLiteral = to<NDTensorLiteral>(node);
  DenseTensorLiteral::copy(ndTensorLiteral);
  for (const auto &elem : ndTensorLiteral->elems) {
    elems.push_back(elem->clone<DenseTensorLiteral>());
  }
}

FIRNode::Ptr NDTensorLiteral::cloneNode() {
  const auto node = std::make_shared<NDTensorLiteral>();
  node->copy(shared_from_this());
  return node;
}

void ApplyStmt::copy(FIRNode::Ptr node) {
  const auto applyStmt = to<ApplyStmt>(node);
  Stmt::copy(applyStmt);
  map = applyStmt->map->clone<UnreducedMapExpr>();
}

FIRNode::Ptr ApplyStmt::cloneNode() {
  const auto node = std::make_shared<ApplyStmt>();
  node->copy(shared_from_this());
  return node;
}

void Test::copy(FIRNode::Ptr node) {
  const auto test = to<Test>(node);
  FIRNode::copy(test);
  func = test->func->clone<Identifier>();
  for (const auto &arg : test->args) {
    args.push_back(arg->clone<Expr>());
  }
  expected = test->expected->clone<Expr>();
}

FIRNode::Ptr Test::cloneNode() {
  const auto node = std::make_shared<Test>();
  node->copy(shared_from_this());
  return node;
}

TensorType::Ptr makeTensorType(ScalarType::Type componentType,
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

}}
