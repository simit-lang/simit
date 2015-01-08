#ifndef SIMIT_TENSOR_COMPONENTS_H
#define SIMIT_TENSOR_COMPONENTS_H

#include <climits>
#include <string>

#include "error.h"
#include "types.h"

namespace simit {

/** The types of supported tensor components. */
enum ComponentType {INT, FLOAT};

/** Helper to convert from C++ type to Simit Type. */
template<typename T> inline ComponentType typeOf() {
  iassert(false) << "Unsupported type";
  return ComponentType::INT; // TODO XXX gcc warning suppression
}

template<> inline ComponentType typeOf<int>   () {return ComponentType::INT;}
template<> inline ComponentType typeOf<float>() {
  iassert(ir::ScalarType::floatBytes == 4);
  return ComponentType::FLOAT;
}
template<> inline ComponentType typeOf<double>() {
  iassert(ir::ScalarType::floatBytes == 8);
  return ComponentType::FLOAT;
}


inline bool isValidComponentType(ComponentType componentType) {
  return (ComponentType::INT || ComponentType::FLOAT);
}

inline std::size_t componentSize(ComponentType ct) {
  switch (ct) {
    case ComponentType::INT:
      return sizeof(int);
    case ComponentType::FLOAT:
      return ir::ScalarType::floatBytes;
  }
  unreachable;
  return 0;
}

inline std::string componentTypeString(ComponentType ct) {
  switch (ct) {
    case ComponentType::INT:
      return "int";
    case ComponentType::FLOAT:
      return "float";
  }
  unreachable;
  return "";
}

} // namespace simit

#endif
