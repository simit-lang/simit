#include "tensor_index.h"

#include "error.h"
#include "util/util.h"

using namespace std;

namespace simit {
namespace ir {

TensorIndex::TensorIndex(std::string name, pe::PathExpression pexpr)
    : name(name), pexpr(pexpr) {
  string prefix = (name == "") ? name : name + "_";
  this->coordArray = Var(prefix + "coords", ArrayType::make(ScalarType::Int));
  this->sinkArray  = Var(prefix + "sinks",  ArrayType::make(ScalarType::Int));
}

ostream &operator<<(ostream& os, const TensorIndex& ti) {
  auto coords = ti.getCoordArray();
  auto sinks = ti.getSinkArray();
  os << "tensor-index " << ti.getName() << ": " << ti.getPathExpression()
     << endl;
  os << "  " << coords << " : " << coords.getType() << endl;
  os << "  " << sinks << " : " << sinks.getType();
  return os;
}

}}
