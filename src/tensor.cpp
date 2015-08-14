#include "tensor.h"

#include <ostream>
#include <string>

#include "ir.h"
#include "types.h"

using namespace std;

namespace simit {

// class Tensor
struct Tensor::Content {
  ir::Type type;
  simit::ir::Expr literal;
};

Tensor::Tensor(const ir::Expr &literal) : content(new Tensor::Content) {
  uassert(ir::isa<ir::Literal>(literal));
  content->type = literal.type();
  content->literal = literal;
}

Tensor::Tensor(int val) : Tensor(ir::Expr(val)) {
}

Tensor::Tensor(double val) : Tensor(ir::Expr(val)) {
}

Tensor::Tensor(const Tensor &other) : content(new Tensor::Content) {
  *content = *other.content;
}

Tensor::Tensor(Tensor &&other) noexcept : content(other.content) {
  other.content = nullptr;
}

Tensor &Tensor::operator=(const Tensor &other) {
  if (&other == this) {
    return *this;
  }
  Tensor tmp(other);
  *this = std::move(tmp);
  return *this;
}

Tensor &Tensor::operator=(Tensor &&other) noexcept {
  std::swap(content, other.content);
  return *this;
}

Tensor::~Tensor() {
  if (content != nullptr) {
    delete content;
    content = nullptr;
  }
}

const ir::Type &Tensor::getType() const {
  return content->type;
}

void *Tensor::getData() {
  const ir::Literal *literal = ir::to<ir::Literal>(content->literal);
  return literal->data;
}

const void *Tensor::getData() const {
  const ir::Literal *literal = ir::to<ir::Literal>(content->literal);
  return literal->data;
}

bool operator==(const Tensor &l, const Tensor &r) {
  return *ir::to<ir::Literal>(l.content->literal)
      == *ir::to<ir::Literal>(r.content->literal);
}

bool operator!=(const Tensor &l, const Tensor &r) {
  return *ir::to<ir::Literal>(l.content->literal)
      != *ir::to<ir::Literal>(r.content->literal);
}

std::ostream &operator<<(std::ostream &os, const Tensor &tensor) {
  os << "tensor";
  return os;
}

}
