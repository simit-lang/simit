#include "irutils.h"

#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

using namespace std;

namespace simit {
namespace internal {

// class IndexVarFactory
std::shared_ptr<IndexVar>
IndexVarFactory::makeFreeVar(const IndexSetProduct &indexSet) {
  auto freeIndexVar = new IndexVar(makeName(), indexSet, IndexVar::FREE);
  return std::shared_ptr<IndexVar>(freeIndexVar);
}

std::shared_ptr<IndexVar>
IndexVarFactory::makeReductionVar(const IndexSetProduct &indexSet,
                                  IndexVar::Operator op) {
  auto reductionIndexVar = new IndexVar(makeName(), indexSet, op);
  return std::shared_ptr<IndexVar>(reductionIndexVar);
}

std::string IndexVarFactory::makeName() {
  char name[2];
  name[0] = 'i' + nameID;
  name[1] = '\0';
  nameID++;
  return std::string(name);
}


// Free functions
IndexExpr *unaryElwiseExpr(IndexExpr::Operator op,
                           const std::shared_ptr<TensorNode> &expr) {
  assert(IndexExpr::numOperands(op) == 1);
  std::vector<std::shared_ptr<TensorNode>> operands;
  operands.push_back(expr);
  return elwiseExpr(op, operands);
}

IndexExpr *binaryElwiseExpr(const std::shared_ptr<TensorNode> &l,
                            IndexExpr::Operator op,
                            const std::shared_ptr<TensorNode> &r) {
  assert(IndexExpr::numOperands(op) == 2);

  if (l->getType()->getOrder() == 0 || r->getType()->getOrder() == 0) {
    auto scalar = (l->getType()->getOrder() == 0) ? l : r;
    auto tensor = (l->getType()->getOrder() == 0) ? r : l;
    assert(scalar->getType()->getOrder() == 0);

    std::vector<IndexExpr::IndexVarPtr> scalarIndexVars;

    IndexVarFactory indexVarFactory;
    std::vector<IndexExpr::IndexVarPtr> tensorIndexVars;
    for (unsigned int i=0; i<tensor->getOrder(); ++i) {
      IndexSetProduct indexSet = tensor->getType()->getDimensions()[i];
      tensorIndexVars.push_back(indexVarFactory.makeFreeVar(indexSet));
    }

    std::vector<IndexExpr::IndexedTensor> indexedOperands;
    indexedOperands.push_back(IndexExpr::IndexedTensor(scalar,scalarIndexVars));
    indexedOperands.push_back(IndexExpr::IndexedTensor(tensor,tensorIndexVars));
    return new IndexExpr(tensorIndexVars, op, indexedOperands);
  }
  else {
    std::vector<std::shared_ptr<TensorNode>> operands;
    operands.push_back(l);
    operands.push_back(r);
    return elwiseExpr(op, operands);
  }
}

IndexExpr *elwiseExpr(IndexExpr::Operator op,
                      std::vector<std::shared_ptr<TensorNode>> &operands) {
  assert((size_t)IndexExpr::numOperands(op) == operands.size());

  IndexVarFactory indexVarFactory;
  std::vector<IndexExpr::IndexVarPtr> indexVars;
  for (unsigned int i=0; i<operands[0]->getOrder(); ++i) {
    IndexSetProduct indexSet = operands[0]->getType()->getDimensions()[i];
    indexVars.push_back(indexVarFactory.makeFreeVar(indexSet));
  }

  std::vector<IndexExpr::IndexedTensor> indexedOperands;
  for (auto &operand : operands) {
    indexedOperands.push_back(IndexExpr::IndexedTensor(operand, indexVars));
  }
  return new IndexExpr(indexVars, op, indexedOperands);
}

IndexExpr *innerProduct(const std::shared_ptr<TensorNode> &l,
                        const std::shared_ptr<TensorNode> &r) {
  assert(*l->getType() == *r->getType());
  return NULL;
}

IndexExpr *outerProduct(const std::shared_ptr<TensorNode> &l,
                        const std::shared_ptr<TensorNode> &r) {
  assert(*l->getType() == *r->getType());

  IndexVarFactory indexVarFactory;
  auto i = indexVarFactory.makeFreeVar(l->getType()->getDimensions()[0]);
  auto j = indexVarFactory.makeFreeVar(l->getType()->getDimensions()[0]);

  std::vector<IndexExpr::IndexVarPtr> iIdxVar;
  std::vector<IndexExpr::IndexVarPtr> jIdxVar;
  std::vector<IndexExpr::IndexVarPtr> idxVars;
  iIdxVar.push_back(i);
  idxVars.push_back(i);
  jIdxVar.push_back(j);
  idxVars.push_back(j);

  std::vector<IndexExpr::IndexedTensor> indexedOperands;
  indexedOperands.push_back(IndexExpr::IndexedTensor(l, iIdxVar));
  indexedOperands.push_back(IndexExpr::IndexedTensor(r, jIdxVar));
  return new IndexExpr(idxVars, IndexExpr::Operator::MUL, indexedOperands);
}

IndexExpr *transposeMatrix(const std::shared_ptr<TensorNode> &mat) {
  assert(mat->getOrder() == 2);
  const std::vector<IndexSetProduct> &dims = mat->getType()->getDimensions();

  IndexVarFactory indexVarFactory;
  std::vector<IndexExpr::IndexVarPtr> indexVars;
  indexVars.push_back(indexVarFactory.makeFreeVar(dims[1]));
  indexVars.push_back(indexVarFactory.makeFreeVar(dims[0]));

  std::vector<IndexExpr::IndexVarPtr> operandIndexVars(indexVars.rbegin(),
                                                       indexVars.rend());
  std::vector<IndexExpr::IndexedTensor> operands;
  operands.push_back(IndexExpr::IndexedTensor(mat, operandIndexVars));

  return new IndexExpr(indexVars, IndexExpr::Operator::NONE, operands);
}

}} // namespace simit::internal
