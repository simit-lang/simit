#include "var.h"

#include <ostream>
#include "ir.h"

namespace simit {
namespace ir {

// class Var
Var::Var(std::string name, const Type &type) : IntrusivePtr(new VarContent) {
  ptr->name = name;
  ptr->type = type;
}

// class LoopVar
struct LoopVar::Content {
  Var var;
  ForDomain domain;
};

LoopVar::LoopVar(Var var, const ForDomain &domain) : content(new Content) {
  content->var = var;
  content->domain = domain;
}

LoopVar::~LoopVar() {
  delete content;
}

const Var &LoopVar::var() const {
  return content->var;
}

const ForDomain &LoopVar::domain() const {
  return content->domain;
}

// Free functions
std::ostream &operator<<(std::ostream &os, const Var &v) {
  return os << v.getName();
}

std::ostream &operator<<(std::ostream &os, const LoopVar &lv) {
  return os << lv.var() << " in " << lv.domain();
}

}}
