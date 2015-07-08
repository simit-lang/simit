#include "path_expressions.h"

#include <string>
#include "error.h"

namespace simit {
namespace pe {

// struct Var
Var::Var() : util::IntrusivePtr<VarContent>() {
}

Var::Var(std::string setName)
    : util::IntrusivePtr<VarContent>(new VarContent) {
  ptr->setName = setName;
}

std::string Var::getSetName() const {
  return ptr->setName;
}

std::ostream &operator<<(std::ostream& os, const Var& v) {
  os << v.getSetName() << "_i" << " in " << v.getSetName();
  return os;
}


// class PathExpression
Var PathExpression::getPathEndpoint(unsigned pathEndpoint) const {
  return ptr->getPathEndpoint(pathEndpoint);
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

Var EV::getPathEndpoint(unsigned pathEndpoint) const {
  iassert(pathEndpoint < 2);
  return (pathEndpoint == 0) ? E : V;
}

void EV::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}

void EV::print(std::ostream &os) const {
  os << "(" << E << ")-(" << V << ")";
}


// class VE
VE::VE(Var V, Var E) : V(V), E(E) {
}

PathExpression VE::make(Var V, Var E) {
  return PathExpression(new VE(V, E));
}

Var VE::getPathEndpoint(unsigned pathEndpoint) const {
  iassert(pathEndpoint < 2);
  return (pathEndpoint == 0) ? V : E;
}

void VE::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}

void VE::print(std::ostream &os) const {
  os << "(" << V << ")-(" << E << ")";
}


// class Predicate
Predicate::Predicate() {
}

Var Predicate::getPathEndpoint(unsigned pathEndpoint) const {
}

void Predicate::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}

void Predicate::print(std::ostream &os) const {
}

}}
