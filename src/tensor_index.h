#ifndef SIMIT_TENSOR_INDEX_H
#define SIMIT_TENSOR_INDEX_H

#include <string>
#include <ostream>
#include <vector>

#include "var.h"
#include "path_expressions.h"
#include "stencils.h"

namespace simit {
namespace ir {

/// A tensor index is a map source->coordinate->sink described by a path
/// expression.
class TensorIndex {
public:
  enum Kind {PExpr, Sten};
  
  TensorIndex() {}

  TensorIndex(std::string name, pe::PathExpression pexpr);

  TensorIndex(std::string name, Stencil stencil);

  const std::string getName() const {return name;}

  const Kind getKind() const {return kind;}

  const pe::PathExpression& getPathExpression() const {
    iassert(kind == PExpr);
    return pexpr;
  }
  const Stencil& getStencil() const {
    iassert(kind == Sten);
    return stencil;
  }

  const Var& getCoordArray() const {
    iassert(kind == PExpr);
    return coordArray;
  }
  const Var& getSinkArray() const {
    iassert(kind == PExpr);
    return sinkArray;
  }

private:
  Kind kind;
  std::string name;
  pe::PathExpression pexpr;
  Stencil stencil;
  Var coordArray;
  Var sinkArray;
};

std::ostream& operator<<(std::ostream&, const TensorIndex&);

}}

#endif
