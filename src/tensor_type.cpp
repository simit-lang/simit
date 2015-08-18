#include "tensor_type.h"

namespace simit {

// enum class ComponentType
std::ostream& operator<<(std::ostream& os, ComponentType componentType) {
  switch (componentType) {
    case ComponentType::Float:
      os << "float";
      break;
    case ComponentType::Int:
      os << "int";
      break;
    case ComponentType::Boolean:
      os << "bool";
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

}
