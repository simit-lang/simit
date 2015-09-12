#include "tensor_index.h"

#include "error.h"
#include "util.h"

using namespace std;

namespace simit {
namespace ir {

ostream &operator<<(ostream& os, const TensorIndex& ti) {
  os << "TensorIndex (" << ti.getSourceDimension() << " -> "
     << ti.getSinkDimension() << ")";
  os << ":" << endl;
  os << "  " << ti.getCoordArray() << endl;
  os << "  " << ti.getSinkArray();
  return os;
}

}}
