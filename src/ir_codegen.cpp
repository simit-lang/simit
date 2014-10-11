#include "ir_codegen.h"

#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

using namespace std;

namespace simit {
namespace ir {

// class IndexVarFactory
shared_ptr<IndexVar> IndexVarFactory::makeIndexVar(const IndexDomain &domain) {
  auto freeIndexVar = new IndexVar(makeName(), domain);
  return std::shared_ptr<IndexVar>(freeIndexVar);
}

shared_ptr<IndexVar> IndexVarFactory::makeIndexVar(const IndexDomain &domain,
                                                   ReductionOperator rop) {
  auto reductionIndexVar = new IndexVar(makeName(), domain, rop);
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

  const TensorType *ltype = l->getType().toTensor();
  const TensorType *rtype = r->getType().toTensor();

  if (ltype->order() == 0 || rtype->order() == 0) {
    std::shared_ptr<Expression> scalar;
    std::shared_ptr<Expression> tensor;
    if (ltype->order() == 0) {
      scalar = l;
      tensor = r;
    }
    else {
      scalar = r;
      tensor = l;
    }
    assert(scalar->getType().toTensor()->order() == 0);

    std::vector<std::shared_ptr<IndexVar>> scalarIndexVars;

    IndexVarFactory factory;
    std::vector<std::shared_ptr<IndexVar>> tensorIndexVars;
    const TensorType *tensorType = tensor->getType().toTensor();
    for (unsigned int i=0; i < tensorType->order(); ++i) {
      IndexDomain domain = tensorType->dimensions[i];
      tensorIndexVars.push_back(factory.makeIndexVar(domain));
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

  IndexVarFactory factory;
  std::vector<std::shared_ptr<IndexVar>> indexVars;
  const TensorType *ttype = operands[0]->getType().toTensor();
  for (unsigned int i=0; i < ttype->order(); ++i) {
    IndexDomain domain = ttype->dimensions[i];
    indexVars.push_back(factory.makeIndexVar(domain));
  }

  std::vector<IndexedTensor> indexedOperands;
  for (auto &operand : operands) {
    indexedOperands.push_back(IndexedTensor(operand, indexVars));
  }
  return new IndexExpr(indexVars, op, indexedOperands);
}

IndexExpr *innerProduct(const std::shared_ptr<Expression> &l,
                        const std::shared_ptr<Expression> &r) {
  assert(l->getType() == r->getType());
  const TensorType *ltype = l->getType().toTensor();

  IndexVarFactory factory;
  auto i = factory.makeIndexVar(ltype->dimensions[0], ReductionOperator::Sum);

  std::vector<IndexedTensor> operands;
  operands.push_back(IndexedTensor(l, indexVars(i)));
  operands.push_back(IndexedTensor(r, indexVars(i)));

  std::vector<std::shared_ptr<IndexVar>> none;
  return new IndexExpr(none, IndexExpr::Operator::MUL, operands);
}

IndexExpr *outerProduct(const std::shared_ptr<Expression> &l,
                        const std::shared_ptr<Expression> &r) {
  assert(l->getType() == r->getType());
  const TensorType *ltype = l->getType().toTensor();

  IndexVarFactory factory;
  auto i = factory.makeIndexVar(ltype->dimensions[0]);
  auto j = factory.makeIndexVar(ltype->dimensions[0]);

  std::vector<IndexedTensor> operands;
  operands.push_back(IndexedTensor(l, indexVars(i)));
  operands.push_back(IndexedTensor(r, indexVars(j)));

  return new IndexExpr(indexVars(i,j), IndexExpr::Operator::MUL, operands);
}

IndexExpr *gemv(const std::shared_ptr<Expression> &l,
                const std::shared_ptr<Expression> &r) {
  const TensorType *ltype = l->getType().toTensor();
  const TensorType *rtype = r->getType().toTensor();

  assert(ltype->order() == 2 && rtype->order() == 1);
  assert(ltype->dimensions[1] == rtype->dimensions[0]);

  IndexVarFactory factory;
  auto i = factory.makeIndexVar(ltype->dimensions[0]);
  auto j = factory.makeIndexVar(ltype->dimensions[1], ReductionOperator::Sum);

  std::vector<IndexedTensor> operands;
  operands.push_back(IndexedTensor(l, indexVars(i, j)));
  operands.push_back(IndexedTensor(r, indexVars(j)));

  return new IndexExpr(indexVars(i), IndexExpr::Operator::MUL, operands);
}

IndexExpr *gevm(const std::shared_ptr<Expression> &l,
                const std::shared_ptr<Expression> &r) {
  const TensorType *ltype = l->getType().toTensor();
  const TensorType *rtype = r->getType().toTensor();

  assert(ltype->order() == 1 && rtype->order() == 2);
  assert(ltype->dimensions[0] == rtype->dimensions[0]);

  IndexVarFactory factory;
  auto i = factory.makeIndexVar(rtype->dimensions[1]);
  auto j = factory.makeIndexVar(rtype->dimensions[0], ReductionOperator::Sum);

  std::vector<IndexedTensor> operands;
  operands.push_back(IndexedTensor(l, indexVars(j)));
  operands.push_back(IndexedTensor(r, indexVars(j,i)));

  return new IndexExpr(indexVars(i), IndexExpr::Operator::MUL, operands);
}

IndexExpr *gemm(const std::shared_ptr<Expression> &l,
                const std::shared_ptr<Expression> &r) {
  const TensorType *ltype = l->getType().toTensor();
  const TensorType *rtype = r->getType().toTensor();

  assert(ltype->order() == 2 && rtype->order() == 2);
  assert(ltype->dimensions[1] == rtype->dimensions[0]);

  IndexVarFactory factory;
  auto i = factory.makeIndexVar(ltype->dimensions[0]);
  auto j = factory.makeIndexVar(rtype->dimensions[1]);
  auto k = factory.makeIndexVar(ltype->dimensions[1], ReductionOperator::Sum);

  std::vector<IndexedTensor> operands;
  operands.push_back(IndexedTensor(l, indexVars(i,k)));
  operands.push_back(IndexedTensor(r, indexVars(k,j)));

  return new IndexExpr(indexVars(i,j), IndexExpr::Operator::MUL, operands);
}

IndexExpr *transposeMatrix(const std::shared_ptr<Expression> &mat) {
  const TensorType *mattype = mat->getType().toTensor();

  assert(mattype->order() == 2);
  const std::vector<IndexDomain> &dims = mattype->dimensions;

  IndexVarFactory factory;
  std::vector<std::shared_ptr<IndexVar>> indexVars;
  indexVars.push_back(factory.makeIndexVar(dims[1]));
  indexVars.push_back(factory.makeIndexVar(dims[0]));

  std::vector<std::shared_ptr<IndexVar>> operandIndexVars(indexVars.rbegin(),
                                                       indexVars.rend());
  std::vector<IndexedTensor> operands;
  operands.push_back(IndexedTensor(mat, operandIndexVars));

  return new IndexExpr(indexVars, IndexExpr::Operator::NONE, operands);
}

}} // namespace simit::internal
