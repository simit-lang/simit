#include "path_expressions.h"

#include <string>
#include <map>
#include <vector>
#include <algorithm>

#include "error.h"
#include "graph.h"

using namespace std;

namespace simit {
namespace pe {

// struct Var
const std::string &Var::getName() const {
  return ptr->name;
}

void Var::accept(PathExpressionVisitor *v) const {
  v->visit(*this);
}

std::ostream &operator<<(std::ostream& os, const Var& v) {
  iassert(v.defined()) << "attempting to print undefined var";
  return os << v.getName();
}


// class PathExpressionImpl
bool operator==(const PathExpressionImpl &l, const PathExpressionImpl &r) {
  // If one of the operands is a renamed path expression then we compare
  // its sub-path expression that was renamed to the other operand
  auto &lid = typeid(l);
  auto &rid = typeid(r);
  if (lid != rid) {
    static auto &renid = typeid(RenamedPathExpression);
    if (lid == renid) {
      auto lptr = static_cast<const RenamedPathExpression*>(&l);
      return lptr->getPathExpression() == PathExpression(&r);
    }
    else if (rid == renid) {
      auto rptr = static_cast<const RenamedPathExpression*>(&r);
      return PathExpression(&l) == rptr->getPathExpression();
    }
    else {
      return false;
    }
  }
  return l.eq(r);
}

bool operator<(const PathExpressionImpl &l, const PathExpressionImpl &r) {
  // If one of the operands is a renamed path expression then we compare
  // its sub-path expression that was renamed to the other operand
  auto lid = std::type_index(typeid(l));
  auto rid = std::type_index(typeid(r));
  if (lid != rid) {
    static auto &renid = typeid(RenamedPathExpression);
    if (lid == renid) {
      auto lptr = static_cast<const RenamedPathExpression*>(&l);
      return lptr->getPathExpression() < PathExpression(&r);
    }
    else if (rid == renid) {
      auto rptr = static_cast<const RenamedPathExpression*>(&r);
      return PathExpression(&l) < rptr->getPathExpression();
    }
    else {
      return lid < rid;
    }
  }
  return l.lt(r);
}

std::ostream &operator<<(std::ostream &os, const PathExpressionImpl &pe) {
  PathExpressionPrinter(os).print(&pe);
  return os;
}


// class PathExpression
class BindPathExpression : public PathExpressionVisitor {
public:
  BindPathExpression(const PathExpression::Bindings &bindings)
      : bindings(bindings) {}

  void bind(const PathExpression &pe) {
    pe.accept(this);
  }

private:
  const PathExpression::Bindings &bindings;

