#ifndef SIMIT_TENSOR_H
#define SIMIT_TENSOR_H

#include <ostream>
#include <memory>

namespace simit {

namespace ir {
class Type;
}

class Tensor {
public:
  Tensor(int val);
  Tensor(double val);

  Tensor(const ir::Type &type);
  Tensor(const ir::Type &type, void *data);

  Tensor(const Tensor &other);
  Tensor(Tensor &&other) noexcept;
  Tensor &operator=(const Tensor &other);
  Tensor &operator=(Tensor &&other) noexcept;
  ~Tensor();

  const ir::Type &getType() const;

  unsigned getSize() const;
  unsigned getComponentSize() const;

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
