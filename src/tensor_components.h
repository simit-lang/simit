#ifndef SIMIT_TENSOR_COMPONENTS_H
#define SIMIT_TENSOR_COMPONENTS_H

#include <climits>
#include <string>

#include "error.h"
#include "types.h"

namespace simit {

/** The types of supported tensor components. */
enum class ComponentType {Float, Int, Boolean };

/** Helper to convert from C++ type to Simit Type. */
template<typename T> inline ComponentType typeOf() {
  ierror << "Unsupported type";
  return ComponentType::Int; // TODO XXX gcc warning suppression
}

template<> inline ComponentType typeOf<int>() {
  return ComponentType::Int;
}

template<> inline ComponentType typeOf<float>() {
  iassert(ir::ScalarType::floatBytes == 4);
  return ComponentType::Float;
}

template<> inline ComponentType typeOf<double>() {
  iassert(ir::ScalarType::floatBytes == 8);
  return ComponentType::Float;
}

template<> inline ComponentType typeOf<bool>() {
  return ComponentType::Boolean;
}

inline std::size_t componentSize(ComponentType ct) {
  switch (ct) {
    case ComponentType::Int:
      return sizeof(int);
    case ComponentType::Float:
      return ir::ScalarType::floatBytes;
    case ComponentType::Boolean:
      return sizeof(bool);
  }
  unreachable;
  return 0;
}

inline std::ostream& operator<<(std::ostream& os, ComponentType componentType) {
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

} // namespace simit

#endif
