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

const std::string &Var::getName() const {
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


// class Formula
Formula::Formula(const std::vector<Var> &freeVars,
                 const std::vector<QuantifiedVar> &quantifiedVars)
    : freeVars(freeVars), quantifiedVars(quantifiedVars) {
  // TODO: Remove these restrictions
  iassert(freeVars.size() == 2)
      << "For now, we only support matrix path expressions";
  iassert(quantifiedVars.size() == 1)
      << "For now, we only support one quantified variable";
}

Var Formula::getPathEndpoint(unsigned i) const {
  return freeVars[i];
}

void Formula::print(std::ostream &os) const {
  os << "(" << freeVars[0] << "," << freeVars[1] << ") " << quantifiedVars[0];
}


// class And
PathExpression And::make(const std::vector<Var> &freeVars,
                         const std::vector<QuantifiedVar> &quantifiedVars,
                         const PathExpression &l, const PathExpression &r) {
  return new And(freeVars, quantifiedVars, l, r);
}

void And::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}

void And::print(std::ostream &os) const {
  Formula::print(os);
  os << " | (" << l << ") \u2227 (" << r << ")";
}


// class PathExpressionVisitor
void PathExpressionVisitor::visit(const EV *pe) {
}

void PathExpressionVisitor::visit(const VE *pe) {
}

void PathExpressionVisitor::visit(const And *pe) {
  pe->getLhs().accept(this);
  pe->getRhs().accept(this);
}


// class PathExpressionRewriter
PathExpression PathExpressionRewriter::rewrite(PathExpression e) {
  if (e.defined()) {
    e.accept(this);
    e = expr;
  }
  else {
    e = PathExpression();
  }
  expr = PathExpression();
  return e;
}

void PathExpressionRewriter::visit(const EV *pe) {
  expr = pe;
}

void PathExpressionRewriter::visit(const VE *pe) {
  expr = pe;
}

template <class T>
PathExpression visitBinaryConnective(const T *pe, PathExpressionRewriter *rw) {
  PathExpression l = pe->getLhs();
  PathExpression r = pe->getRhs();
  if (l.ptr == pe->getLhs().ptr && r.ptr == pe->getRhs().ptr) {
    return pe;
  }
  else {
    return T::make(pe->getFreeVars(), pe->getQuantifiedVars(), l, r);
  }
}

void PathExpressionRewriter::visit(const And *pe) {
  expr = visitBinaryConnective(pe, this);
}

}}
