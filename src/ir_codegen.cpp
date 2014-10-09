#include "ir_codegen.h"

#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

using namespace std;

namespace simit {
namespace ir {

// class IndexVarFactory
IndexVar IndexVarFactory::makeIndexVar(const IndexDomain &domain) {
  return IndexVar(makeName(), domain);
}

IndexVar IndexVarFactory::makeIndexVar(const IndexDomain &domain,
                                       ReductionOperator rop) {
  return IndexVar(makeName(), domain, rop);
}

std::string IndexVarFactory::makeName() {
  char name[2];
  name[0] = 'i' + nameID;
  name[1] = '\0';
  nameID++;
  return std::string(name);
}

std::vector<IndexVar> indexVars(const IndexVar &v1) {
  std::vector<IndexVar> idxVars;
  idxVars.push_back(v1);
  return idxVars;
}

std::vector<IndexVar> indexVars(const IndexVar &v1, const IndexVar &v2) {
  std::vector<IndexVar> idxVars;
  idxVars.push_back(v1);
  idxVars.push_back(v2);
  return idxVars;
}

// Free functions
Expr unaryElwiseExpr(UnaryOperator op, Expr e) {
  std::vector<IndexVar> indexVars;
  IndexVarFactory factory;
  const TensorType *tensorType = e.type().toTensor();
  for (unsigned int i=0; i < tensorType->order(); ++i) {
    IndexDomain domain = tensorType->dimensions[i];
    indexVars.push_back(factory.makeIndexVar(domain));
  }

  Expr a = IndexedTensor::make(e, indexVars);

  Expr expr;
  switch (op) {
    case None:
      expr = a;
      break;
    case Neg:
      expr = Neg::make(a);
      break;
  }
  assert(expr.defined());

  return IndexExpr::make(indexVars, expr);
}

Expr binaryElwiseExpr(Expr l, BinaryOperator op, Expr r) {
  const TensorType *ltype = l.type().toTensor();
  const TensorType *rtype = r.type().toTensor();

  Expr tensor = (ltype->order() > 0) ? l : r;

  std::vector<IndexVar> indexVars;
  IndexVarFactory factory;
  const TensorType *tensorType = tensor.type().toTensor();
  for (unsigned int i=0; i < tensorType->order(); ++i) {
    IndexDomain domain = tensorType->dimensions[i];
    indexVars.push_back(factory.makeIndexVar(domain));
  }

  Expr a, b;
  if (ltype->order() == 0 || rtype->order() == 0) {
    std::vector<IndexVar> scalarIndexVars;

    std::vector<IndexVar> *lIndexVars;
    std::vector<IndexVar> *rIndexVars;
    if (ltype->order() == 0) {
      lIndexVars = &scalarIndexVars;
      rIndexVars = &indexVars;
    }
    else {
      lIndexVars = &indexVars;
      rIndexVars = &scalarIndexVars;
    }

    a = IndexedTensor::make(l, *lIndexVars);
    b = IndexedTensor::make(r, *rIndexVars);
  }
  else {
    assert(l.type() == r.type());
    a = IndexedTensor::make(l, indexVars);
    b = IndexedTensor::make(r, indexVars);
  }
  assert(a.defined() && b.defined());

  Expr expr;
  switch (op) {
    case Add:
      expr = Add::make(a, b);
      break;
    case Sub:
      expr = Sub::make(a, b);
      break;
    case Mul:
      expr = Mul::make(a, b);
      break;
    case Div:
      expr = Div::make(a, b);
      break;
  }
  assert(expr.defined());

  return IndexExpr::make(indexVars, expr);
}

Expr innerProduct(Expr l, Expr r) {
  assert(l.type() == r.type());
  const TensorType *ltype = l.type().toTensor();

  IndexVarFactory factory;
  auto i = factory.makeIndexVar(ltype->dimensions[0], ReductionOperator::Sum);

  Expr a = IndexedTensor::make(l, indexVars(i));
  Expr b = IndexedTensor::make(r, indexVars(i));
  Expr mul = Mul::make(a, b);

  std::vector<IndexVar> none;
  return IndexExpr::make(none, mul);
}

Expr outerProduct(Expr l, Expr r) {
  assert(l.type() == r.type());
  const TensorType *ltype = l.type().toTensor();

  IndexVarFactory factory;
  auto i = factory.makeIndexVar(ltype->dimensions[0]);
  auto j = factory.makeIndexVar(ltype->dimensions[0]);

  Expr a = IndexedTensor::make(l, indexVars(i));
  Expr b = IndexedTensor::make(r, indexVars(j));
  Expr mul = Mul::make(a, b);

  return IndexExpr::make(indexVars(i,j), mul);
}

Expr gemv(Expr l, Expr r) {
  const TensorType *ltype = l.type().toTensor();
  const TensorType *rtype = r.type().toTensor();

  assert(ltype->order() == 2 && rtype->order() == 1);
  assert(ltype->dimensions[1] == rtype->dimensions[0]);

  IndexVarFactory factory;
  auto i = factory.makeIndexVar(ltype->dimensions[0]);
  auto j = factory.makeIndexVar(ltype->dimensions[1], ReductionOperator::Sum);

  Expr a = IndexedTensor::make(l, indexVars(i, j));
  Expr b = IndexedTensor::make(r, indexVars(j));
  Expr mul = Mul::make(a, b);

  return IndexExpr::make(indexVars(i), mul);
}

Expr gevm(Expr l, Expr r) {
  const TensorType *ltype = l.type().toTensor();
  const TensorType *rtype = r.type().toTensor();

  assert(ltype->order() == 1 && rtype->order() == 2);
  assert(ltype->dimensions[0] == rtype->dimensions[0]);

  IndexVarFactory factory;
  auto i = factory.makeIndexVar(rtype->dimensions[1]);
  auto j = factory.makeIndexVar(rtype->dimensions[0], ReductionOperator::Sum);

  Expr a = IndexedTensor::make(l, indexVars(j));
  Expr b = IndexedTensor::make(r, indexVars(j,i));
  Expr mul = Mul::make(a, b);

  return IndexExpr::make(indexVars(i), mul);
}

Expr gemm(Expr l, Expr r) {
  const TensorType *ltype = l.type().toTensor();
  const TensorType *rtype = r.type().toTensor();

  assert(ltype->order() == 2 && rtype->order() == 2);
  assert(ltype->dimensions[1] == rtype->dimensions[0]);

  IndexVarFactory factory;
  auto i = factory.makeIndexVar(ltype->dimensions[0]);
  auto j = factory.makeIndexVar(rtype->dimensions[1]);
  auto k = factory.makeIndexVar(ltype->dimensions[1], ReductionOperator::Sum);

  Expr a = IndexedTensor::make(l, indexVars(i,k));
  Expr b = IndexedTensor::make(r, indexVars(k,j));
  Expr mul = Mul::make(a, b);

  return IndexExpr::make(indexVars(i,j), mul);
}

Expr transposeMatrix(Expr mat) {
  const TensorType *mattype = mat.type().toTensor();

  assert(mattype->order() == 2);
  const std::vector<IndexDomain> &dims = mattype->dimensions;

  IndexVarFactory factory;
  std::vector<IndexVar> indexVars;
  indexVars.push_back(factory.makeIndexVar(dims[1]));
  indexVars.push_back(factory.makeIndexVar(dims[0]));

  std::vector<IndexVar> operandIndexVars(indexVars.rbegin(),
                                                       indexVars.rend());
  Expr expr = IndexedTensor::make(mat, operandIndexVars);

  return IndexExpr::make(indexVars, expr);
}

}} // namespace simit::internal
