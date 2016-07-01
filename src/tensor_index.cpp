#include "tensor_index.h"

#include "error.h"
#include "util/util.h"

#include "path_expressions.h"
#include "var.h"

using namespace std;

namespace simit {
namespace ir {

struct TensorIndex::Content {
  Kind kind;
  std::string name;
  pe::PathExpression pexpr;
  StencilLayout stencil;
  Var coordArray;
  Var sinkArray;
};

TensorIndex::TensorIndex(std::string name, pe::PathExpression pexpr)
    : content(new Content) {
  content->name = name;
  content->pexpr = pexpr;
  content->kind = PExpr;

  string prefix = (name == "") ? name : name + ".";
  content->coordArray = Var(prefix + "coords", ArrayType::make(ScalarType::Int));
  content->sinkArray  = Var(prefix + "sinks",  ArrayType::make(ScalarType::Int));
}

TensorIndex::TensorIndex(std::string name, StencilLayout stencil)
    : content(new Content) {
  content->name = name;
  content->stencil = stencil;
  content->kind = Sten;
}

const std::string TensorIndex::getName() const {
  return content->name;
}

const TensorIndex::Kind TensorIndex::getKind() const {
  return content->kind;
}

const pe::PathExpression& TensorIndex::getPathExpression() const {
  iassert(content->kind == PExpr);
  return content->pexpr;
}

const StencilLayout& TensorIndex::getStencilLayout() const {
  iassert(content->kind == Sten);
  return content->stencil;
}

void TensorIndex::setStencilLayout(StencilLayout stencil) {
  iassert(content->kind == Sten);
  content->stencil = stencil;
}

const Var& TensorIndex::getRowptrArray() const {
  return content->coordArray;
}

const Var& TensorIndex::getColidxArray() const {
  return content->sinkArray;
}

ostream &operator<<(ostream& os, const TensorIndex& ti) {
  if (ti.getKind() == TensorIndex::PExpr) {
    auto rowptr = ti.getRowptrArray();
    auto colidx = ti.getColidxArray();
    os << "tensor-index " << ti.getName() << ": " << ti.getPathExpression()
       << endl;
    os << "  " << rowptr << " : " << rowptr.getType() << endl;
    os << "  " << colidx << " : " << colidx.getType();
  }
  else if (ti.getKind() == TensorIndex::Sten) {
    os << "tensor-index " << ti.getName() << ": " << ti.getStencilLayout()
       << endl;
  }
  else {
    not_supported_yet;
  }
  return os;
}

}}
