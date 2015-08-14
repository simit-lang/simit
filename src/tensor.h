#ifndef SIMIT_TENSOR_H
#define SIMIT_TENSOR_H

#include <ostream>
#include <memory>

#include "ir.h" // TODO: Remove
#include "comparable.h"

namespace simit {


class Tensor : public interfaces::Comparable<Tensor> {
public:
  Tensor(const simit::ir::Expr &literal);
  Tensor(int val);
  Tensor(double val);

  Tensor(const Tensor &other);
  Tensor(Tensor &&other) noexcept;
  Tensor &operator=(const Tensor &other);
  Tensor &operator=(Tensor &&other) noexcept;
  ~Tensor();

  const ir::Type &getType() const;

  void *getData();
  const void *getData() const;

  friend bool operator==(const Tensor&, const Tensor&);
  friend bool operator!=(const Tensor&, const Tensor&);

  friend std::ostream &operator<<(std::ostream &os, const Tensor &tensor);

private:
  struct Content;
  Content *content;
};

}
#endif
