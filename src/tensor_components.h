#ifndef SIMIT_TENSOR_COMPONENTS_H
#define SIMIT_TENSOR_COMPONENTS_H

#include <cassert>
#include <climits>
#include <string>

namespace simit {

/** The types of supported tensor components. */
enum ComponentType {INT, FLOAT};

/** Helper to convert from C++ type to Simit Type. */
template<typename T> inline ComponentType typeOf() {
  assert(false && "Unsupported type");
}

template<> inline ComponentType typeOf<int>   () {return ComponentType::INT;  }
template<> inline ComponentType typeOf<double>() {return ComponentType::FLOAT;}


inline bool isValidComponentType(ComponentType componentType) {
  return (ComponentType::INT || ComponentType::FLOAT);
}

inline std::size_t componentSize(ComponentType ct) {
  switch (ct) {
    case ComponentType::INT:
      return sizeof(int);
    case ComponentType::FLOAT:
      return sizeof(double);
  }
  assert(false);
  return 0;
}

inline std::string componentTypeString(ComponentType ct) {
  switch (ct) {
    case ComponentType::INT:
      return "int";
    case ComponentType::FLOAT:
      return "float";
  }
  assert(false);
  return "";
}

} // namespace simit

#endif
