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

// class IRNode
IRNode::~IRNode() {}

// class Literal
static size_t getTensorByteSize(const TensorType *tensorType) {
  return tensorType->size() * tensorType->componentType.toScalar()->bytes();
}

Literal::Literal(const Type &type) : Expression(type) {
  assert(type.isTensor());
  this->dataSize = getTensorByteSize(type.toTensor());
  this->data = malloc(dataSize);
}

Literal::Literal(const Type &type, void *values) : Literal(type) {
  memcpy(this->data, values, this->dataSize);
}

Literal::~Literal() {
  free(data);
}

void Literal::clear() {
  memset(data, 0, dataSize);
}

void Literal::cast(const Type &type) {
  assert(type.isTensor());
  const TensorType *newType = type.toTensor();
  const TensorType *oldType = getType().toTensor();
  assert(newType->componentType == oldType->componentType);
  assert(newType->size() == oldType->size());

  setType(type);
}

bool operator==(const Literal& l, const Literal& r) {
  assert(l.getType().isTensor() && r.getType().isTensor());

  if (l.getType() != r.getType()) {
    return false;
  }

  assert(getTensorByteSize(l.getType().toTensor()) ==
         getTensorByteSize(r.getType().toTensor()));
  size_t tensorDataSize = getTensorByteSize(l.getType().toTensor());

  if (memcmp(l.getConstData(), r.getConstData(), tensorDataSize) != 0) {
    return false;
  }
  return true;
}

bool operator!=(const Literal& l, const Literal& r) {
  return !(l == r);
}


// class IndexedTensor
IndexedTensor::IndexedTensor(const shared_ptr<Expression> &tensor,
                             vector<shared_ptr<IndexVar>> indexVars){
  assert(tensor->getType().isTensor() && "Only tensors can be indexed.");
  const TensorType *ttype = tensor->getType().toTensor();
  assert(indexVars.size() == ttype->order());
  for (size_t i=0; i < indexVars.size(); ++i) {
    assert(indexVars[i]->getDomain() == ttype->dimensions[i]
           && "IndexVar domain does not match tensordimension");
  }

  this->tensor = tensor;
  this->indexVariables = indexVars;
}

std::ostream &operator<<(std::ostream &os, const IndexedTensor &t) {
  os << t.getTensor()->getName();

  if (t.getIndexVariables().size() > 0) {
    os << "(";
    auto it = t.getIndexVariables().begin();
    if (it != t.getIndexVariables().end()) {
      os << (*it)->getName();
      ++it;
    }
    while (it != t.getIndexVariables().end()) {
      os << "," << (*it)->getName();
      ++it;
    }
    os << ")";
  }
  return os;
}


// class IndexExpr
int IndexExpr::numOperands(Operator op) {
  return (op == NONE || op == NEG) ? 1 : 2;
}

IndexExpr::IndexExpr(const std::vector<std::shared_ptr<IndexVar>> &indexVars,
                     Operator op, const std::vector<IndexedTensor> &operands)
    : Expression(Type()), indexVars(indexVars), op(op), operands(operands) {
  initType();

  // Can't have reduction variables on rhs
  for (auto &idxVar : indexVars) {
    assert(idxVar->isFreeVar());
  }

  // Operand typechecks
  assert(operands.size() == (size_t)IndexExpr::numOperands(op));
  assert(operands[0].getTensor()->getType().isTensor() &&
         "Only tensors can be indexed.");
  const TensorType *firstType = operands[0].getTensor()->getType().toTensor();
  Type first = firstType->componentType;
  for (auto &operand : operands) {
    assert(operand.getTensor()->getType().isTensor() &&
           "Only tensors can be indexed.");

    const TensorType *operandType = operand.getTensor()->getType().toTensor();
    assert(first == operandType->componentType &&
           "Operand component types differ");
  }
}

IndexExpr::IndexExpr(IndexExpr::Operator op,
                     const vector<IndexedTensor> &operands)
    : IndexExpr(std::vector<std::shared_ptr<IndexVar>>(), op, operands) {

}

