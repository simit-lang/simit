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
IndexExpr *binaryElwiseExpr(const std::shared_ptr<TensorNode> &l,
                            IndexExpr::Operator op,
                            const std::shared_ptr<TensorNode> &r) {
  IndexVarFactory indexVarFactory;
  std::vector<IndexExpr::IndexVarPtr> indexVars;
  for (unsigned int i=0; i<l->getOrder(); ++i) {
    IndexSetProduct indexSet = l->getType()->getDimensions()[i];
    indexVars.push_back(indexVarFactory.makeFreeVar(indexSet));
  }

  std::vector<IndexExpr::IndexedTensor> operands;
  operands.push_back(IndexExpr::IndexedTensor(l, indexVars));
  operands.push_back(IndexExpr::IndexedTensor(r, indexVars));
  return new IndexExpr(indexVars, op, operands);
}

IndexExpr *transposeMatrix(const std::shared_ptr<TensorNode> &mat) {
  assert(mat->getOrder() == 2);
  const std::vector<IndexSetProduct> &dims = mat->getType()->getDimensions();
//  std::vector<IndexSetProduct> transposedDims(dims.rbegin(), dims.rend());
//  TensorType *transposedType = new TensorType(type->getComponentType(),
//                                              transposedDims);

  IndexVarFactory indexVarFactory;
  std::vector<IndexExpr::IndexVarPtr> indexVars;
  indexVars.push_back(indexVarFactory.makeFreeVar(dims[0]));
  indexVars.push_back(indexVarFactory.makeFreeVar(dims[1]));

  std::vector<IndexExpr::IndexedTensor> operands;
  operands.push_back(IndexExpr::IndexedTensor(mat, indexVars));

  std::reverse(indexVars.begin(), indexVars.end());

  return new IndexExpr(indexVars, IndexExpr::Operator::NONE, operands);
}

}} // namespace simit::internal
