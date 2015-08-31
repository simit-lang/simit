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

  TensorIndex(std::string name,
              unsigned sourceDim, unsigned sinkDim)
      : name(name), sourceDimension(sourceDim), sinkDimension(sinkDim) {}

  std::string getName() const {return name;}

  const Var& getCoordsArray() const {return coordsArray;}
  const Var& getSinksArray() const {return sinksArray;}

  unsigned getSourceDimension() const {return sourceDimension;}
  unsigned getSinkDimension() const {return sinkDimension;}

private:
  std::string name;
  Var coordsArray;
  Var sinksArray;

  unsigned sourceDimension;
  unsigned sinkDimension;
};

std::ostream &operator<<(std::ostream&, const TensorIndex&);
std::ostream &operator<<(std::ostream&, const std::vector<TensorIndex>&);

}}

#endif
