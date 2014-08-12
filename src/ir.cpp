#include "ir.h"

#include <cstdlib>
#include <string>
#include <iostream>
#include <string.h>

#include "types.h"
#include "util.h"

using namespace std;

namespace simit {
namespace internal {

/* class IRNode */
IRNode::~IRNode() {}

std::ostream &operator<<(std::ostream &os, const IRNode &node){
  node.print(os);
  return os;
}


/* class TensorNode */
TensorNode::~TensorNode() {
  delete type;
}

void TensorNode::print(std::ostream &os) const {
  os << getName() << " : " << *type;
}


/* class LiteralTensor */
LiteralTensor::LiteralTensor(TensorType *type, void *data)
    : TensorNode(type) {
  auto componentSize = TensorType::componentSize(type->getComponentType());
  auto dataSize = type->getSize() * componentSize;
  this->data = malloc(dataSize);
  memcpy(this->data, data, dataSize);
}

LiteralTensor::~LiteralTensor() {
  free(data);
}

void LiteralTensor::cast(TensorType *type) {
  assert(this->type->getComponentType() == type->getComponentType() &&
         this->type->getSize() == type->getSize());
  delete this->type;
  this->type = type;
}

void LiteralTensor::print(std::ostream &os) const {
  // TODO: Add nicer value printing that prints matrices and tensors properly
  switch (type->getComponentType()) {
    case Type::INT: {
      int *idata = (int*)data;
      if (type->getSize() == 1) {
        os << idata[0];
      }
      else {
        os << "[" << idata[0];
        for (int i=0; i<type->getSize(); ++i) {
          os << ", " << idata[i];
        }
        os << "]";
      }
      break;
    }
    case Type::FLOAT: {
      double *fdata = (double*)data;
      if (type->getSize() == 1) {
        os << fdata[0];
      }
      else {
        os << "[" << fdata[0];
        for (int i=0; i<type->getSize(); ++i) {
          os << ", " + to_string(fdata[i]);
        }
        os << "]";
      }
      break;
    }
    case Type::ELEMENT:
      assert(false && "Unsupported (TODO)");
      break;
  }
}


/* class IndexVar */
std::ostream &operator<<(std::ostream &os, const IndexVar &var) {
  switch (var.getOperator()) {
    case IndexVar::FREE:
      break;
    case IndexVar::SUM:
      os << "+";
      break;
    case IndexVar::PRODUCT:
      os << "*";
      break;
    default:
      assert(false);
      break;
  }
  return os << var.getName();
}


/* class IndexVarFactory */
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


/* class IndexExpr */
IndexExpr::IndexedTensor::IndexedTensor(
    const std::shared_ptr<TensorNode> &t,
    const std::vector<IndexExpr::IndexVarPtr> &ivs) {
  assert(ivs.size() == t->getOrder());
  auto titer = t->getType()->getDimensions().begin();
  auto iviter = ivs.begin();
  for (; iviter != ivs.end(); ++iviter, ++titer) {
    assert((*iviter)->getIndexSet() == (*titer));
  }
  this->tensor = t;
  this->indexVariables = ivs;
}

static TensorType *
computeIndexExprType(const std::vector<IndexExpr::IndexVarPtr> &indexVars,
                     const std::vector<IndexExpr::IndexedTensor> &operands) {
  std::vector<IndexSetProduct> dimensions;
  for (auto &iv : indexVars) {
    dimensions.push_back(iv->getIndexSet());
  }
  Type ctype = operands[0].getTensor()->getType()->getComponentType();
  return new TensorType(ctype, dimensions);
}

IndexExpr::IndexExpr(const std::vector<IndexVarPtr> &indexVars,
                     Operator op, const std::vector<IndexedTensor> &operands)
    : TensorNode(computeIndexExprType(indexVars, operands)),
      indexVars{indexVars}, op{op}, operands{operands} {
  unsigned int expectedNumOperands = (op == NEG) ? 1 : 2;
  assert(expectedNumOperands == operands.size());
  Type firstType = operands[0].getTensor()->getType()->getComponentType();
  for (auto &operand : operands) {
    Type componentType = operand.getTensor()->getType()->getComponentType();
    assert(firstType == componentType && "All operands must have same ctype");
  }
}

const std::vector<IndexExpr::IndexVarPtr> &IndexExpr::getDomain() const {
  return indexVars;
}

static std::string opString(IndexExpr::Operator op) {
  switch (op) {
    case IndexExpr::NEG:
      return "-";
    case IndexExpr::ADD:
      return "+";
    case IndexExpr::SUB:
      return "-";
    case IndexExpr::MUL:
      return "*";
    case IndexExpr::DIV:
      return "//";
  }
  assert(false);
  return "";
}

static inline
std::string indexVarString(const std::vector<IndexExpr::IndexVarPtr> &idxVars) {
  return (idxVars.size()!=0) ? "(" + simit::util::join(idxVars,",") + ")" : "";
}

static inline
std::string indexedTensorString(const IndexExpr::IndexedTensor &it) {
  return it.getTensor()->getName() + indexVarString(it.getIndexVariables());
}

void IndexExpr::print(std::ostream &os) const {
  os << getName() << indexVarString(indexVars) << " = ";

  unsigned int numOperands = operands.size();
  auto opit = operands.begin();
  if (numOperands == 1) {
    os << opString(op) + indexedTensorString(*opit++);
  }
  else if (numOperands == 2) {
    os << indexedTensorString(*opit++) << opString(op) <<
          indexedTensorString(*opit++);
  } else {
    assert(false && "Not supported yet");
  }
}


/* class VariableStore */
void VariableStore::print(std::ostream &os) const {
  os << getName();
}


/* class Function */
void Function::addStatements(const std::vector<std::shared_ptr<IRNode>> &stmts){
  body.insert(body.end(), stmts.begin(), stmts.end());
}

void Function::print(std::ostream &os) const {
  string argumentString = "(" + util::join(this->arguments, ", ") + ")";
  string resultString = (results.size() == 0)
                        ? ""
                        : " -> (" + util::join(this->results, ", ") + ")";
  os << "func " << name << argumentString << resultString;

  string bodyString = (body.size() > 0) ? "  " + util::join(body, "  \n") + "\n"
                                        : "";
  os << endl << bodyString;

  os << "end";
}


/* class Argument */
void Argument::print(std::ostream &os) const {
  TensorNode::print(os);
}

/* class Result */
void Result::print(std::ostream &os) const {
  TensorNode::print(os);
}

/* class Test */
void Test::print(std::ostream &os) const {
  os << "Test";
}

}} // namespace simit::internal
