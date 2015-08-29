#include "environment.h"

#include "var.h"
#include "ir.h"
#include "util/collections.h"

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

const std::vector<Var>& Environment::getBindables() const {
}

const Var& Environment::getBindable(const std::string& name) const {
}

const std::vector<Var>& Environment::getExternsOfBindable() const {
}

void Environment::addConstant(const Var& var, const Expr& initializer) {
  content->constants.push_back({var, initializer});
}

void Environment::addExtern(const Var& var, const Var& bindable) {
  content->externs.push_back(var);
}

void Environment::addTemporary(const Var& var) {
  content->temporaries.push_back(var);
}

std::ostream& operator<<(std::ostream& os, const Environment& env) {
  bool somethingPrinted = false;

  if (env.getConstants().size() > 0) {
    for (auto& con : env.getConstants()) {
      os << "const " << con.first.getName() << " : " << con.first.getType()
         << " = " << con.second << ";" << std::endl;
    }

    os << std::endl << "const " << env.getConstants().begin()->first << " : "
       << env.getConstants().begin()->first.getType()
       << " = " << env.getConstants().begin()->second << ";";
    for (auto& con : util::excludeFirst(env.getConstants())) {
      os << std::endl << "const " << con.first.getName() << " : "
         << con.first.getType() << " = " << con.second << ";";
    }
    somethingPrinted = true;
  }

  if (env.getExterns().size() > 0) {
    if (somethingPrinted) {
      os << std::endl;
    }
    os << "extern " << *env.getExterns().begin()  << " : "
       << env.getExterns().begin()->getType() << ";";
    for (auto& ext : util::excludeFirst(env.getExterns())) {
      os << std::endl << "extern " << ext  << " : " << ext.getType() << ";";
    }
    somethingPrinted = true;
  }

  if (env.getTemporaries().size() > 0) {
    if (somethingPrinted) {
      os << std::endl;
    }

    os << "temp " << *env.getTemporaries().begin()  << " : "
       << env.getTemporaries().begin()->getType() << ";";
    for (auto& temp : util::excludeFirst(env.getTemporaries())) {
      os << std::endl << "temp " << temp  << " : " << temp.getType() << ";";
    }
  }
  return os;
}

}}
