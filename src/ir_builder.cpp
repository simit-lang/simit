#include "ir_builder.h"

#include <iostream>
#include <memory>
#include <vector>
#include <algorithm>

using namespace std;

namespace simit {
namespace ir {

// class UniqueNameGenerator
std::string UniqueNameGenerator::getName() {
  return getName("tmp");
}

std::string UniqueNameGenerator::getName(const std::string &suggestion) {
  if (takenNames.find(suggestion) == takenNames.end()) {
    takenNames[suggestion] = 1;
    return suggestion;
  }
  else {
    string name = suggestion;
    do {
      name = suggestion + to_string(takenNames[suggestion]++);
    } while (takenNames.find(name) != takenNames.end());
    return  getName(name);
  }
}


// class IndexVarFactory
IndexVar IndexVarFactory::createIndexVar(const IndexDomain &domain) {
  return IndexVar(makeName(), domain);
}

IndexVar IndexVarFactory::createIndexVar(const IndexDomain &domain,
                                       ReductionOperator rop) {
  return IndexVar(makeName(), domain, rop);
}

std::string IndexVarFactory::makeName() {
  char name[2];
  name[0] = 'i' + nameID;
  name[1] = '\0';
  nameID++;
  if (nameID == 18) {
    nameID = 0;
  }
  return std::string(name);
}


// class IRBuilder
Expr IRBuilder::unaryElwiseExpr(UnaryOperator op, Expr e) {
  std::vector<IndexVar> indexVars;
  const TensorType *tensorType = e.type().toTensor();
  for (unsigned int i=0; i < tensorType->order(); ++i) {
    IndexDomain domain = tensorType->dimensions[i];
    indexVars.push_back(factory.createIndexVar(domain));
  }

  Expr a = IndexedTensor::make(e, indexVars);

  Expr val;
  switch (op) {
    case None:
      val = a;
      break;
    case Neg:
      val = Neg::make(a);
      break;
  }
  iassert(val.defined());

  return IndexExpr::make(indexVars, val);
}

Expr IRBuilder::binaryElwiseExpr(Expr l, BinaryOperator op, Expr r) {
  const TensorType *ltype = l.type().toTensor();
  const TensorType *rtype = r.type().toTensor();

  Expr tensor = (ltype->order() > 0) ? l : r;

  std::vector<IndexVar> indexVars;
  const TensorType *tensorType = tensor.type().toTensor();
  for (unsigned int i=0; i < tensorType->order(); ++i) {
    IndexDomain domain = tensorType->dimensions[i];
    indexVars.push_back(factory.createIndexVar(domain));
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
    iassert(l.type() == r.type());
    a = IndexedTensor::make(l, indexVars);
    b = IndexedTensor::make(r, indexVars);
  }
  iassert(a.defined() && b.defined());

  Expr val;
  switch (op) {
    case Add:
      val = Add::make(a, b);
      break;
    case Sub:
      val = Sub::make(a, b);
      break;
    case Mul:
      val = Mul::make(a, b);
      break;
    case Div:
      val = Div::make(a, b);
      break;
  }
  iassert(val.defined());

  return IndexExpr::make(indexVars, val);
}

Expr IRBuilder::innerProduct(Expr l, Expr r) {
  iassert(l.type() == r.type());
  const TensorType *ltype = l.type().toTensor();

  auto i = factory.createIndexVar(ltype->dimensions[0], ReductionOperator::Sum);

  Expr a = IndexedTensor::make(l, {i});
  Expr b = IndexedTensor::make(r, {i});
  Expr val = Mul::make(a, b);

  std::vector<IndexVar> none;
  return IndexExpr::make(none, val);
}

Expr IRBuilder::outerProduct(Expr l, Expr r) {
  iassert(l.type() == r.type());
  const TensorType *ltype = l.type().toTensor();

  auto i = factory.createIndexVar(ltype->dimensions[0]);
  auto j = factory.createIndexVar(ltype->dimensions[0]);

  Expr a = IndexedTensor::make(l, {i});
  Expr b = IndexedTensor::make(r, {j});
  Expr val = Mul::make(a, b);

  return IndexExpr::make({i,j}, val);
}

Expr IRBuilder::gemv(Expr l, Expr r) {
  const TensorType *ltype = l.type().toTensor();
  const TensorType *rtype = r.type().toTensor();

  iassert(ltype->order() == 2 && rtype->order() == 1);
  iassert(ltype->dimensions[1] == rtype->dimensions[0]);
//  iassert(rtype->isColumnVector);

  auto i = factory.createIndexVar(ltype->dimensions[0]);
  auto j = factory.createIndexVar(ltype->dimensions[1], ReductionOperator::Sum);

  Expr a = IndexedTensor::make(l, {i, j});
  Expr b = IndexedTensor::make(r, {j});
  Expr val = Mul::make(a, b);

  Expr gemv = IndexExpr::make({i}, val);

  // This is a hack and we should find another solution.
  const_cast<TensorType*>(gemv.type().toTensor())->isColumnVector = true;

  return gemv;
}

Expr IRBuilder::gevm(Expr l, Expr r) {
  const TensorType *ltype = l.type().toTensor();
  const TensorType *rtype = r.type().toTensor();

  iassert(ltype->order() == 1 && rtype->order() == 2);
  iassert(ltype->dimensions[0] == rtype->dimensions[0]);

  auto i = factory.createIndexVar(rtype->dimensions[1]);
  auto j = factory.createIndexVar(rtype->dimensions[0], ReductionOperator::Sum);

  Expr a = IndexedTensor::make(l, {j});
  Expr b = IndexedTensor::make(r, {j,i});
  Expr val = Mul::make(a, b);

  return IndexExpr::make({i}, val);
}

Expr IRBuilder::gemm(Expr l, Expr r) {
  const TensorType *ltype = l.type().toTensor();
  const TensorType *rtype = r.type().toTensor();

  iassert(ltype->order() == 2 && rtype->order() == 2);
  iassert(ltype->dimensions[1] == rtype->dimensions[0]);

  auto i = factory.createIndexVar(ltype->dimensions[0]);
  auto j = factory.createIndexVar(rtype->dimensions[1]);
  auto k = factory.createIndexVar(ltype->dimensions[1], ReductionOperator::Sum);

  Expr a = IndexedTensor::make(l, {i,k});
  Expr b = IndexedTensor::make(r, {k,j});
  Expr val = Mul::make(a, b);

  return IndexExpr::make({i,j}, val);
}

Expr IRBuilder::transposedMatrix(Expr mat) {
  const TensorType *mattype = mat.type().toTensor();

  iassert(mattype->order() == 2);
  const std::vector<IndexDomain> &dims = mattype->dimensions;

  std::vector<IndexVar> indexVars;
  indexVars.push_back(factory.createIndexVar(dims[1]));
  indexVars.push_back(factory.createIndexVar(dims[0]));

  std::vector<IndexVar> operandIndexVars(indexVars.rbegin(), indexVars.rend());
  Expr val = IndexedTensor::make(mat, operandIndexVars);
  
  return IndexExpr::make(indexVars, val);
}

}} // namespace simit::internal
