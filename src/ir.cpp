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

static size_t getTensorByteSize(const TensorType *tensorType) {
  return tensorType->size() * tensorType->componentType.toScalar()->bytes();
}

// class Expr
bool operator==(const Expr &l, const Expr &r) {
  return l.expr() == r.expr();
}

bool operator!=(const Expr &l, const Expr &r) {
  return !(l == r);
}

// Type compute functions
Type fieldType(Expr setOrElement, std::string fieldName) {
  assert(setOrElement.type().isElement() || setOrElement.type().isSet());

  Type fieldType;
  if (setOrElement.type().isElement()) {
    const ElementType *elemType = setOrElement.type().toElement();
    fieldType = elemType->fields.at(fieldName);
  }
  else if (setOrElement.type().isSet()) {
    const SetType *setType = setOrElement.type().toSet();
    const ElementType *elemType = setType->elementType.toElement();

    const TensorType *elemFieldType = elemType->fields.at(fieldName).toTensor();

    // The type of a set field is:
    // `Tensor[set][elementFieldDimensions](elemFieldComponentType)`
    std::vector<IndexDomain> dimensions;
    if (elemFieldType->order() == 0) {
      dimensions.push_back(IndexDomain(IndexSet(setOrElement)));
    }
    else {
      std::vector<IndexSet> dim;
      dim.push_back(IndexSet(setOrElement));

      for (const IndexDomain &elemFieldDim : elemFieldType->dimensions) {
        for (const IndexSet &indexSet : elemFieldDim.getFactors()) {
          dim.push_back(indexSet);
        }
        dimensions.push_back(IndexDomain(dim));
      }
    }
    fieldType = TensorType::make(elemFieldType->componentType, dimensions);
  }

  return fieldType;
}

Type blockType(Expr tensor) {
  assert(tensor.type().isTensor());

  const TensorType *type = tensor.type().toTensor();
  const std::vector<IndexDomain> &dimensions = type->dimensions;
  assert(dimensions.size() > 0);

  std::vector<IndexDomain> blockDimensions;

  size_t numNests = dimensions[0].getFactors().size();
  for (auto &dim : dimensions) {
    assert(dim.getFactors().size() == numNests &&
           "All dimensions should have the same number of nestings");
  }

  Type blockType;
  if (numNests) {
    blockType = TensorType::make(type->componentType);
  }
  else {
    for (auto &dim : dimensions) {
      const std::vector<IndexSet> &nests = dim.getFactors();
      std::vector<IndexSet> blockNests(nests.begin()+1, nests.end());
      blockDimensions.push_back(IndexDomain(blockNests));
    }
    blockType = TensorType::make(type->componentType, blockDimensions);
  }
  assert(blockType.defined());

  return blockType;
}

Type indexExprType(std::vector<IndexVar> lhsIndexVars, Expr expr) {
  assert(expr.type().isScalar());
  std::vector<IndexDomain> dimensions;
  for (auto &indexVar : lhsIndexVars) {
    dimensions.push_back(indexVar.getDomain());
  }
  return TensorType::make(expr.type(), dimensions);
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

// struct IndexExpr
std::vector<IndexVar> IndexExpr::domain() {
  std::vector<IndexVar> domain;
  std::set<std::string> added;
  for (auto &iv : lhsIndexVars) {
    if (added.find(iv.getName()) == added.end()) {
      added.insert(iv.getName());
      domain.push_back(iv);
    }
  }

  // TODO: Visitor that adds reduction variables
  return domain;
}

}} // namespace simit::internal
