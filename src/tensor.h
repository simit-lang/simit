#ifndef SIMIT_TENSOR_H
#define SIMIT_TENSOR_H

#include <ostream>
#include <memory>

// TODO: Remove
#include "ir.h"
#include "types.h"

// TODO: Replace with a simple tensor implementation
//typedef simit::ir::Expr Tensor;

namespace simit {

class Tensor {
public:
  Tensor(simit::ir::Expr literal) : literal(literal) {}
  Tensor(int val) : literal(val) {}
  Tensor(double val) : literal(val) {}

  operator simit::ir::Expr() {return literal;}

  ir::Type type() {return literal.type();}

  friend std::ostream &operator<<(std::ostream &os, const Tensor &tensor);

private:
  simit::ir::Expr literal;
  class Content;
  class std::shared_ptr<Content> content;
};



}
#endif
