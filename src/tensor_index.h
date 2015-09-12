#ifndef SIMIT_TENSOR_INDEX_H
#define SIMIT_TENSOR_INDEX_H

#include <string>
#include <ostream>
#include <vector>

#include "var.h"
#include "path_expressions.h"

namespace simit {
namespace ir {

/// A tensor index describes a mapping of the form source->coordinate->sink,
/// where source is an element of the source dimension, sink an element of the
/// sink dimension and a coordinate corresponds to the location of a tensor
/// value.
///
/// A tensor index consist of a path index and source and sink dimensions. The
/// path indices that implement tensor indices can be shared among.
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
