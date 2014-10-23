#include "ir.h"

#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <string.h>

#include "types.h"
#include "util.h"
#include "macros.h"
#include "ir_printer.h"

using namespace std;

namespace simit {
namespace ir {

// class Expr
Expr::Expr(const Var &var) : Expr(VarExpr::make(var)) {}

static size_t getTensorByteSize(const TensorType *tensorType) {
  return tensorType->size() * tensorType->componentType.bytes();
}

// class Expr
Expr::Expr(int val) : IntrusivePtr(Literal::make(Int(), &val)) {
}

Expr::Expr(double val) : IntrusivePtr(Literal::make(Float(), &val)) {
}

bool operator==(const Expr &l, const Expr &r) {
  return l.expr() == r.expr();
}

bool operator!=(const Expr &l, const Expr &r) {
  return !(l == r);
}

// class Intrinsics
Func Intrinsics::sin =
    Func("sin", {Var("x", Float())}, {Var("r", Float())}, Func::Intrinsic);
Func Intrinsics::cos =
    Func("cos", {Var("x", Float())}, {Var("r", Float())}, Func::Intrinsic);
Func Intrinsics::atan2 =
    Func("atan2", {Var("y", Float()), Var("x", Float())},
                  {Var("r", Float())}, Func::Intrinsic);
Func Intrinsics::sqrt =
    Func("sqrt", {Var("x", Float())}, {Var("r", Float())}, Func::Intrinsic);
Func Intrinsics::log =
    Func("log", {Var("x", Float())}, {Var("r", Float())}, Func::Intrinsic);
Func Intrinsics::exp =
    Func("exp", {Var("x", Float())}, {Var("r", Float())}, Func::Intrinsic);

std::map<std::string, Func> Intrinsics::byName = {{"sin",sin},
                                                  {"cos",cos},
                                                  {"atan2",atan2},
                                                  {"sqrt",sqrt},
                                                  {"log",log},
                                                  {"exp",exp}};

// Type compute functions
Type fieldType(Expr elementOrSet, std::string fieldName) {
  assert(elementOrSet.type().isElement() || elementOrSet.type().isSet());

  Type fieldType;
  if (elementOrSet.type().isElement()) {
    const ElementType *elemType = elementOrSet.type().toElement();
    fieldType = elemType->fields.at(fieldName);
  }
  else if (elementOrSet.type().isSet()) {
    const SetType *setType = elementOrSet.type().toSet();
    const ElementType *elemType = setType->elementType.toElement();

    const TensorType *elemFieldType = elemType->fields.at(fieldName).toTensor();

    // The type of a set field is:
    // `Tensor[set][elementFieldDimensions](elemFieldComponentType)`
    std::vector<IndexDomain> dimensions;
    if (elemFieldType->order() == 0) {
      dimensions.push_back(IndexDomain(IndexSet(elementOrSet)));
    }
    else {
      std::vector<IndexSet> dim;
      dim.push_back(IndexSet(elementOrSet));

      for (const IndexDomain &elemFieldDim : elemFieldType->dimensions) {
        for (const IndexSet &indexSet : elemFieldDim.getIndexSets()) {
          dim.push_back(indexSet);
        }
        dimensions.push_back(IndexDomain(dim));
      }
    }
    fieldType = TensorType::make(elemFieldType->componentType, dimensions);
  }

  return fieldType;
}

// TODO: The logic in this function is wacky and doesn't work with nests
Type blockType(Expr tensor) {
  assert(tensor.type().isTensor());

  const TensorType *type = tensor.type().toTensor();
  const std::vector<IndexDomain> &dimensions = type->dimensions;
  if (dimensions.size() == 0) {
    return TensorType::make(type->componentType);
  }

  std::vector<IndexDomain> blockDimensions;

  size_t numNests = dimensions[0].getIndexSets().size();
  for (auto &dim : dimensions) {
    assert(dim.getIndexSets().size() == numNests &&
           "All dimensions should have the same number of nestings");
  }

  Type blockType;
  if (numNests) {
    blockType = TensorType::make(type->componentType);
  }
  else {
    for (auto &dim : dimensions) {
      const std::vector<IndexSet> &nests = dim.getIndexSets();
      std::vector<IndexSet> blockNests(nests.begin()+1, nests.end());
      blockDimensions.push_back(IndexDomain(blockNests));
    }
    blockType = TensorType::make(type->componentType, blockDimensions);
  }
  assert(blockType.defined());

  return blockType;
}

Type indexExprType(std::vector<IndexVar> lhsIndexVars, Expr expr) {
  assert(isScalarTensor(expr.type()));
  std::vector<IndexDomain> dimensions;
  for (auto &indexVar : lhsIndexVars) {
    dimensions.push_back(indexVar.getDomain());
  }
  return TensorType::make(expr.type().toTensor()->componentType, dimensions);
}

// struct Literal
void Literal::cast(Type type) {
  assert(type.isTensor());
  const TensorType *newType = type.toTensor();
  const TensorType *oldType = this->type.toTensor();
  assert(newType->componentType == oldType->componentType);
  assert(newType->size() == oldType->size());

  this->type = type;
}

bool operator==(const Literal& l, const Literal& r) {
  assert(l.type.isTensor() && r.type.isTensor());

  if (l.type != r.type) {
    return false;
  }

  assert(getTensorByteSize(l.type.toTensor()) ==
         getTensorByteSize(r.type.toTensor()));
  size_t tensorDataSize = getTensorByteSize(l.type.toTensor());

  if (memcmp(l.data, r.data, tensorDataSize) != 0) {
    return false;
  }
  return true;
}

bool operator!=(const Literal& l, const Literal& r) {
  return !(l == r);
}

// struct IndexStmt
class DomainGatherer : private IRVisitor {
public:
  vector<IndexVar> getDomain(const IndexExpr &indexExpr) {
    domain.clear();
    added.clear();
    add(indexExpr.resultVars);
    indexExpr.value.accept(this);
    return domain;
  }

private:
  vector<IndexVar> domain;
  set<string> added;

  void add(const vector<IndexVar> &indexVars) {
    for (const IndexVar &ivar : indexVars) {
      if (added.find(ivar.getName()) == added.end()) {
        added.insert(ivar.getName());
        domain.push_back(ivar);
      }
    }
  }

  void visit(const IndexedTensor *op) {
    add(op->indexVars);
  }
};

std::vector<IndexVar> IndexExpr::domain() const {
  return DomainGatherer().getDomain(*this);
}

}} // namespace simit::internal
