#include "types.h"

#include <assert.h>
#include <climits>
#include <iostream>

#include "util.h"
#include "macros.h"

using namespace simit::internal;
using namespace std;

/* class IndexSet */
int IndexSet::getSize() const {
  int size = 0;
  switch (type) {
    case RANGE:
      size = rangeSize;
      break;
    case SET:
      NOT_SUPPORTED_YET;
      break;
    case VARIABLE:
      NOT_SUPPORTED_YET;
      break;
    default:
      assert(false);
  }
  return size;
}

std::ostream &IndexSet::print(std::ostream &os) const {
  switch (type) {
    case RANGE:
      os << "[0-" << to_string(rangeSize-1) << "]";
      break;
    case SET:
      NOT_SUPPORTED_YET;
      break;
    case VARIABLE:
      os << "*";
      break;
    default:
      assert(false);
      break;
  }
  return os;
}

bool simit::internal::operator==(const IndexSet &left, const IndexSet &right) {
   if (left.type != right.type) {
    return false;
  }

  switch (left.type) {
    case IndexSet::RANGE:
      if (left.rangeSize != right.rangeSize) {
        return false;
      }
      break;
    case IndexSet::SET:
      NOT_SUPPORTED_YET;
      break;
    case IndexSet::VARIABLE:
      NOT_SUPPORTED_YET;
      break;
    default:
      assert(false);
      break;
  }

  return true;
}


/* class IndexSetProduct */
int IndexSetProduct::getSize() const {
  int size = 1;
  for (auto &indexSet : getIndexSets()) {
    size *= indexSet.getSize();
  }
  return size;
}

std::ostream &IndexSetProduct::print(std::ostream &os) const {
  return os << util::join(getIndexSets(), " x ");
}

bool simit::internal::operator==(const IndexSetProduct &left,
                                 const IndexSetProduct &right) {
  if (left.getIndexSets().size() != right.getIndexSets().size()) {
    return false;
  }
  auto leftIter = left.getIndexSets().begin();
  auto rightIter = right.getIndexSets().begin();
  auto leftEnd = left.getIndexSets().end();
  for (; leftIter != leftEnd; ++leftIter, ++rightIter) {
    if (*leftIter != *rightIter) {
      return false;
    }
  }
  return true;
}


/* TensorType */
std::size_t TensorType::componentSize(ComponentType ct) {
  switch (ct) {
    case TensorType::INT:
      return sizeof(int);
    case TensorType::FLOAT:
      return sizeof(double);
    case TensorType::ELEMENT:
      assert(false && "currently unsupported");  // TODO
      return INT_MAX;
  }
  assert(false);
  return 0;
}

std::string TensorType::componentTypeString(ComponentType ct) {
  switch (ct) {
    case TensorType::INT:
      return "int";
    case TensorType::FLOAT:
      return "float";
    case TensorType::ELEMENT:
      return "element";
  }
  assert(false);
  return "";
}

TensorType::~TensorType() {}

std::ostream &simit::internal::operator<<(std::ostream &os,
                                          const TensorType &type) {
  return os << type.toString();
}


/* ScalarType */
ScalarType::~ScalarType() {}

unsigned int ScalarType::getSize() const {
  return 1;
}

TensorType::ComponentType ScalarType::getComponentType() const {
  return componentType;
}

bool ScalarType::operator==(const TensorType& other) {
  const ScalarType *otherPtr = dynamic_cast<const ScalarType *>(&other);
  return otherPtr != NULL && componentType == otherPtr->componentType;
}

std::string ScalarType::toString() const {
  return componentTypeString(componentType);
}


/* Dimension */
Dimension::Dimension(const Dimension& other) {
  type = other.type;
  switch (type) {
    case ANONYMOUS:
      size = other.size;
      break;
    case VARIABLE:
      assert(false);  // TODO: Not supported yet
      break;
    default:
      assert(false);
      break;
  };
}

unsigned int Dimension::getSize() const {
  switch (type) {
    case ANONYMOUS:
      return size;
    case VARIABLE:
      assert(false);
      return UINT_MAX; // TODO: Fix this by storing var sizes in types
  }
}

Dimension::operator std::string() const {
  switch (type) {
    case ANONYMOUS:
      return to_string(size);
    case VARIABLE:
      return "*";
  }
  assert(false);
  return "";
}

bool Dimension::operator==(const Dimension &other) const {
  if (type != other.type) {
    return false;
  }
  switch (type) {
    case ANONYMOUS:
      if (size != other.size) {
        return false;
      }
      break;
    case VARIABLE:
      assert(false);  // TODO: Not supported yet
      break;
    default:
      assert(false);
      break;
  }

  return true;
}


/* Shape */
Shape::~Shape() {
  for (auto dim : dimensions) {
    delete dim;
  }
}

unsigned int Shape::getSize() const {
  unsigned int shapeSize = 1;
  for (Dimension *dim : dimensions) {
    shapeSize *= dim->getSize();
  }
  return shapeSize;
}

bool Shape::operator==(const Shape &other) const {
  if (getOrder() != other.getOrder()) {
    return false;
  }
  for (unsigned int i=0; i<getOrder(); ++i) {
    if (*dimensions[i] != *other.dimensions[i]) {
      return false;
    }
  }
  return true;
}

Shape::operator std::string() const {
  return "[" + util::join(dimensions, ",") + "]";
}


/* NDTensorType */
NDTensorType::~NDTensorType() {
  delete blockShape;
  delete blockType;
}

unsigned int NDTensorType::getSize() const {
  return blockShape->getSize() * blockType->getSize();
}

TensorType::ComponentType NDTensorType::getComponentType() const {
  return blockType->getComponentType();
}

bool NDTensorType::operator==(const TensorType& other) {
  const NDTensorType *otherPtr = dynamic_cast<const NDTensorType *>(&other);
  return (otherPtr != NULL &&
          *blockShape == *otherPtr->blockShape &&
          *blockType == *otherPtr->blockType);
}

std::string NDTensorType::toString() const {
  string blockTypeStr = (dynamic_cast<ScalarType*>(blockType) != NULL)
                      ? "(" + blockType->toString() + ")"
                      : blockType->toString().substr(string().size());
  return string("Tensor") + string(*blockShape) + blockTypeStr;
}
