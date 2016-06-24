#include "tensor_index.h"

#include "error.h"
#include "util/util.h"

#include "path_expressions.h"
#include "var.h"

using namespace std;

namespace simit {
namespace ir {

struct TensorIndex::Content {
  std::string name;
  pe::PathExpression pexpr;
  Var coordArray;
  Var sinkArray;
};

TensorIndex::TensorIndex(std::string name, pe::PathExpression pexpr)
    : content(new Content) {
  content->name = name;
  content->pexpr = pexpr;

  string prefix = (name == "") ? name : name + "_";
  content->coordArray = Var(prefix + "coords", ArrayType::make(ScalarType::Int));
  content->sinkArray  = Var(prefix + "sinks",  ArrayType::make(ScalarType::Int));
}

const std::string TensorIndex::getName() const {
  return content->name;
}

const pe::PathExpression& TensorIndex::getPathExpression() const {
  return content->pexpr;
}

const Var& TensorIndex::getCoordArray() const {
  return content->coordArray;
}

const Var& TensorIndex::getSinkArray() const {
  return content->sinkArray;
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
