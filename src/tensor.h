#ifndef SIMIT_TENSOR_H
#define SIMIT_TENSOR_H

#include <ostream>
#include <memory>

#include "ir.h" // TODO: Remove

namespace simit {

class Tensor {
public:
  Tensor(const simit::ir::Expr &literal);
  Tensor(int val);
  Tensor(double val);

  Tensor(const Tensor &other);
  Tensor(Tensor &&other) noexcept;
  Tensor &operator=(const Tensor &other);
  Tensor &operator=(Tensor &&other) noexcept;
  ~Tensor();

  const ir::Type &type();
  operator simit::ir::Expr();

  friend std::ostream &operator<<(std::ostream &os, const Tensor &tensor);

private:
  struct Content;
  Content *content;
};

}
#endif