void IndexExpr::setIndexVariables(const vector<shared_ptr<IndexVar>> &ivs) {
  this->indexVars = ivs;
  initType();
}

void IndexExpr::setOperator(IndexExpr::Operator op) {
  this->op = op;
}

void IndexExpr::setOperands(const std::vector<IndexedTensor> &operands) {
  assert(operands.size() > 0);
  const TensorType *newType = operands[0].getTensor()->getType().toTensor();
  const TensorType *oldType = this->operands[0].getTensor()->getType().toTensor();

  bool reinit = (newType->componentType != oldType->componentType);
  this->operands = operands;
  if (reinit) {
    initType();
  }
}

vector<shared_ptr<IndexVar>> IndexExpr::getDomain() const {
  vector<shared_ptr<IndexVar>> domain;
  set<shared_ptr<IndexVar>> added;
  for (auto &iv : indexVars) {
    if (added.find(iv) == added.end()) {
      added.insert(iv);
      domain.push_back(iv);
    }
  }
  for (auto &operand : operands) {
    for (auto &iv : operand.getIndexVariables()) {
      if (added.find(iv) == added.end()) {
        assert(iv->isReductionVar() && "free variable not used on lhs");
        added.insert(iv);
        domain.push_back(iv);
      }
    }
  }
  return domain;

}

void IndexExpr::initType() {
  assert(operands.size() > 0);
  const TensorType *ttype = operands[0].getTensor()->getType().toTensor();
  Type ctype = ttype->componentType;
  std::vector<IndexDomain> dimensions;
  for (auto &iv : indexVars) {
    dimensions.push_back(iv->getDomain());
  }
  setType(TensorType::make(ctype, dimensions));
}


// class FieldRead
static Type getFieldType(const std::shared_ptr<Expression> &expr,
                         const std::string &fieldName){
  assert(expr->getType().isElement() || expr->getType().isSet());

  Type fieldType;
  if (expr->getType().isElement()) {
    const ElementType *elemType = expr->getType().toElement();
    fieldType = elemType->fields.at(fieldName);
  }
  else if (expr->getType().isSet()) {
    const SetType *setType = expr->getType().toSet();
    const ElementType *elemType = setType->elementType.toElement();

    const TensorType *elemFieldType = elemType->fields.at(fieldName).toTensor();

    // The type of a set field is:
    // `Tensor[set][elementFieldDimensions](elemFieldComponentType)`
    std::vector<IndexDomain> dimensions;
    if (elemFieldType->order() == 0) {
      IndexSet setDim(expr->getName());
      dimensions.push_back(IndexDomain(setDim));
    }
    else {
      std::vector<IndexSet> dim;
      dim.push_back(IndexSet(expr->getName()));

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

FieldRead::FieldRead(const std::shared_ptr<Expression> &setOrElem,
                     const std::string &fieldName)
    : Read(getFieldType(setOrElem, fieldName)),
      setOrElem(setOrElem), fieldName(fieldName) {}


// class TensorRead
static Type getBlockType(const std::shared_ptr<Expression> &expr) {
  assert(expr->getType().isTensor());

  const TensorType *type = expr->getType().toTensor();
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

TensorRead::TensorRead(const std::shared_ptr<Expression> &tensor,
                       const std::vector<std::shared_ptr<Expression>> &indices)
    : Read(getBlockType(tensor)), tensor(tensor), indices(indices) {}


// class TupleRead
TupleRead::TupleRead(const std::shared_ptr<Expression> &tuple,
                     const std::shared_ptr<Expression> &index)
    : Read(tuple->getType()), tuple(tuple), index(index) {}


// class Function
void Function::addStatements(const vector<shared_ptr<Expression>> &stmts) {
  body.insert(body.end(), stmts.begin(), stmts.end());
}


// class Test
std::ostream &operator<<(std::ostream &os, const Test &test) {
  std::vector<std::shared_ptr<Expression>> args;
  args.insert(args.end(), test.getActuals().begin(), test.getActuals().end());
  Call call(test.getCallee(), args);
  os << call << " == " << util::join(test.getExpectedResults(), ", ");
  return os;
}

}} // namespace simit::internal
