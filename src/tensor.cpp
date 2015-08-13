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

Tensor::Tensor(const simit::ir::Expr &literal)
    : content(new Tensor::Content) {
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
}

const ir::Type &Tensor::type() {
  return content->type;
}

Tensor::operator simit::ir::Expr() {
  return content->literal;
}

std::ostream &operator<<(std::ostream &os, const Tensor &tensor) {
  os << "tensor";
  return os;
}

}
