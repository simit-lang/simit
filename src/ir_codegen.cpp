#include "ir_codegen.h"

#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

using namespace std;

namespace simit {
namespace ir {

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

std::vector<std::shared_ptr<IndexVar>>
indexVars(const std::shared_ptr<IndexVar> &v1) {
  std::vector<std::shared_ptr<IndexVar>> idxVars;
  idxVars.push_back(v1);
  return idxVars;
}

std::vector<std::shared_ptr<IndexVar>>
indexVars(const std::shared_ptr<IndexVar> &v1,
          const std::shared_ptr<IndexVar> &v2) {
  std::vector<std::shared_ptr<IndexVar>> idxVars;
  idxVars.push_back(v1);
  idxVars.push_back(v2);
  return idxVars;
}

// Free functions
IndexExpr *unaryElwiseExpr(IndexExpr::Operator op,
                           const std::shared_ptr<Expression> &expr) {
  assert(IndexExpr::numOperands(op) == 1);
  std::vector<std::shared_ptr<Expression>> operands;
  operands.push_back(expr);
  return elwiseExpr(op, operands);
}

IndexExpr *binaryElwiseExpr(const std::shared_ptr<Expression> &l,
                            IndexExpr::Operator op,
                            const std::shared_ptr<Expression> &r) {
  assert(IndexExpr::numOperands(op) == 2);

  TensorType *ltype = tensorTypePtr(l->getType());
  TensorType *rtype = tensorTypePtr(r->getType());

  if (ltype->getOrder() == 0 || rtype->getOrder() == 0) {
    std::shared_ptr<Expression> scalar;
    std::shared_ptr<Expression> tensor;
    if (ltype->getOrder() == 0) {
      scalar = l;
      tensor = r;
    }
    else {
      scalar = r;
      tensor = l;
    }
    assert(tensorTypePtr(scalar->getType())->getOrder() == 0);

    std::vector<std::shared_ptr<IndexVar>> scalarIndexVars;

    IndexVarFactory indexVarFactory;
    std::vector<std::shared_ptr<IndexVar>> tensorIndexVars;
    TensorType *tensorType = tensorTypePtr(tensor->getType());
    for (unsigned int i=0; i < tensorType->getOrder(); ++i) {
      IndexSetProduct indexSet = tensorType->getDimensions()[i];
      tensorIndexVars.push_back(indexVarFactory.makeFreeVar(indexSet));
    }

    std::vector<IndexedTensor> indexedOperands;
    indexedOperands.push_back(IndexedTensor(scalar,scalarIndexVars));
    indexedOperands.push_back(IndexedTensor(tensor,tensorIndexVars));
    return new IndexExpr(tensorIndexVars, op, indexedOperands);
  }
  else {
    std::vector<std::shared_ptr<Expression>> operands;
    operands.push_back(l);
    operands.push_back(r);
    return elwiseExpr(op, operands);
  }
}

IndexExpr *elwiseExpr(IndexExpr::Operator op,
                      std::vector<std::shared_ptr<Expression>> &operands) {
  assert((size_t)IndexExpr::numOperands(op) == operands.size());

  IndexVarFactory indexVarFactory;
  std::vector<std::shared_ptr<IndexVar>> indexVars;
  TensorType *ttype = tensorTypePtr(operands[0]->getType());
  for (unsigned int i=0; i < ttype->getOrder(); ++i) {
    IndexSetProduct indexSet = ttype->getDimensions()[i];
    indexVars.push_back(indexVarFactory.makeFreeVar(indexSet));
  }

  std::vector<IndexedTensor> indexedOperands;
  for (auto &operand : operands) {
    indexedOperands.push_back(IndexedTensor(operand, indexVars));
  }
  return new IndexExpr(indexVars, op, indexedOperands);
}

IndexExpr *innerProduct(const std::shared_ptr<Expression> &l,
                        const std::shared_ptr<Expression> &r) {
  TensorType *ltype = tensorTypePtr(l->getType());
  TensorType *rtype = tensorTypePtr(r->getType());

  assert(ltype->getOrder() == 1 && rtype->getOrder() == 1);
  assert(*l->getType() == *r->getType());

  IndexVarFactory indexVarFactory;
  auto i = indexVarFactory.makeReductionVar(ltype->getDimensions()[0],
                                            IndexVar::Operator::SUM);

  std::vector<IndexedTensor> operands;
  operands.push_back(IndexedTensor(l, indexVars(i)));
  operands.push_back(IndexedTensor(r, indexVars(i)));

  std::vector<std::shared_ptr<IndexVar>> none;
  return new IndexExpr(none, IndexExpr::Operator::MUL, operands);
}

IndexExpr *outerProduct(const std::shared_ptr<Expression> &l,
                        const std::shared_ptr<Expression> &r) {
  TensorType *ltype = tensorTypePtr(l->getType());
  TensorType *rtype = tensorTypePtr(r->getType());

  assert(ltype->getOrder() == 1 && rtype->getOrder() == 1);
  assert(*ltype == *r->getType());

  IndexVarFactory indexVarFactory;
  auto i = indexVarFactory.makeFreeVar(ltype->getDimensions()[0]);
  auto j = indexVarFactory.makeFreeVar(ltype->getDimensions()[0]);

  std::vector<IndexedTensor> operands;
  operands.push_back(IndexedTensor(l, indexVars(i)));
  operands.push_back(IndexedTensor(r, indexVars(j)));

  return new IndexExpr(indexVars(i,j), IndexExpr::Operator::MUL, operands);
}

IndexExpr *gemv(const std::shared_ptr<Expression> &l,
                const std::shared_ptr<Expression> &r) {
  TensorType *ltype = tensorTypePtr(l->getType());
  TensorType *rtype = tensorTypePtr(r->getType());

  assert(ltype->getOrder() == 2 && rtype->getOrder() == 1);
  assert(ltype->getDimensions()[1] == rtype->getDimensions()[0]);

  IndexVarFactory indexVarFactory;
  auto i = indexVarFactory.makeFreeVar(ltype->getDimensions()[0]);
  auto j = indexVarFactory.makeReductionVar(ltype->getDimensions()[1],
                                            IndexVar::Operator::SUM);

  std::vector<IndexedTensor> operands;
  operands.push_back(IndexedTensor(l, indexVars(i, j)));
  operands.push_back(IndexedTensor(r, indexVars(j)));

  return new IndexExpr(indexVars(i), IndexExpr::Operator::MUL, operands);
}

IndexExpr *gevm(const std::shared_ptr<Expression> &l,
                const std::shared_ptr<Expression> &r) {
  TensorType *ltype = tensorTypePtr(l->getType());
  TensorType *rtype = tensorTypePtr(r->getType());

  assert(ltype->getOrder() == 1 && rtype->getOrder() == 2);
  assert(ltype->getDimensions()[0] == rtype->getDimensions()[0]);

  IndexVarFactory indexVarFactory;
  auto i = indexVarFactory.makeFreeVar(rtype->getDimensions()[1]);
  auto j = indexVarFactory.makeReductionVar(rtype->getDimensions()[0],
                                            IndexVar::Operator::SUM);

  std::vector<IndexedTensor> operands;
  operands.push_back(IndexedTensor(l, indexVars(j)));
  operands.push_back(IndexedTensor(r, indexVars(j,i)));

  return new IndexExpr(indexVars(i), IndexExpr::Operator::MUL, operands);
}

IndexExpr *gemm(const std::shared_ptr<Expression> &l,
                const std::shared_ptr<Expression> &r) {
  TensorType *ltype = tensorTypePtr(l->getType());
  TensorType *rtype = tensorTypePtr(r->getType());

  assert(ltype->getOrder() == 2 && rtype->getOrder() == 2);
  assert(ltype->getDimensions()[1] == rtype->getDimensions()[0]);

  IndexVarFactory indexVarFactory;
  auto i = indexVarFactory.makeFreeVar(ltype->getDimensions()[0]);
  auto j = indexVarFactory.makeFreeVar(rtype->getDimensions()[1]);
  auto k = indexVarFactory.makeReductionVar(ltype->getDimensions()[1],
                                            IndexVar::Operator::SUM);

  std::vector<IndexedTensor> operands;
  operands.push_back(IndexedTensor(l, indexVars(i,k)));
  operands.push_back(IndexedTensor(r, indexVars(k,j)));

  return new IndexExpr(indexVars(i,j), IndexExpr::Operator::MUL, operands);
}

IndexExpr *transposeMatrix(const std::shared_ptr<Expression> &mat) {
  TensorType *mattype = tensorTypePtr(mat->getType());

  assert(mattype->getOrder() == 2);
  const std::vector<IndexSetProduct> &dims = mattype->getDimensions();

  IndexVarFactory indexVarFactory;
  std::vector<std::shared_ptr<IndexVar>> indexVars;
  indexVars.push_back(indexVarFactory.makeFreeVar(dims[1]));
  indexVars.push_back(indexVarFactory.makeFreeVar(dims[0]));

  std::vector<std::shared_ptr<IndexVar>> operandIndexVars(indexVars.rbegin(),
                                                       indexVars.rend());
  std::vector<IndexedTensor> operands;
  operands.push_back(IndexedTensor(mat, operandIndexVars));

  return new IndexExpr(indexVars, IndexExpr::Operator::NONE, operands);
}

}} // namespace simit::internal
