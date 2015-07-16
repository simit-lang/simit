#ifndef SIMIT_PATH_EXPRESSIONS_H
#define SIMIT_PATH_EXPRESSIONS_H

#include <memory>
#include <vector>
#include <map>
#include <typeinfo>
#include <typeindex>

#include "printable.h"
#include "intrusive_ptr.h"
#include "comparable.h"

/// \file
/// Path Expressions describe a neighborhood of a vertex or edge in a graph.
/// A neighborhood is here defined as the vertices or edges that you can reach
/// through a path expression, starting from a vertex or edge. Path Expressions
/// are typically used to describe what vertices and edges contribute to the new
/// value of a given vertex or edge in a computation. The computation can be the
/// application of a stencil/data-graph kernel, but can also be a matrix-vector
/// multiplication, since the matrix is a linear function that compute new
/// vertex or edge values by (linearly) combining the values from multiple
/// vertices or edges in a neighborhood.
///
/// This file defines the classes that make up path expressions as well as Path
/// Expression visitors. The EBNF for path expressions is:
///
/// PathExpression := EV
///                 | VE
///                 | Predicate
///
/// Formula := (Var,Var) QuantifiedVar | Predicate
///
/// Predicate := PathExpression and PathExpression
///            | PathExpression  or Expression
///
/// QuantifiedVar := exist Var

namespace simit {
class Set;

namespace pe {
class PathExpressionVisitor;

struct VarContent {
  std::string name;
  const Set *set;

  VarContent() : VarContent("") {}
  VarContent(const std::string &name) : VarContent(name, nullptr) {}
  VarContent(const std::string &name, const Set *set) : name(name), set(set) {}

  mutable long ref = 0;
  friend inline void aquire(const VarContent *v) {++v->ref;}
  friend inline void release(const VarContent *v) {if (--v->ref==0) delete v;}
};

class Var : public util::IntrusivePtr<const VarContent> {
public:
  Var();
  explicit Var(const std::string &name);
  explicit Var(const std::string &name, const Set &set);

  const std::string &getName() const;
  const Set *getBinding() const;

  bool isBound() const;

  void accept(PathExpressionVisitor*) const;

  friend std::ostream &operator<<(std::ostream&, const Var&);
};


class PathExpressionImpl : public interfaces::Printable {
public:
  virtual ~PathExpressionImpl() {}

  virtual Var getPathEndpoint(unsigned i) const = 0;
  virtual void accept(PathExpressionVisitor *visitor) const = 0;

  friend bool
  operator==(const PathExpressionImpl &l, const PathExpressionImpl &r) {
    return typeid(l) == typeid(r) && l.eq(r);
  }

  friend bool
  operator<(const PathExpressionImpl &l, const PathExpressionImpl &r) {
    auto lidx = std::type_index(typeid(l));
    auto ridx = std::type_index(typeid(r));
    return (lidx != ridx) ? lidx < ridx : l.lt(r);
  }

  mutable long ref = 0;
  friend inline void aquire(const PathExpressionImpl *p) {++p->ref;}
  friend inline void release(const PathExpressionImpl *p) {
    if (--p->ref==0) delete p;
  }

private:
  virtual bool eq(const PathExpressionImpl &o) const = 0;
  virtual bool lt(const PathExpressionImpl &o) const = 0;
};


class PathExpression
    : public util::IntrusivePtr<const PathExpressionImpl,false>,
      public interfaces::Comparable<PathExpression> {
public:
  typedef std::map<Var,const Set&> Bindings;

  PathExpression() : IntrusivePtr() {}
  PathExpression(const PathExpressionImpl *impl) : IntrusivePtr(impl) {}
  PathExpression(const PathExpressionImpl *impl, const Bindings &bindings);

  PathExpression bind(const Bindings &bindings) const;

  /// True if the variables are bound to sets, false otherwise.
  bool isBound() const;

  Var getPathEndpoint(unsigned i) const;

  void accept(PathExpressionVisitor*) const;

  friend bool operator==(const PathExpression &l, const PathExpression &r) {
    return (l.ptr == r.ptr) || (*l.ptr == *r.ptr);
  }

  friend bool operator<(const PathExpression &l, const PathExpression &r) {
    return (l.ptr != r.ptr) && *l.ptr < *r.ptr;
  }

  friend std::ostream &operator<<(std::ostream &os, const PathExpression &pe) {
    return os << *pe.ptr;
  }
};


/// EV are path expression atoms, that connect an edge to its endpoints
class EV : public PathExpressionImpl {
public:
  static PathExpression make(Var E, Var V);

