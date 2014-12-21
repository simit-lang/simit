#include "var.h"

#include <ostream>

#include "ir.h"
#include "reduction.h"

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
  ReductionOperator reductionOperator;
};

LoopVar::LoopVar(Var var, const ForDomain &domain) : content(new Content) {
  content->var = var;
  content->domain = domain;
  content->reductionOperator = ReductionOperator::Undefined;
}

LoopVar::LoopVar(Var var, const ForDomain &domain, const ReductionOperator &rop)
    : LoopVar(var, domain) {
  content->reductionOperator = rop;
}

const Var &LoopVar::getVar() const {
  return content->var;
}

const ForDomain &LoopVar::getDomain() const {
  return content->domain;
}

bool LoopVar::hasReduction() const {
  return content->reductionOperator != ReductionOperator::Undefined;
}

ReductionOperator LoopVar::getReductionOperator() const {
  return content->reductionOperator;
}

// Free functions
std::ostream &operator<<(std::ostream &os, const Var &v) {
  return os << v.getName();
}

std::ostream &operator<<(std::ostream &os, const LoopVar &lv) {
  return os << lv.getVar() << lv.getReductionOperator() << " in " << lv.getDomain();
}

}}
