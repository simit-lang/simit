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


/* FreeIndexVariable */
std::list<std::shared_ptr<IndexVariable>> simit::makeFreeIndexVariables(int n) {
  auto freeIndexVars = std::list<std::shared_ptr<IndexVariable>>();
  for (int i=0; i<n; ++i) {
    char name[2];
    name[0] = 'i' + i;
    name[1] = '\0';
    auto freeIndexVar = new FreeIndexVariable(name);
    freeIndexVars.push_back(std::shared_ptr<IndexVariable>(freeIndexVar));
  }
  return freeIndexVars;
}


/* ReductionIndexVariable */
static std::string opString(ReductionIndexVariable::Operator op) {
  switch (op) {
    case ReductionIndexVariable::ADD:
      return "+";
    case ReductionIndexVariable::MUL:
      return "*";
  }
  assert(false);
  return "";
}

std::string ReductionIndexVariable::toString() const {
  return opString(op) + getName();
}


/* Merge */
Merge *Merge::make(Operator op,
                   const std::list<IndexVariablePtr> &indexVariables,
                   const std::list<IndexedTensor> &operands) {
  unsigned int expectedNumOperands = (op == NEG) ? 1 : 2;
  assert(expectedNumOperands == operands.size());
  if (expectedNumOperands != operands.size()) {
    return NULL;
  }
  return new Merge(op, indexVariables, operands);
}

TensorType *Merge::getTensorType() {
  return NULL;
}

static std::string opString(Merge::Operator op) {
  switch (op) {
    case Merge::NEG:
      return "-";
    case Merge::ADD:
      return "+";
    case Merge::SUB:
      return "-";
    case Merge::MUL:
      return "*";
    case Merge::DIV:
      return "//";
  }
  assert(false);
  return "";
}

static inline
std::string indexVarString(const std::list<Merge::IndexVariablePtr> &idxVars) {
  return (idxVars.size() != 0) ? "(" + util::join(idxVars, ",") + ")" : "";
}

std::string Merge::IndexedTensor::toString() const {
  return tensor->getName() + indexVarString(indexVariables);
}

std::string Merge::toString() const {
  unsigned int numOperands = operands.size();
  auto iter = operands.begin();
  std::string rhsString;
  if (numOperands == 1) {
    rhsString = opString(op) + util::toString(*iter++);
  }
  else if (numOperands) {
    rhsString = util::toString(*iter++) + opString(op) + util::toString(*iter++);
  } else {
    assert(false);  // Not supported yet
    return "";
  }

  return getName() + indexVarString(indexVariables) + "=" + rhsString;
}


/* VariableStore */
std::string VariableStore::toString() const {
  return getName();
}
