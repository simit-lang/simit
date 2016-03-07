#include "tensor_type.h"

namespace simit {

// enum class ComponentType
std::ostream& operator<<(std::ostream& os, ComponentType componentType) {
  switch (componentType) {
    case ComponentType::Float:
      os << "float";
      break;
    case ComponentType::Double:
      os << "double";
      break;
    case ComponentType::Int:
      os << "int";
      break;
    case ComponentType::Boolean:
      os << "bool";
      break;
    case ComponentType::FloatComplex:
      os << "float_complex";
      break;
    case ComponentType::DoubleComplex:
      os << "double_complex";
      break;
  }
  return os;
}


// class TensorType
TensorType TensorType::getBlockType() const {
  iassert(isBlocked());
  return *blockType;
}

ComponentType TensorType::getComponentType() const {
  // TODO: If it is blocked, then recurse to the component type
  iassert(!isBlocked());
  return componentType;
}

size_t TensorType::getSize() const {
  size_t size = (isBlocked()) ? blockType->getSize() : 1;
  for (int dimension : dimensions) {
    size *= dimension;
  }
  return size;
}

std::ostream& operator<<(std::ostream& os, const TensorType& tensorType) {
  size_t order = tensorType.getOrder();
  os << "tensor";
  if (order > 0) {
    os << "[";
    os << tensorType.getDimension(0);
    for (size_t i=1; i < tensorType.getOrder(); ++i) {
      os << "," << tensorType.getDimension(i);
    }
    os << "]";
  }
  os << "(";
  if (tensorType.isBlocked()) {
    os << tensorType.getBlockType();
  }
  // Base case
  else {
    os << tensorType.getComponentType();
  }
  os << ")";
  return os;
}

bool operator==(const TensorType& l, const TensorType& r) {
  return true; // TODO: Fix
}

bool operator!=(const TensorType& l, const TensorType& r) {
  return !(l == r);
}

}
