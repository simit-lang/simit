#include "types.h"

#include <ostream>
#include <iostream>  // TODO: remove

#include "macros.h"
#include "util.h"

namespace simit {
namespace ir {

// class TensorType
size_t TensorType::size() const {
  int size = 1;
  for (auto &dimension : dimensions) {
    size *= dimension.getSize();
  }
  return size;
}

bool operator==(const Type& l, const Type& r) {
  if (l.kind() != r.kind()) {
    return false;
  }

  switch (l.kind()) {
    case Type::Tensor:
      return *l.toTensor() == *r.toTensor();
    case Type::Element:
      return *l.toElement() == *r.toElement();
    case Type::Set:
      return *l.toSet() == *r.toSet();
    case Type::Tuple:
      return *l.toTuple() == *r.toTuple();
  }
}

bool operator!=(const Type& l, const Type& r) {
  return !(l == r);
}

bool operator==(const ScalarType &l, const ScalarType &r) {
  return l.kind == r.kind;
}

bool operator==(const TensorType &l, const TensorType &r) {
  if (l.componentType != r.componentType) {
    return false;
  }
  if (l.order() != r.order()) {
    return false;
  }

  auto li = l.dimensions.begin();
  auto ri = r.dimensions.begin();
  for (; li != l.dimensions.end(); ++li, ++ri) {
    if (*li != *ri) {
      return false;
    }
  }

  return true;
}

bool operator==(const ElementType &l, const ElementType &r) {
  // Element type names are unique
  return (l.name == r.name);
}

bool operator==(const SetType &l, const SetType &r) {
  return l.elementType == r.elementType;
}

bool operator==(const TupleType &l, const TupleType &r) {
  return l.elementType == r.elementType && l.size == r.size;
}

bool operator!=(const ScalarType &l, const ScalarType &r) {
  return !(l == r);
}

bool operator!=(const TensorType &l, const TensorType &r) {
  return !(l == r);
}

bool operator!=(const ElementType &l, const ElementType &r) {
  return !(l == r);
}

bool operator!=(const SetType &l, const SetType &r) {
  return !(l == r);
}

bool operator!=(const TupleType &l, const TupleType &r) {
  return !(l == r);
}

std::ostream &operator<<(std::ostream &os, const Type &type) {
  switch (type.kind()) {
    case Type::Tensor:
      return os << *type.toTensor();
    case Type::Element:
      return os << *type.toElement();
    case Type::Set:
      return os << *type.toSet();
    case Type::Tuple:
      return os << *type.toTuple();
  }
}

std::ostream &operator<<(std::ostream &os, const ScalarType &type) {
  switch (type.kind) {
    case ScalarType::Int:
      os << "int";
      break;
    case ScalarType::Float:
      os << "float";
      break;
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const TensorType &type) {
  if (type.order() == 0) {
    os << type.componentType;
  }
  else {
    os << "tensor";
    os << "[" << util::join(type.dimensions, "][") << "]";
    os << "(" << type.componentType << ")";
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const ElementType &type) {
//  os << "struct " << type.name;
//  if (type.fields.size() > 0) {
//    os << std::endl << "\n  ";
//  }
//  for (auto &field : type.fields) {
//    os << field.first << " : " << field.second << ";" << std::endl;
//  }
//  return os << "end";
  os << type.name;
  return os;
}

std::ostream &operator<<(std::ostream &os, const SetType &type) {
  return os << type.elementType.toElement()->name << "{}";
}

std::ostream &operator<<(std::ostream &os, const TupleType &type) {
  return os << "(" << type.elementType.toElement()->name << "*" << type.size
            << ")";
}


}} // namespace simit::ir
