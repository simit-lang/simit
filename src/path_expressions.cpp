#include "path_expressions.h"

#include <string>
#include <map>
#include <vector>

#include "error.h"
#include "graph.h"

using namespace std;

namespace simit {
namespace pe {

// struct Var
Var::Var() : util::IntrusivePtr<const VarContent>() {
}

Var::Var(const std::string &name)
    : util::IntrusivePtr<const VarContent>(new VarContent(name)) {
}

Var::Var(const std::string &name, const Set &set)
    : util::IntrusivePtr<const VarContent>(new VarContent(name, &set)) {
}

const std::string &Var::getName() const {
  return ptr->name;
}

bool Var::isBound() const {
  return ptr->set != nullptr;
}

const Set *Var::getBinding() const {
  iassert(isBound())
      << "attempting to get binding of unbound var " << getName();
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
  map<Var, Var> boundVars;

  void visit(const Var &v) {
    // If we've already created a bound replacement for this Var then insert it
    if (boundVars.find(v) != boundVars.end()) {
      var = boundVars.at(v);
    }
    else {
      iassert(bindings.find(v) != bindings.end()) << "no binding for" << v;
      var = Var(v.getName(), bindings.at(v));
      boundVars.insert({v, var});
    }
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

void PathExpression::accept(PathExpressionVisitor *visitor) const {
  ptr->accept(visitor);
}


// class Link
PathExpression Link::make(const Var &lhs, const Var &rhs, Type type) {
  return PathExpression(new Link(lhs, rhs, type));
}

Link::Link(const Var &lhs, const Var &rhs, Type type)
    : lhs(lhs), rhs(rhs), type(type) {
}

Var Link::getPathEndpoint(unsigned i) const {
  iassert(i < 2) << "attempting to retrieve non-existing path endpoint";
  return (i==0) ? lhs : rhs;
}

void Link::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}

void Link::print(std::ostream &os) const {
    os << lhs << "-" << rhs;
}


static bool allOrNoneBound(const Link *l) {
  iassert(l->getNumPathEndpoints() == 2)
      << "only binary path expressions currently supported";
  return ( l->getLhs().isBound() &&  l->getRhs().isBound())
      || (!l->getLhs().isBound() && !l->getRhs().isBound());
}

bool Link::eq(const PathExpressionImpl &o) const {
  const Link *optr = static_cast<const Link*>(&o);
  iassert(allOrNoneBound(this)) << "either all should be bound or none";
  iassert(allOrNoneBound(optr)) << "either all should be bound or none";
  return (!getLhs().isBound() || !optr->getLhs().isBound())
      || (getLhs().getBinding() == optr->getLhs().getBinding() &&
          getRhs().getBinding() == optr->getRhs().getBinding());
}

bool Link::lt(const PathExpressionImpl &o) const {
  const Link *optr = static_cast<const Link*>(&o);
  iassert(allOrNoneBound(this)) << "either all should be bound or none";
  iassert(allOrNoneBound(optr)) << "either all should be bound or none";

  return (getLhs().isBound() && optr->getLhs().isBound())
      && ((getLhs().getBinding() != optr->getLhs().getBinding())
           ? getLhs().getBinding() < optr->getLhs().getBinding()
           : getRhs().getBinding() < optr->getRhs().getBinding());
}


// class QuantifiedConnective
QuantifiedConnective::QuantifiedConnective(const vector<Var> &freeVars,
                                           const vector<QVar> &quantifiedVars,
                                           const PathExpression &lhs,
                                           const PathExpression &rhs)
    : freeVars(freeVars), quantifiedVars(quantifiedVars), lhs(lhs), rhs(rhs) {
  // TODO: Remove these restrictions
  iassert(freeVars.size() == 2)
      << "For now, we only support matrix path expressions";
  iassert(quantifiedVars.size() == 1)
      << "For now, we only support one quantified variable";
}

Var QuantifiedConnective::getPathEndpoint(unsigned i) const {
  return freeVars[i];
}

void QuantifiedConnective::print(std::ostream &os) const {
  os << "(" << freeVars[0] << "," << freeVars[1] << ") " << quantifiedVars[0];
}


// class QuantifiedAnd
PathExpression QuantifiedAnd::make(const std::vector<Var> &freeVars,
                                   const std::vector<QVar> &quantifiedVars,
                                   const PathExpression &l,
                                   const PathExpression &r) {
  return new QuantifiedAnd(freeVars, quantifiedVars, l, r);
}

void QuantifiedAnd::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}

void QuantifiedAnd::print(std::ostream &os) const {
  QuantifiedConnective::print(os);
  os << " | (" << getLhs() << ") \u2227 (" << getRhs() << ")";
}


// class PathExpressionVisitor
void PathExpressionVisitor::visit(const Var &var) {
}

void PathExpressionVisitor::visit(const Link *pe) {
  pe->getLhs().accept(this);
  pe->getRhs().accept(this);
}

void PathExpressionVisitor::visit(const QuantifiedAnd *pe) {
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

void PathExpressionRewriter::visit(const Link *pe) {
  Var lhs = rewrite(pe->getLhs());
  Var rhs = rewrite(pe->getRhs());
  if (lhs.ptr == pe->getLhs().ptr && rhs.ptr == pe->getRhs().ptr) {
    expr = pe;
  }
  else {
    expr = Link::make(lhs, rhs, pe->getType());
  }
}

template <class T>
PathExpression visitBinaryConnective(const T *pe, PathExpressionRewriter *rw) {
  bool varsChanged = false;

  vector<Var> freeVars;
  for (size_t i=0; i < pe->getFreeVars().size(); ++i) {
    freeVars.push_back(rw->rewrite(pe->getFreeVars()[i]));
    if (freeVars[i] != pe->getFreeVars()[i]) {
      varsChanged = true;
    }
  }

  vector<QVar> qVars;
  for (auto &qvar : pe->getQVars()) {
    Var var = rw->rewrite(qvar.getVar());
    if (var != qvar.getVar()) {
      varsChanged = true;
      qVars.push_back(QVar(qvar.getQuantifier(), var));
    }
    else {
      qVars.push_back(qvar);
    }
  }

  PathExpression l = rw->rewrite(pe->getLhs());
  PathExpression r = rw->rewrite(pe->getRhs());
  if (!varsChanged && l.ptr == pe->getLhs().ptr && r.ptr == pe->getRhs().ptr) {
    return pe;
  }
  else {
    return T::make(freeVars, qVars, l, r);
  }
}

void PathExpressionRewriter::visit(const QuantifiedAnd *pe) {
  expr = visitBinaryConnective(pe, this);
}

}}
