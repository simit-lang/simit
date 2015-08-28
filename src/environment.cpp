#include "environment.h"

#include "var.h"
#include "ir.h"

namespace simit {
namespace ir {

struct Environment::Content {
  std::vector<std::pair<Var, Expr>> constants;
  std::vector<ir::Var>              externs;
  std::vector<ir::Var>              temporaries;
};

Environment::Environment() : content(new Content) {
}

Environment::~Environment() {
  delete content;
}

Environment::Environment(const Environment& other) : Environment() {
  *content = *other.content;
}

Environment::Environment(Environment&& other) noexcept : content(other.content) {
  other.content = nullptr;
}

Environment& Environment::operator=(const Environment& other) {
  if (&other == this) {
    return *this;
  }
  Environment tmp(other);
  *this = std::move(tmp);
  return *this;
}

Environment& Environment::operator=(Environment&& other) noexcept {
  std::swap(content, other.content);
  return *this;
}

const std::vector<std::pair<Var, Expr>>& Environment::getConstants() const {
  return content->constants;
}

const std::vector<Var>& Environment::getExterns() const {
  return content->externs;
}

const std::vector<Var>& Environment::getTemporaries() const {
  return content->temporaries;
}

const pe::PathExpression&
Environment::getPathExpression(const Var& tensorVar) const {
}

const Var& Environment::getDataArray(const Var& tensorVar) const {
}

const TensorIndex& Environment::getTensorIndex(const Var& tensorVar) const {
}

void* Environment::getTemporaryDataPointer(const Var& tensorVar) const {
}

void Environment::addConstant(const Var& var, const Expr& initializer) {
  content->constants.push_back({var, initializer});
}

void Environment::addExtern(const Var& var) {
  content->externs.push_back(var);
}

void Environment::addTemporary(const Var& var) {
  content->temporaries.push_back(var);
}

std::ostream& operator<<(std::ostream& os, const Environment& env) {
  for (auto& con : env.getConstants()) {
    os << "const " << con.first << " = " << con.second << ";" << std::endl;
  }
  os << std::endl;
  for (auto& ext : env.getExterns()) {
    os << "extern " << ext << ";" << std::endl;
  }
  os << std::endl << "% Global temporaries" << std::endl;
  for (auto& temp : env.getTemporaries()) {
    os << temp << ";" << std::endl;
  }
  return os;
}

}}
