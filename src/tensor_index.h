#ifndef SIMIT_TENSOR_INDEX_H
#define SIMIT_TENSOR_INDEX_H

#include <string>
#include <ostream>
#include <vector>

#include "var.h"

namespace simit {
namespace ir {

/// A tensor index describes
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
