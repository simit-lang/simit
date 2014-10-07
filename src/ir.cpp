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
static size_t getTensorByteSize(TensorType *tensorType) {
  return tensorType->getSize() * componentSize(tensorType->getComponentType());
}

Literal::Literal(const std::shared_ptr<Type> &type) : Expression(type) {
  assert(type->isTensor());
  this->dataSize = getTensorByteSize(tensorTypePtr(type));
  this->data = malloc(dataSize);
}

Literal::Literal(const std::shared_ptr<Type> &type, void *values) : Literal(type) {
  memcpy(this->data, values, this->dataSize);
}

Literal::~Literal() {
  free(data);
}

void Literal::clear() {
  memset(data, 0, dataSize);
}

void Literal::cast(const std::shared_ptr<TensorType> &type) {
  assert(type->getKind() == Type::Kind::Tensor);
  TensorType *litType = tensorTypePtr(this->getType());
  assert(litType->getComponentType() == type->getComponentType() &&
         litType->getSize() == type->getSize());

  setType(type);
}

bool operator==(const Literal& l, const Literal& r) {
  assert(l.getType()->isTensor() && r.getType()->isTensor());

  if (*l.getType() != *r.getType()) {
    return false;
  }

  assert(getTensorByteSize(tensorTypePtr(l.getType())) ==
         getTensorByteSize(tensorTypePtr(r.getType())));
  size_t tensorDataSize = getTensorByteSize(tensorTypePtr(l.getType()));

  if (memcmp(l.getConstData(), r.getConstData(), tensorDataSize) != 0) {
    return false;
  }
  return true;
}

bool operator!=(const Literal& l, const Literal& r) {
  return !(l == r);
}


// class IndexVar
std::string IndexVar::operatorString(Operator op) {
  switch (op) {
    case IndexVar::FREE:
      return "free";
    case IndexVar::SUM:
      return "sum";
    case IndexVar::PRODUCT:
      return "product";
    default:
      UNREACHABLE;
  }
}

std::string IndexVar::operatorSymbol(Operator op) {
  switch (op) {
    case IndexVar::FREE:
      return "";
    case IndexVar::SUM:
      return "+";
    case IndexVar::PRODUCT:
      return "*";
    default:
      UNREACHABLE;
  }
}

std::ostream &operator<<(std::ostream &os, const IndexVar &var) {
  return os << IndexVar::operatorSymbol(var.getOperator()) << var.getName();
}