  const Var &getE() const {return E;}
  const Var &getV() const {return V;}

  Var getPathEndpoint(unsigned i) const;
  void accept(PathExpressionVisitor *visitor) const;

private:
  Var E;
  Var V;

  EV(Var E, Var V);

  bool eq(const PathExpressionImpl &o) const;
  bool lt(const PathExpressionImpl &o) const;

  void print(std::ostream &os) const {
    os << E << "-" << V;
  }
};


/// VE at path expression atoms, that connect an element to the edges it is an
/// endpoint of.
class VE : public PathExpressionImpl {
public:
  static PathExpression make(Var V, Var E);

  const Var &getV() const {return V;}
  const Var &getE() const {return E;}

  Var getPathEndpoint(unsigned i) const;
  void accept(PathExpressionVisitor *visitor) const;

private:
  Var V;
  Var E;

  VE(Var V, Var E);

  bool eq(const PathExpressionImpl &o) const;
  bool lt(const PathExpressionImpl &o) const;

  void print(std::ostream &os) const {
    os << V << "-" << E;
  }
};


class QVar {
public:
  enum Quantifier { Exist };

  QVar(Quantifier quantifier, const Var &var)
      : quantifier(quantifier), var(var) {}

  QVar(const std::pair<Quantifier,Var> &qvar)
      : quantifier(qvar.first), var(qvar.second) {}

  Var getVar() const {return var;}
  Quantifier getQuantifier() const {return quantifier;}

  friend std::ostream &operator<<(std::ostream &os, const QVar &q) {
    std::string typeStr;
    switch (q.getQuantifier()) {
      case QVar::Exist:
        typeStr = "\u2203";
        break;
    }
    return os << typeStr << q.getVar();
  }

private:
  Quantifier quantifier;
  Var var;
};


class Formula : public PathExpressionImpl {
public:
  bool isQuantified() const {return quantifiedVars.size() > 0;}

  const std::vector<Var> &getFreeVars() const {return freeVars;}

  const std::vector<QVar> &getQVars() const {
    return quantifiedVars;
  }

  Var getPathEndpoint(unsigned i) const;

protected:
  Formula(const std::vector<Var> &freeVars,
          const std::vector<QVar> &quantifiedVars);

  void print(std::ostream &os) const;

private:
  std::vector<Var> freeVars;
  std::vector<QVar> quantifiedVars;
};


class And : public Formula {
public:
  static PathExpression make(const std::vector<Var> &freeVars,
                             const std::vector<QVar> &quantifiedVars,
                             const PathExpression &l, const PathExpression &r);

  PathExpression getLhs() const {return l;}
  PathExpression getRhs() const {return r;}

  void accept(PathExpressionVisitor *visitor) const;

private:
  // TODO: Factor fields and eq/lt into BinaryFormula class?
  PathExpression l, r;

  And(const std::vector<Var> &freeVars,
      const std::vector<QVar> &quantifiedVars,
      const PathExpression &l, const PathExpression &r)
      : Formula(freeVars, quantifiedVars), l(l), r(r) {}

  bool eq(const PathExpressionImpl &o) const {
    const And *optr = static_cast<const And*>(&o);
    return l == optr->l && r == optr->r;
  }

  bool lt(const PathExpressionImpl &o) const {
    const And *optr = static_cast<const And*>(&o);
    return (l != optr->l) ? l < optr->l : r < optr->r;
  }

  void print(std::ostream &os) const;
};


class PathExpressionVisitor {
public:
  virtual void visit(const Var &v);
  virtual void visit(const EV *pe);
  virtual void visit(const VE *pe);
  virtual void visit(const And *pe);
};


class PathExpressionRewriter : public PathExpressionVisitor {
public:
  virtual Var rewrite(Var v);
  virtual PathExpression rewrite(PathExpression e);

protected:
  using PathExpressionVisitor::visit;

  Var var;
  PathExpression expr;

  virtual void visit(const Var &v);
  virtual void visit(const EV *pe);
  virtual void visit(const VE *pe);
  virtual void visit(const And *pe);
};

}}

#endif
