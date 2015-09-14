#include "path_expressions.h"

#include <string>
#include <map>
#include <vector>
#include <algorithm>

#include "graph.h"
#include "util/collections.h"
#include "error.h"

using namespace std;

namespace simit {
namespace pe {

// class QuantifiedVar
bool operator==(const QuantifiedVar& l, const QuantifiedVar& r) {
  return l.getQuantifier() == r.getQuantifier();
}

bool operator<(const QuantifiedVar& l, const QuantifiedVar&r ) {
  return l.getQuantifier() < r.getQuantifier();
}

// class PathExpressionImpl
bool PathExpressionImpl::isBound() const {
#ifdef SIMIT_ASSERTS
  /// Check invariant: either all or none are bound
  class CheckThatAllOrNoneAreBound : public PathExpressionVisitor {
  public:
    enum AllOrNoneBoundState { Unknown, None, All };
    AllOrNoneBoundState allOrNoneBoundState = Unknown;

    void visit(const Link *link) {
      switch (allOrNoneBoundState) {
        case Unknown:
          allOrNoneBoundState = link->isBound() ? All : None;
          break;
        case None:
          iassert(!link->isBound())
              << "Some but not all variables in the PathExpression are bound.\n"
              << *link << " is bound";
          break;
        case All:
          iassert(link->isBound())
              << "Some but not all variables in the PathExpression are bound.\n"
              << *link << " is not bound";
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
    bool check(const PathExpressionImpl *pe) {
      pe->accept(this);
      return isBound;
    }
    void visit(const Link *link) {
      isBound = link->isBound();
    }
  };
  return CheckIfBound().check(this);
}

const simit::Set *PathExpressionImpl::getBinding(const Var &var) const {
  iassert(isBound())
      << "attempting to get the binding of a var in an unbound path expression";
  auto bindings = getBindings();
  return util::contains(bindings,var) ? bindings.at(var) : nullptr;
}

map<Var, const simit::Set*> PathExpressionImpl::getBindings() const {
  iassert(isBound())
      << "attempting to get the bindings of an unbound path expression";
  class GetBindings : public PathExpressionVisitor {
  public:
    map<Var, const simit::Set*> find(const PathExpressionImpl *pe) {
      bindings.clear();
      pe->accept(this);
      return bindings;
    }

  private:
    map<Var, const simit::Set*> bindings;

    void visit(const Link *link) {
      Var lhs = rename(link->getLhs());
      Var rhs = rename(link->getRhs());
      const simit::Set *lhsBinding = link->getLhsBinding();
      const simit::Set *rhsBinding = link->getRhsBinding();
      bindings.insert({lhs, lhsBinding});
      bindings.insert({rhs, rhsBinding});
    }
  };
  return GetBindings().find(this);
}

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

// class PathExpression
void PathExpression::bind(const simit::Set &lhsBinding, const simit::Set &rhsBinding) {
  iassert(isa<Link>(*this)) << "bind is only defined for Link PathExpressions";
  class BindPathExpression : public PathExpressionVisitor {
  public:
    void bind(const PathExpression &pe, const simit::Set &lhs,
              const simit::Set &rhs) {
      this->lhs = &lhs;
      this->rhs = &rhs;
      pe.accept(this);
    }
  private:
    const simit::Set *lhs, *rhs;
    void visit(const Link *link) {
      link->bind(lhs, rhs);
    }
  };
  BindPathExpression().bind(*this,lhsBinding,rhsBinding);
}

bool PathExpression::isBound() const {
  return ptr->isBound();
}

const simit::Set* PathExpression::getBinding(const Var &var) const {
  return ptr->getBinding(var);
}

std::map<Var, const simit::Set*> PathExpression::getBindings() const {
  return ptr->getBindings();
}

void PathExpression::accept(PathExpressionVisitor *visitor) const {
  ptr->accept(visitor);
}

PathExpression PathExpression::operator()(Var v0, Var v1) {
  return new RenamedPathExpression(*this, {{getPathEndpoint(0),v0},
                                   {getPathEndpoint(1),v1}});
}


// class Link
Link::Link(const Var &lhs, const Var &rhs, Type type)
    : type(type), lhs(lhs), rhs(rhs) {
}

PathExpression Link::make(const Var &lhs, const Var &rhs, Type type) {
  return new Link(lhs, rhs, type);
}

void Link::bind(const simit::Set *lhsBinding, const simit::Set *rhsBinding) const {
  this->lhs.getSet().bind(lhsBinding);
  this->rhs.getSet().bind(rhsBinding);
}

const simit::Set* Link::getLhsBinding() const {
  return lhs.getSet().getBinding();
}

const simit::Set* Link::getRhsBinding() const {
  return rhs.getSet().getBinding();
}

const simit::Set* Link::getVertexBinding() const {
  return (type==ev) ? rhs.getSet().getBinding() : lhs.getSet().getBinding();
}

const simit::Set* Link::getEdgeBinding() const   {
  return (type==ev) ? lhs.getSet().getBinding() : rhs.getSet().getBinding();
}

bool Link::isBound() const {
  iassert((lhs.getSet().isBound() && rhs.getSet().isBound()) ||
          (!lhs.getSet().isBound() && !rhs.getSet().isBound()))
      << "either all should be bound or none";
  return lhs.getSet().isBound();
}

const Var &Link::getPathEndpoint(unsigned i) const {
  iassert(i < 2) << "attempting to retrieve non-existing path endpoint";
  return (i==0) ? lhs : rhs;
}

void Link::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}

bool Link::eq(const PathExpressionImpl &o) const {
  auto optr = static_cast<const Link*>(&o);
  return this->getLhs().getSet() == optr->getLhs().getSet() &&
         this->getRhs().getSet() == optr->getRhs().getSet();
}

bool Link::lt(const PathExpressionImpl &o) const {
  auto optr = static_cast<const Link*>(&o);
  return (getLhs().getSet() != optr->getLhs().getSet())
         ? getLhs().getSet() < optr->getLhs().getSet()
         : getRhs().getSet() < optr->getRhs().getSet();
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
  iassert(quantifiedVars.size() < 2)
      << "For now, we only support one quantified variable";
}

const Var &QuantifiedConnective::getPathEndpoint(unsigned i) const {
  return freeVars[i];
}

bool QuantifiedConnective::eq(const PathExpressionImpl &o) const {
  auto optr = static_cast<const QuantifiedConnective*>(&o);
  for (auto qvars : util::zip(getQuantifiedVars(), optr->getQuantifiedVars())) {
    if (qvars.first != qvars.second) {
      return false;
    }
  }
  return getLhs() == optr->getLhs() && getRhs() == optr->getRhs();
}

bool QuantifiedConnective::lt(const PathExpressionImpl &o) const {
  auto optr = static_cast<const QuantifiedConnective*>(&o);
  if (getLhs() != optr->getLhs()) {
    return getLhs() < optr->getLhs();
  }
  else if (getRhs() != optr->getRhs()) {
    return getRhs() < optr->getRhs();
  }
  else {
    if (getQuantifiedVars().size() != optr->getQuantifiedVars().size()) {
      return getQuantifiedVars().size() < optr->getQuantifiedVars().size();
    }
    for (auto qv : util::zip(getQuantifiedVars(), optr->getQuantifiedVars())) {
      if (qv.first != qv.second) {
        return qv.first < qv.second;
      }
    }
    return false;
  }
}


// class And
PathExpression And::make(const std::vector<Var> &freeVars,
                         const vector<QuantifiedVar> &quantifiedVars,
                         const PathExpression &lhs,
                         const PathExpression &rhs) {
  return new And(freeVars, quantifiedVars, lhs, rhs);
}

void And::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}


// class Or
PathExpression Or::make(const std::vector<Var> &freeVars,
                        const std::vector<QuantifiedVar> &quantifiedVars,
                        const PathExpression &lhs,
                        const PathExpression &rhs) {
  return new Or(freeVars, quantifiedVars, lhs, rhs);
}

void Or::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}


// class RenamedPathExpression
const Var &RenamedPathExpression::getPathEndpoint(unsigned i) const {
  iassert(i < pe.getNumPathEndpoints())
      << "path expression does not have requested endpoint";
  iassert(renames.find(pe.getPathEndpoint(i)) != renames.end())
      << "no mapping exist for endpoint " << pe.getPathEndpoint(i);
  iassert(util::contains(renames, pe.getPathEndpoint(i)));
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
void PathExpressionVisitor::visit(const Link *pe) {
}

void PathExpressionVisitor::visit(const And *pe) {
  pe->getLhs().accept(this);
  pe->getRhs().accept(this);
}

void PathExpressionVisitor::visit(const Or *pe) {
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
  std::map<Var,Var> newRenames;
  for (auto &rename : pe->getRenames()) {
    // If v0 is renamed to a v1, which is renamed to v2, then rename v0 to v2
    if (renames.contains(rename.second)) {
      newRenames.insert({rename.first, renames.get(rename.second)});
    }
    else {
      newRenames.insert(rename);
    }
  }
  renames.scope(newRenames);
  visit(pe);  // template method
  renames.unscope();
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
  var = Var();
  expr = PathExpression();
  return e;
}

void PathExpressionRewriter::visit(const Link *pe) {
  expr = pe;
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

void PathExpressionRewriter::visit(const Or *pe) {
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
void PathExpressionPrinter::print(const Var &v) {
  std::string name;
  if (util::contains(names, v)) {
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

void PathExpressionPrinter::print(const simit::Set *binding) {
  os << ELEMENTOF;
  string setName;
  if (binding->getName() != "") {
    setName = binding->getName();
  }
  else {
    if (util::contains(setNames, binding)) {
      setName = setNames.at(binding);
    }
    else {
      setName = nameGenerator.getName("S");
      setNames.insert({binding,setName});
    }
  }
  os << setName;
}

void PathExpressionPrinter::print(const QuantifiedVar &v) {
  switch (v.getQuantifier()) {
    case QuantifiedVar::Quantifier::Exist:
      os << EXIST;
      break;
  }
  print(v.getVar());
}

void PathExpressionPrinter::print(const PathExpression &pe) {
  os << "(";
  if (!pe.isBound()) {
    unsigned numFreeVars = pe.getNumPathEndpoints();
    for (unsigned i=0; i<numFreeVars; ++i) {
      print(pe.getPathEndpoint(i));
      if (i < numFreeVars-1) {
        os << ",";
      }
    }
  }
  else {
    auto bindings = pe.getBindings();
    unsigned numFreeVars = pe.getNumPathEndpoints();
    for (unsigned i=0; i<numFreeVars; ++i) {
      Var ep = pe.getPathEndpoint(i);
      iassert(util::contains(bindings, ep))
          << "no binding for " << ep << " (" << ep.ptr << ")";
      print(ep);
      print(bindings.at(ep));
      if (i < numFreeVars-1) {
        os << ", ";
      }
    }
  }
  os << ") | ";
  pe.accept(this);
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
    if (!pe->isBound()) {
      unsigned numQuantifiedVars = qvars.size();
      for (unsigned i=0; i < numQuantifiedVars; ++i) {
        auto &qvar = qvars[i];
        print(qvar);
        if (i < numQuantifiedVars-1) {
          os << ",";
        }
      }
    }
    else {
      auto bindings = pe->getBindings();
      unsigned numQuantifiedVars = qvars.size();
      for (unsigned i=0; i<numQuantifiedVars; ++i) {
        auto &qvar = qvars[i];
        iassert(util::contains(bindings, qvar.getVar()))
            << "no binding for " << qvar.getVar();
        print(qvar);
        print(bindings.at(qvar.getVar()));
        if (i < numQuantifiedVars-1) {
          os << ", ";
        }
      }
    }
    os << ".";
  }
}

void PathExpressionPrinter::visit(const And *pe) {
  printConnective(pe);
  os << "(";
  pe->getLhs().accept(this);
  os << " " << OR << " ";
  pe->getRhs().accept(this);
  os << ")";
}

void PathExpressionPrinter::visit(const Or *pe) {
  printConnective(pe);
  os << "(";
  pe->getLhs().accept(this);
  os << " " << AND << " ";
  pe->getRhs().accept(this);
  os << ")";
}

std::ostream &operator<<(std::ostream& os, const Var& v) {
  iassert(v.defined()) << "attempting to print undefined var";
  return os << v.getName();
}

std::ostream &operator<<(std::ostream &os, const PathExpressionImpl &pe) {
  PathExpressionPrinter(os).print(&pe);
  return os;
}

std::ostream &operator<<(std::ostream &os, const PathExpression &pe) {
  if (pe.defined()) {
    PathExpressionPrinter(os).print(pe);
  }
  else {
    os << "Undefined PathExpression";
  }
  return os;
}

}}