  void visit(const Link *link) {
    auto &lhs = link->getLhs();
    auto &rhs = link->getRhs();
    iassert(bindings.find(lhs) != bindings.end()) << "no binding for " << lhs;
    iassert(bindings.find(rhs) != bindings.end()) << "no binding for " << rhs;
    link->bind(&bindings.at(lhs), &bindings.at(rhs));
  }
};

void PathExpression::bind(const Bindings &bindings) {
  iassert(isa<Link>(*this)) << "only binding to links is currently supported";
  BindPathExpression(bindings).bind(*this);
}

bool PathExpression::isBound() const {
#ifndef WITHOUT_INTERNAL_ASSERTS
  /// Check invariant: either all or none are bound
  class CheckThatAllOrNoneAreBound : public PathExpressionVisitor {
  public:
    enum AllOrNoneBoundState { Unknown, None, All };
    AllOrNoneBoundState allOrNoneBoundState;

    void visit(const Link *link) {
      switch (allOrNoneBoundState) {
        case Unknown:
          allOrNoneBoundState = link->isBound() ? All : None;
          break;
        case None:
          iassert(!link->isBound())
              << "Some but not all variables in the PathExpression are bound";
          break;
        case All:
          iassert(link->isBound())
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
    void visit(const Link *link) {
      isBound = link->isBound();
    }
  };
  return CheckIfBound().check(*this);
}

void PathExpression::accept(PathExpressionVisitor *visitor) const {
  ptr->accept(visitor);
}

PathExpression PathExpression::operator()(Var v0, Var v1) {
  return new RenamedPathExpression(*this, {{getPathEndpoint(0),v0},
                                   {getPathEndpoint(1),v1}});
}

std::ostream &operator<<(std::ostream &os, const PathExpression &pe) {
  PathExpressionPrinter(os).print(pe);
  return os;
}


// class Link
Link::Link(const Var &lhs, const Var &rhs, Type type)
    : type(type), lhs(lhs), rhs(rhs), lhsBinding(nullptr), rhsBinding(nullptr) {
}

PathExpression Link::make(const Var &lhs, const Var &rhs, Type type) {
  return PathExpression(new Link(lhs, rhs, type));
}

void Link::bind(const Set *lhsBinding, const Set *rhsBinding) const {
  this->lhsBinding = lhsBinding;
  this->rhsBinding = rhsBinding;
}

bool Link::isBound() const {
  iassert((lhsBinding != nullptr && rhsBinding != nullptr)
       || (lhsBinding == nullptr && rhsBinding == nullptr))
      << "either all should be bound or none";
  return lhsBinding != nullptr;
}

Var Link::getPathEndpoint(unsigned i) const {
  iassert(i < 2) << "attempting to retrieve non-existing path endpoint";
  return (i==0) ? lhs : rhs;
}

void Link::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}

bool Link::eq(const PathExpressionImpl &o) const {
  auto optr = static_cast<const Link*>(&o);
  return !this->isBound()
      || !optr->isBound()
      || (this->getLhsBinding() == optr->getLhsBinding() &&
          this->getRhsBinding() == optr->getRhsBinding());
}

bool Link::lt(const PathExpressionImpl &o) const {
  auto optr = static_cast<const Link*>(&o);
  return this->isBound()
      && optr->isBound()
      && ((this->getLhsBinding() != optr->getLhsBinding())
              ? this->getLhsBinding() < optr->getLhsBinding()
              : this->getRhsBinding() < optr->getRhsBinding());
}


// class QuantifiedConnective
QuantifiedConnective::QuantifiedConnective(const vector<Var> &freeVars,
                                           const vector<QuantifiedVar> &qvars,
                                           const PathExpression &lhs,
                                           const PathExpression &rhs)
    : freeVars(freeVars), quantifiedVars(qvars), lhs(lhs), rhs(rhs) {
  // TODO: Remove these restrictions
  iassert(freeVars.size() == 2)
      << "For now, we only support matrix path expressions";
  iassert(quantifiedVars.size() == 1)
      << "For now, we only support one quantified variable";
}

Var QuantifiedConnective::getPathEndpoint(unsigned i) const {
  return freeVars[i];
}


bool QuantifiedConnective::eq(const PathExpressionImpl &o) const {
  auto optr = static_cast<const QuantifiedConnective*>(&o);
  return getLhs() == optr->getLhs() && getRhs() == optr->getRhs();
}

bool QuantifiedConnective::lt(const PathExpressionImpl &o) const {
  auto optr = static_cast<const QuantifiedConnective*>(&o);
  return (getLhs() != optr->getLhs()) ? getLhs() < optr->getLhs()
                                      : getRhs() < optr->getRhs();
}

// class QuantifiedAnd
PathExpression QuantifiedAnd::make(const std::vector<Var> &freeVars,
                                   const vector<QuantifiedVar> &qvars,
                                   const PathExpression &l,
                                   const PathExpression &r) {
  return new QuantifiedAnd(freeVars, qvars, l, r);
}

void QuantifiedAnd::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}


// class RenamedPathExpression
Var RenamedPathExpression::getPathEndpoint(unsigned i) const {
  iassert(i < pe.getNumPathEndpoints())
      << "path expression does not have requested endpoint";
  iassert(renames.find(pe.getPathEndpoint(i)) != renames.end())
      << "no mapping exist for endpoint " << pe.getPathEndpoint(i);
  return renames.at(pe.getPathEndpoint(i));
}

void RenamedPathExpression::accept(PathExpressionVisitor *visitor) const {
  visitor->visitRename(this);
}

bool RenamedPathExpression::eq(const PathExpressionImpl &o) const {
  auto optr = static_cast<const RenamedPathExpression*>(&o);
  return pe == optr->pe;
}

bool RenamedPathExpression::lt(const PathExpressionImpl &o) const {
  auto optr = static_cast<const RenamedPathExpression*>(&o);
  return pe < optr->pe;
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

void PathExpressionVisitor::visit(const RenamedPathExpression *pe) {
  pe->getPathExpression().accept(this);
}

const Var &PathExpressionVisitor::rename(const Var &var) const {
  return renames.contains(var) ? renames.get(var) : var;
}

void PathExpressionVisitor::visitRename(const RenamedPathExpression *pe) {
  renames.scope();
  for (auto &rename : pe->getRenames()) {
    // If v0 is renamed to a v1, which is renamed to v2, then rename v0 to v2
    if (renames.contains(rename.second)) {
      renames.insert(rename.first, renames.get(rename.second));
    }
    else {
      renames.insert(rename);
    }
  }
  visit(pe);  // template method
  renames.unscope();
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

  vector<QuantifiedVar> qVars;
  for (auto &qvar : pe->getQuantifiedVars()) {
    Var var = rw->rewrite(qvar.getVar());
    if (var != qvar.getVar()) {
      varsChanged = true;
      qVars.push_back(QuantifiedVar(qvar.getQuantifier(), var));
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

void PathExpressionRewriter::visit(const RenamedPathExpression *pe) {
  PathExpression renamedPe = rewrite(pe->getPathExpression());
  if (renamedPe == pe->getPathExpression()) {
    expr = pe;
  }
  else {
    // TODO: Test this
    expr = renamedPe(pe->getPathEndpoint(0), pe->getPathEndpoint(1));
  }
}


// class PathExpressionPrinter
void PathExpressionPrinter::print(const PathExpression &pe) {
  os << "(";
  print(pe.getPathEndpoint(0));
  os << ",";
  print(pe.getPathEndpoint(1));
  os << ") | ";
  pe.accept(this);
}

void PathExpressionPrinter::print(const Var &v) {
  std::string name;
  if (names.find(v) != names.end()) {
    name = names.at(v);
  }
  else {
    if (v.getName() != "") {
      name = nameGenerator.getName(v.getName());
    }
    else {
      name = nameGenerator.getName("e");
    }
    names[v] = name;
  }
  os << name;
}

void PathExpressionPrinter::visit(const Var &v) {
}

void PathExpressionPrinter::visit(const Link *pe) {
  print(rename(pe->getLhs()));
  os << "-";
  print(rename(pe->getRhs()));
}

void PathExpressionPrinter::printConnective(const QuantifiedConnective *pe) {
  auto &qvars = pe->getQuantifiedVars();
  iassert(qvars.size() == 0 || qvars.size() == 1)
      << "only one or zero quantified variables currently supported";
  if (qvars.size() > 0) {
    os << qvars[0].getQuantifier();
    print(qvars[0].getVar());
    os << ".";
  }
}

void PathExpressionPrinter::visit(const QuantifiedAnd *pe) {
  printConnective(pe);
  os << "(";
  pe->getLhs().accept(this);
  os << " \u2227 ";
  pe->getRhs().accept(this);
  os << ")";
}

}}
