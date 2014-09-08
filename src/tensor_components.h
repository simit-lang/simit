#ifndef SIMIT_TENSOR_COMPONENTS_H
#define SIMIT_TENSOR_COMPONENTS_H

#include <cassert>
#include <climits>
#include <string>

namespace simit {

/** The types of supported tensor components. */
enum Type {INT, FLOAT, ELEMENT};


/** Helper to convert from C++ type to Simit Type. */
template<typename T> inline Type typeOf        () { return Type::ELEMENT; }
template<>           inline Type typeOf<double>() { return Type::FLOAT; }
template<>           inline Type typeOf<int>   () { return Type::INT; }


inline std::size_t componentSize(Type ct) {
  switch (ct) {
    case Type::INT:
      return sizeof(int);
    case Type::FLOAT:
      return sizeof(double);
    case Type::ELEMENT:
      assert(false && "currently unsupported");  // TODO
      return INT_MAX;
  }
  assert(false);
  return 0;
}

inline std::string componentTypeString(Type ct) {
    switch (ct) {
      case Type::INT:
        return "int";
      case Type::FLOAT:
        return "float";
      case Type::ELEMENT:
        return "element";
    }
    assert(false);
    return "";
  }

} // namespace simit

#endif
