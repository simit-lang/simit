#include "tensor_index.h"

#include "error.h"
#include "util/util.h"

using namespace std;

namespace simit {
namespace ir {

TensorIndex::TensorIndex(std::string name, pe::PathExpression pexpr)
    : name(name), pexpr(pexpr), kind(PExpr) {
  string prefix = (name == "") ? name : name + "_";
  this->coordArray = Var(prefix + "coords", ArrayType::make(ScalarType::Int));
  this->sinkArray  = Var(prefix + "sinks",  ArrayType::make(ScalarType::Int));
}

TensorIndex::TensorIndex(std::string name, Stencil stencil)
    : name(name), stencil(stencil), kind(Sten) {}

ostream &operator<<(ostream& os, const TensorIndex& ti) {
  if (ti.getKind() == TensorIndex::PExpr) {
    auto coords = ti.getCoordArray();
    auto sinks = ti.getSinkArray();
    os << "tensor-index " << ti.getName() << ": " << ti.getPathExpression()
       << endl;
    os << "  " << coords << " : " << coords.getType() << endl;
    os << "  " << sinks << " : " << sinks.getType();
  }
  else if (ti.getKind() == TensorIndex::Sten) {
    os << "tensor-index " << ti.getName() << ": " << ti.getStencil() << endl;
  }
  else {
    not_supported_yet;
  }
  return os;
}

}}
