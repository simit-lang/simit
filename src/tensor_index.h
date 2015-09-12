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

  TensorIndex(Var coordArray, Var sinkArray, unsigned srcDim, unsigned sinkDim)
      : coordArray(coordArray), sinkArray(sinkArray),
        sourceDimension(srcDim), sinkDimension(sinkDim) {
  }

  const Var& getCoordArray() const {return coordArray;}
  const Var& getSinkArray() const {return sinkArray;}

  unsigned getSourceDimension() const {return sourceDimension;}
  unsigned getSinkDimension() const {return sinkDimension;}

private:
  Var coordArray;
  Var sinkArray;

  unsigned sourceDimension;
  unsigned sinkDimension;
};

std::ostream& operator<<(std::ostream&, const TensorIndex&);

}}

#endif
