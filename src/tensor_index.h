#ifndef SIMIT_TENSOR_INDEX_H
#define SIMIT_TENSOR_INDEX_H

#include <string>
#include <ostream>
#include <vector>

#include "var.h"
#include "path_expressions.h"

namespace simit {
namespace ir {

/// A tensor index is a map source->coordinate->sink described by a path
/// expression.
class TensorIndex {
public:
  TensorIndex() {}

  TensorIndex(std::string name, pe::PathExpression pexpr);

  const std::string getName() const {return name;}

  const pe::PathExpression& getPathExpression() const {return pexpr;}

  const Var& getCoordArray() const {return coordArray;}
  const Var& getSinkArray() const {return sinkArray;}

private:
  std::string name;
  pe::PathExpression pexpr;
  Var coordArray;
  Var sinkArray;
};

std::ostream& operator<<(std::ostream&, const TensorIndex&);

}}

#endif
