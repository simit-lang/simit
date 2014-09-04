#include "function.h"

namespace simit {

// class Function
std::ostream &operator<<(std::ostream &os, const Function &f) {
  f.print(os);
  return os;
}

} // namespace simit
