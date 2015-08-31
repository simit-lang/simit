#include "tensor_index.h"

#include "error.h"
#include "util.h"

using namespace std;

namespace simit {
namespace ir {

ostream &operator<<(ostream& os, const TensorIndex& ti) {
  os << "TensorIndex (" << ti.getSourceDimension() << " -> "
     << ti.getSinkDimension() << "):" << std::endl;
  os << ti.getCoordsArray() << std::endl;
  os << ti.getSinksArray();
  return os;
}

}}
