#include "IR.h"

#include <cstdlib>
#include <string>
#include <iostream>
#include <string.h>
#include "Types.h"
#include "Util.h"

using namespace simit;
using namespace std;

/* LiteralTensor */
void LiteralTensor::cast(TensorType *type) {
  assert(this->type->getComponentType() == type->getComponentType() &&
         this->type->getSize() == type->getSize());
  delete this->type;
  this->type = type;
}


/* DenseTensorLiteral */
DenseLiteralTensor::DenseLiteralTensor(TensorType *type, void *data)
: LiteralTensor(type) {
  auto componentSize = TensorType::componentSize(type->getComponentType());
  auto dataSize = type->getSize() * componentSize;
  this->data = malloc(dataSize);
  memcpy(this->data, data, dataSize);
}

DenseLiteralTensor::~DenseLiteralTensor() {
  free(data);
}

std::string DenseLiteralTensor::toString() const {
  string result;

  // TODO: Add nicer value printing that prints matrices and tensors properly
  switch (type->getComponentType()) {
    case TensorType::INT: {
      int *idata = (int*)data;
      if (type->getSize() == 1) {
        result += to_string(idata[0]);
      }
      else {
        result += "[";
        result += to_string(idata[0]);
        for (unsigned int i=0; i<type->getSize(); ++i) {
          result += ", " + to_string(idata[i]);
        }
        result += "]";
      }
      break;
    }
    case TensorType::FLOAT: {
      double *fdata = (double*)data;
      if (type->getSize() == 1) {
        result += to_string(fdata[0]);
      }
      else {
        result += "[";
        result += to_string(fdata[0]);
        for (unsigned int i=0; i<type->getSize(); ++i) {
          result += ", " + to_string(fdata[i]);
        }
        result += "]";
      }
      break;
    }
  }
  return result;
}


/* ReductionIndexVariable */
std::string ReductionIndexVariable::opString() const {
  switch (op) {
    case ADD:
      return "+";
    case MUL:
      return "*";
  default:
    assert(false);
    break;
  }
}


/* Merge */
Merge *Merge::make(Operator op, const list<shared_ptr<Tensor>> &operands) {
  unsigned int expectedNumOperands = (op == NEG) ? 1 : 2;
  if (expectedNumOperands != operands.size()) {
    return NULL;
  }
  return new Merge(op, operands);
}

TensorType *Merge::getTensorType() {
  return NULL;
}

std::string Merge::toString() const {
  unsigned int numOperands = operands.size();
  auto iter = operands.begin();
  if (numOperands == 1) {
    return opString() + (*iter++)->getName();
  }
  else if (numOperands) {
    return (*iter++)->getName() + opString() + (*iter++)->getName();
  } else {
    assert(false);  // Not supported yet
    return "";
  }
}

std::string Merge::opString() const {
  switch (op) {
    case NEG:
      return "-";
    case ADD:
      return "+";
    case SUB:
      return "-";
    case MUL:
      return "*";
    case DIV:
      return "//";
  default:
    assert(false);
    break;
  }
}


/* VariableStore */
std::string VariableStore::toString() const {
  return getName();
}
