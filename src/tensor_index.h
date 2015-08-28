#ifndef SIMIT_TENSOR_INDEX_H
#define SIMIT_TENSOR_INDEX_H

#include <string>
#include <ostream>
#include <vector>

#include "var.h"

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

  TensorIndex(const Var &tensor, unsigned sourceDimension)
      : tensor(tensor), sourceDimension(sourceDimension)  {}

  const Var &getTensor() const {return tensor;}

  unsigned getSourceDimension() const {return sourceDimension;}
  unsigned getSinkDimension() const {return (sourceDimension==0) ? 1 : 0;}

private:
  Var tensor;
  unsigned sourceDimension;
};

std::ostream &operator<<(std::ostream&, const TensorIndex&);
std::ostream &operator<<(std::ostream&, const std::vector<TensorIndex>&);

}}

#endif
