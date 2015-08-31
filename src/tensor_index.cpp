#include "tensor_index.h"

#include "error.h"
#include "util.h"

using namespace std;

namespace simit {
namespace ir {

ostream &operator<<(ostream &os, const TensorIndex &ti) {
  os << ti.getName() << ".";
  if (ti.getSourceDimension() == 0) {
    os << "row2col";
  }
  else if (ti.getSourceDimension() == 1) {
    os << "col2row";
  }
  else {
    not_supported_yet;
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const std::vector<TensorIndex> &tis){
  os << "Tensor Indices:" << endl;
  os << util::join(tis, "\n");
  return os;
}

}}