// class IndexedTensor
typedef std::vector<std::shared_ptr<IndexVar>> IndexVariables;
IndexedTensor::IndexedTensor(const std::shared_ptr<Expression> &tensor,
                             const IndexVariables &indexVars){
  assert(tensor->getType()->getKind() == Type::Kind::Tensor &&
         "Only tensors can be indexed.");
  TensorType *ttype = tensorTypePtr(tensor->getType());
  assert(indexVars.size() == ttype->getOrder());
  for (size_t i=0; i < indexVars.size(); ++i) {
    assert(indexVars[i]->getDomain() == ttype->getDimensions()[i]
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
    : Expression(NULL), indexVars(indexVars), op(op), operands(operands) {
  initType();

  // Can't have reduction variables on rhs
  for (auto &idxVar : indexVars) {
    assert(idxVar->getOperator() == IndexVar::Operator::FREE);
  }

  // Operand typechecks
  assert(operands.size() == (size_t)IndexExpr::numOperands(op));
  assert(operands[0].getTensor()->getType()->getKind() == Type::Kind::Tensor &&
         "Only tensors can be indexed.");
  TensorType *firstType = tensorTypePtr(operands[0].getTensor()->getType());
  ComponentType first = firstType->getComponentType();
  for (auto &operand : operands) {
    assert(operand.getTensor()->getType()->getKind() == Type::Kind::Tensor &&
           "Only tensors can be indexed.");
  TensorType *ttype = tensorTypePtr(operand.getTensor()->getType());
    assert(first == ttype->getComponentType() &&
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
  TensorType *newType = tensorTypePtr(operands[0].getTensor()->getType());
  TensorType *oldType = tensorTypePtr(this->operands[0].getTensor()->getType());

  bool reinit = (newType->getComponentType() != oldType->getComponentType());
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
        assert(iv->getOperator() != IndexVar::FREE
               && "freevars not used on lhs");
        added.insert(iv);
        domain.push_back(iv);
      }
    }
  }
  return domain;

}

void IndexExpr::initType() {
  assert(operands.size() > 0);
  TensorType *ttype = tensorTypePtr(operands[0].getTensor()->getType());
  ComponentType ctype = ttype->getComponentType();
  std::vector<IndexDomain> dimensions;
  for (auto &iv : indexVars) {
    dimensions.push_back(iv->getDomain());
  }
  setType(std::shared_ptr<TensorType>(new TensorType(ctype, dimensions)));
}


// class FieldRead
static std::shared_ptr<Type>
fieldType(const std::shared_ptr<Expression> &expr,const std::string &fieldName){
  assert(expr->getType()->isElement() || expr->getType()->isSet());

  std::shared_ptr<TensorType> fieldType;
  if (expr->getType()->isElement()) {
    const auto &elemType = static_pointer_cast<ElementType>(expr->getType());
    fieldType = shared_ptr<TensorType>(elemType->getFields().at(fieldName));
  }
  else if (expr->getType()->isSet()) {
    const auto &setType = static_pointer_cast<SetType>(expr->getType());
    auto elemFieldType = setType->getElementType()->getFields().at(fieldName);

    // The type of a set field is:
    // `Tensor[set][elementFieldDimensions](elemFieldComponentType)`
    std::vector<IndexDomain> dimensions;
    if (elemFieldType->getOrder() == 0) {
      IndexSet setDim(expr->getName());
      dimensions.push_back(IndexDomain(setDim));
    }
    else {
      std::vector<IndexSet> dim;
      dim.push_back(IndexSet(expr->getName()));

      for (const IndexDomain &elemFieldDim : elemFieldType->getDimensions()) {
        for (const IndexSet &indexSet : elemFieldDim.getFactors()) {
          dim.push_back(indexSet);
        }
        dimensions.push_back(IndexDomain(dim));
      }
    }
    auto newFieldType = new TensorType(elemFieldType->getComponentType(),
                                       dimensions);
    fieldType = shared_ptr<TensorType>(newFieldType);
  }

  return std::shared_ptr<TensorType>(fieldType);
}

FieldRead::FieldRead(const std::shared_ptr<Expression> &target,
                     const std::string &fieldName)
    : Read(fieldType(target, fieldName)),
      target(target), fieldName(fieldName) {}


// class TensorRead
static std::shared_ptr<Type>
blockType(const std::shared_ptr<Expression> &expr) {
  assert(expr->getType()->isTensor());

  TensorType *type = tensorTypePtr(expr->getType());
  const std::vector<IndexDomain> &dimensions = type->getDimensions();
  assert(dimensions.size() > 0);

  std::vector<IndexDomain> blockDimensions;

  size_t numNests = dimensions[0].getFactors().size();
  for (auto &dim : dimensions) {
    assert(dim.getFactors().size() == numNests &&
           "All dimensions should have the same number of nestings");
  }

  TensorType *blockType = nullptr;
  if (numNests) {
    blockType = new TensorType(type->getComponentType());
  }
  else {
    for (auto &dim : dimensions) {
      const std::vector<IndexSet> &nests = dim.getFactors();
      std::vector<IndexSet> blockNests(nests.begin()+1, nests.end());
      blockDimensions.push_back(IndexDomain(blockNests));
    }
    blockType = new TensorType(type->getComponentType(), blockDimensions);
  }
  assert(blockType);

  return std::shared_ptr<Type>(blockType);
}

TensorRead::TensorRead(const std::shared_ptr<Expression> &tensor,
                       const std::vector<std::shared_ptr<Expression>> &indices)
    : Read(blockType(tensor)), tensor(tensor), indices(indices) {}


// class Function
void Function::addStatements(const std::vector<std::shared_ptr<IRNode>> &stmts){
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
