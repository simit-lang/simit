#ifndef SIMIT_TENSOR_COMPONENTS_H
#define SIMIT_TENSOR_COMPONENTS_H

#include <cassert>
#include <climits>
#include <string>

namespace simit {

/** The types of supported tensor components. */
enum ComponentType {INT, FLOAT, ELEMENT};


/** Helper to convert from C++ type to Simit Type. */
template<typename T> inline ComponentType typeOf        () { return ComponentType::ELEMENT; }
template<>           inline ComponentType typeOf<double>() { return ComponentType::FLOAT; }
template<>           inline ComponentType typeOf<int>   () { return ComponentType::INT; }


inline std::size_t componentSize(ComponentType ct) {
  switch (ct) {
    case ComponentType::INT:
      return sizeof(int);
    case ComponentType::FLOAT:
      return sizeof(double);
    case ComponentType::ELEMENT:
      assert(false && "currently unsupported");  // TODO
      return INT_MAX;
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
      case ComponentType::ELEMENT:
        return "element";
    }
    assert(false);
    return "";
  }

} // namespace simit

#endif
