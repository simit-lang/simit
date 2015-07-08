#include "path_expressions.h"

#include <string>
#include "error.h"

namespace simit {
namespace pe {

// struct Var
Var::Var() : util::IntrusivePtr<VarContent>() {
}

Var::Var(std::string name) : util::IntrusivePtr<VarContent>(new VarContent) {
  ptr->name = name;
}

std::string Var::getName() const {
  return ptr->name;
}

std::ostream &operator<<(std::ostream& os, const Var& v) {
  os << v.getName();
  return os;
}


// class PathExpression
Var PathExpression::getPathEndpoint(unsigned i) const {
  return ptr->getPathEndpoint(i);
}

void PathExpression::accept(PathExpressionVisitor *visitor) const {
  ptr->accept(visitor);
}

std::ostream &operator<<(std::ostream& os, const PathExpression& pe) {
  os << *pe.ptr;
  return os;
}


// class EV
EV::EV(Var E, Var V) : E(E), V(V) {
}

PathExpression EV::make(Var E, Var V) {
  return PathExpression(new EV(E, V));
}

Var EV::getPathEndpoint(unsigned i) const {
  iassert(i < 2);
  return (i == 0) ? E : V;
}

void EV::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}

void EV::print(std::ostream &os) const {
  os << E << "-" << V;
}


// class VE
VE::VE(Var V, Var E) : V(V), E(E) {
}

PathExpression VE::make(Var V, Var E) {
  return new VE(V, E);
}

Var VE::getPathEndpoint(unsigned i) const {
  iassert(i < 2);
  return (i == 0) ? V : E;
}

void VE::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}

void VE::print(std::ostream &os) const {
  os << V << "-" << E;
}


// class Formula
Formula::Formula(const std::vector<Var> &freeVars,
                 const Quantifier &quantifier,
                 const Predicate &predicate)
    : freeVars(freeVars), quantifier(quantifier), predicate(predicate) {
  iassert(freeVars.size() == 2)
      << "Only currently support matrix path expressions";
}

PathExpression Formula::make(const std::vector<Var> &freeVars,
                             const Quantifier &quantifier,
                             const Predicate &predicate) {
  return new Formula(freeVars, quantifier, predicate);
}

Var Formula::getPathEndpoint(unsigned i) const {
  return freeVars[i];
}

void Formula::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}

void Formula::print(std::ostream &os) const {
  os << "(" << freeVars[0] << "," << freeVars[1] << ") " << quantifier
     << " | " << predicate;
}

}}
