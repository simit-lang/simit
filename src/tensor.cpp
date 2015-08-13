#include "tensor.h"

#include <ostream>
#include <string>

using namespace std;

namespace simit {

// class Tensor
struct Content {
};

std::ostream &operator<<(std::ostream &os, const Tensor &tensor) {
  os << "tensor";
  return os;
}

}
