#include "environment.h"

#include "var.h"
#include "ir.h"
#include "tensor_index.h"
#include "util/collections.h"

using namespace std;

namespace simit {
namespace ir {

struct Environment::Content {
  vector<pair<Var, Expr>> constants;
  vector<Var>             externs;
  vector<Var>             temporaries;

  map<Var, TensorIndex> tensorIndices;
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

bool Environment::isEmpty() const {
  return content->constants.size() == 0 && content->externs.size() == 0 &&
         content->temporaries.size() == 0;
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

const TensorIndex& Environment::getTensorIndex(const Var& tensor,
                                               unsigned sourceDim,
                                               unsigned sinkDim) {
  iassert(tensor.getType().isTensor()) << "Only tensors have tensor indices";
  tassert(sourceDim == 0 && sinkDim == 1)
      << "Only currently support row->col indices";

  if (!util::contains(content->tensorIndices, tensor)) {
    string name = tensor.getName() + "_rows2cols";

    Var coordsArray(name+"_coords", ArrayType::make(ScalarType::Int));
    Var sinksArray(name+"_sinks",   ArrayType::make(ScalarType::Int));

    TensorIndex ti(tensor, coordsArray, sinksArray, sourceDim, sinkDim);
    addTensorIndex(tensor, ti);
  }
  return content->tensorIndices.at(tensor);
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

void Environment::addTensorIndex(Var tensor, TensorIndex ti) {
  iassert(util::contains(content->externs, tensor) ||
          util::contains(content->temporaries, tensor))
      << tensor << " is not an extern or temporary";

  if (util::contains(content->externs, tensor)) {
    addExtern(ti.getCoordsArray());
    addExtern(ti.getSinksArray());
  }
  else {  // (util::contains(content->temporaries, tensor))
    addTemporary(ti.getCoordsArray());
    addTemporary(ti.getSinksArray());
  }

  content->tensorIndices.insert({tensor, ti});
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
