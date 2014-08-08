#include "ir.h"

#include <cstdlib>
#include <string>
#include <iostream>
#include <string.h>

#include "types.h"
#include "indexvariables.h"
#include "util.h"

using namespace simit::internal;
using namespace std;


/* class IRNode */
std::ostream &simit::internal::operator<<(std::ostream &os, const IRNode &node) {
  node.print(os);
  return os;
}


/* class TensorNode */
void TensorNode::print(std::ostream &os) const {
  os << getName() << " : " << type->toString();
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
    case TensorType::INT: {
      int *idata = (int*)data;
      if (type->getSize() == 1) {
        os << idata[0];
      }
      else {
        os << "[" << idata[0];
        for (unsigned int i=0; i<type->getSize(); ++i) {
          os << ", " << idata[i];
        }
        os << "]";
      }
      break;
    }
    case TensorType::FLOAT: {
      double *fdata = (double*)data;
      if (type->getSize() == 1) {
        os << fdata[0];
      }
      else {
        os << "[" << fdata[0];
        for (unsigned int i=0; i<type->getSize(); ++i) {
          os << ", " + to_string(fdata[i]);
        }
        os << "]";
      }
      break;
    }
    case TensorType::ELEMENT:
      assert(false && "Unsupported (TODO)");
      break;
  }
}


/* class IndexExpr */
IndexExpr *IndexExpr::make(Operator op,
                           const std::list<IndexVariablePtr> &indexVariables,
                           const std::list<IndexedTensor> &operands) {
  unsigned int expectedNumOperands = (op == NEG) ? 1 : 2;
  assert(expectedNumOperands == operands.size());
  if (expectedNumOperands != operands.size()) {
    return NULL;
  }
  return new IndexExpr(op, indexVariables, operands);
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
std::string indexVarString(const std::list<IndexExpr::IndexVariablePtr> &idxVars) {
  return (idxVars.size()!=0) ? "(" + simit::util::join(idxVars,",") + ")" : "";
}

std::string IndexExpr::IndexedTensor::toString() const {
  return tensor->getName() + indexVarString(indexVariables);
}

void IndexExpr::print(std::ostream &os) const {
  os << getName() << indexVarString(indexVariables) << " = ";

  unsigned int numOperands = operands.size();
  auto iter = operands.begin();
  if (numOperands == 1) {
    os << opString(op) + util::toString(*iter++);
  }
  else if (numOperands) {
    os << util::toString(*iter++) + opString(op) + util::toString(*iter++);
  } else {
    assert(false && "Not supported yet");
  }
}


/* class VariableStore */
void VariableStore::print(std::ostream &os) const {
  os << getName();
}


/* class Function */
void Function::addStatements(const std::list<std::shared_ptr<IRNode>> &stmts) {
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
