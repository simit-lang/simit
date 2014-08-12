#include "tensor.h"

namespace simit {
template<>
  Type type_of<int>() { return Type::INT; }

template<>
  Type type_of<double>() { return Type::FLOAT; }
}