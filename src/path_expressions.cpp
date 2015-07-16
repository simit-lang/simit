#include "path_expressions.h"

#include <string>

#include "error.h"
#include "graph.h"

namespace simit {
namespace pe {

// struct Var
Var::Var() : util::IntrusivePtr<const VarContent,false>() {
}

Var::Var(const std::string &name)
    : util::IntrusivePtr<const VarContent,false>(new VarContent(name)) {
}

Var::Var(const std::string &name, const Set &set)
    : util::IntrusivePtr<const VarContent,false>(new VarContent(name, &set)) {
}

const std::string &Var::getName() const {
  return ptr->name;
}

bool Var::isBound() const {
  return ptr->set != nullptr;
}

const Set *Var::getBinding() const {
  iassert(isBound()) << "attempting to get binding of unbound var" << getName();
  return ptr->set;
}

void Var::accept(PathExpressionVisitor *v) const {
  v->visit(*this);
}

std::ostream &operator<<(std::ostream& os, const Var& v) {
  iassert(v.defined()) << "attempting to print undefined var";

  if (v.isBound() && v.getBinding()->getName() != "") {
    os << "(" << v.getName() << " in " << v.getBinding()->getName() << ")";
  }
  else {
    os << v.getName();
  }
  return os;
}


// class PathExpression
class BindPathExpression : public PathExpressionRewriter {
public:
  BindPathExpression(const PathExpression::Bindings &bindings)
      : bindings(bindings) {}

  PathExpression bind(const PathExpression &pe) {
    return this->rewrite(pe);
  }

private:
  const PathExpression::Bindings &bindings;

  void visit(const Var &v) {
    iassert(bindings.find(v) != bindings.end()) << "no binding for" << v;
    var = Var(v.getName(), bindings.at(v));
  }
};

PathExpression::PathExpression(const PathExpressionImpl *impl,
                               const Bindings &bindings)
    : IntrusivePtr(BindPathExpression(bindings).bind(impl).ptr) {
}

PathExpression PathExpression::bind(const Bindings &bindings) const {
  return PathExpression(ptr, bindings);
}

bool PathExpression::isBound() const {
#ifndef WITHOUT_INTERNAL_ASSERTS
  // Invariant: Either all or none are bound
  class CheckThatAllOrNoneAreBound : public PathExpressionVisitor {
  public:
    enum AllOrNoneBoundState { Unknown, None, All };
    AllOrNoneBoundState allOrNoneBoundState;
    void visit(const Var &var) {
      switch (allOrNoneBoundState) {
        case Unknown:
          allOrNoneBoundState = (var.isBound()) ? All : None;
          break;
        case None:
          iassert(!var.isBound())
              << "Some but not all variables in the PathExpression are bound";
          break;
        case All:
          iassert(var.isBound())
              << "Some but not all variables in the PathExpression are bound";
          break;
      }
    }
  };
  CheckThatAllOrNoneAreBound visitor;
  this->accept(&visitor);
#endif

  class CheckIfBound : public PathExpressionVisitor {
  public:
    bool isBound = false;
    bool check(const PathExpression &pe) {
      pe.accept(this);
      return isBound;
    }
    void visit(const Var &var) {
      isBound = var.isBound();
    }
  };
  return CheckIfBound().check(*this);
}

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
void PathExpressionVisitor::visit(const Var &var) {
}

void PathExpressionVisitor::visit(const EV *pe) {
  pe->getE().accept(this);
  pe->getV().accept(this);
}

void PathExpressionVisitor::visit(const VE *pe) {
  pe->getV().accept(this);
  pe->getE().accept(this);
}

void PathExpressionVisitor::visit(const And *pe) {
  pe->getLhs().accept(this);
  pe->getRhs().accept(this);
}


// class PathExpressionRewriter
Var PathExpressionRewriter::rewrite(Var v) {
  if (v.defined()) {
    v.accept(this);
    v = var;
  }
  else {
    v = Var();
  }
  var = Var();
  expr = PathExpression();
  return v;
}

PathExpression PathExpressionRewriter::rewrite(PathExpression e) {
  if (e.defined()) {
    e.accept(this);
    e = expr;
  }
  else {
    e = PathExpression();
  }
  var = Var();
  expr = PathExpression();
  return e;
}

void PathExpressionRewriter::visit(const Var &v) {
  var = v;
}

void PathExpressionRewriter::visit(const EV *pe) {
  Var e = rewrite(pe->getE());
  Var v = rewrite(pe->getV());
  if (e.ptr == pe->getE().ptr && v.ptr == pe->getV().ptr) {
    expr = pe;
  }
  else {
    expr = EV::make(e, v);
  }
}

void PathExpressionRewriter::visit(const VE *pe) {
  Var v = rewrite(pe->getV());
  Var e = rewrite(pe->getE());
  if (v.ptr == pe->getV().ptr && e.ptr == pe->getE().ptr) {
    expr = pe;
  }
  else {
    expr = VE::make(v, e);
  }
}

template <class T>
PathExpression visitBinaryConnective(const T *pe, PathExpressionRewriter *rw) {
  PathExpression l = rw->rewrite(pe->getLhs());
  PathExpression r = rw->rewrite(pe->getRhs());
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
