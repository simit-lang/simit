#ifndef SIMIT_PATH_EXPRESSIONS_H
#define SIMIT_PATH_EXPRESSIONS_H

#include <memory>
#include <vector>
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
/// This file defines the classes that make up path expressions (EV, VE and
/// Formula) as well as Path Expression visitors.

namespace simit {
namespace pe {
class PathExpressionVisitor;

struct VarContent {
  std::string name;
  mutable long ref = 0;
  friend inline void aquire(VarContent *vc) {++vc->ref;}
  friend inline void release(VarContent *vc) {if (--vc->ref==0) delete vc;}
};

class Var : public util::IntrusivePtr<VarContent> {
public:
  Var();
  explicit Var(std::string name);

  bool defined();
  std::string getName() const;

  friend std::ostream &operator<<(std::ostream&, const Var&);
};


class PathExpressionImpl : public interfaces::Printable {
public:
  virtual ~PathExpressionImpl() {}

  virtual Var getPathEndpoint(unsigned i) const = 0;
  virtual void accept(PathExpressionVisitor *visitor) const = 0;

  mutable long ref = 0;
  friend inline void aquire(PathExpressionImpl *p) {++p->ref;}
  friend inline void release(PathExpressionImpl *p) {if (--p->ref==0) delete p;}
};


class PathExpression : public util::IntrusivePtr<PathExpressionImpl> {
public:
  typedef std::vector<Var> Path;

  PathExpression() : IntrusivePtr() {}
  PathExpression(PathExpressionImpl *impl) : IntrusivePtr(impl) {}

  Var getPathEndpoint(unsigned i) const;

  void accept(PathExpressionVisitor*) const;
};
std::ostream &operator<<(std::ostream&, const PathExpression&);


/// EV are path expression atoms, that connect an edge to its endpoints
class EV : public PathExpressionImpl {
public:
  static PathExpression make(Var E, Var V);

  Var getPathEndpoint(unsigned i) const;
  void accept(PathExpressionVisitor *visitor) const;

private:
  Var E;
  Var V;

  EV(Var E, Var V);
  void print(std::ostream &os) const;
};


/// VE at path expression atoms, that connect an element to the edges it is an
/// endpoint of.
class VE : public PathExpressionImpl {
public:
  static PathExpression make(Var V, Var E);

  Var getPathEndpoint(unsigned i) const;
  void accept(PathExpressionVisitor *visitor) const;

private:
  Var V;
  Var E;

  VE(Var V, Var E);
  void print(std::ostream &os) const;
};


class Formula : public PathExpressionImpl {
public:
  struct PathExpr;
  struct And;
  struct Or;

  class PredicateVisitor {
  public:
    virtual void visit(const PathExpr *) {}
    virtual void visit(const And *) {}
    virtual void visit(const Or *) {}
  };

  struct PredicateImpl : public interfaces::Printable {
    virtual ~PredicateImpl() {}
    virtual void accept(PredicateVisitor *visitor) const = 0;

    mutable long ref = 0;
    friend inline void aquire(PredicateImpl *p) {++p->ref;}
    friend inline void release(PredicateImpl *p) {if (--p->ref==0) delete p;}
  };

  class Predicate : util::IntrusivePtr<PredicateImpl> {
  public:
    Predicate() : util::IntrusivePtr<PredicateImpl>() {}
    Predicate(PredicateImpl *pred) : util::IntrusivePtr<PredicateImpl>(pred) {}
    Predicate(PathExpression pathExpr) : Predicate(PathExpr::make(pathExpr)) {}

    void accept(PredicateVisitor *visitor) const {ptr->accept(visitor);}

    friend std::ostream &operator<<(std::ostream &os, const Predicate &p) {
      return os << *p.ptr;
    }
  };

  struct PathExpr : public PredicateImpl {
    PathExpression pathExpr;

    static Predicate make(const PathExpression &pathExpr) {
      PathExpr *pred = new PathExpr;
      pred->pathExpr = pathExpr;
      return pred;
    }

    void accept(PredicateVisitor *visitor) const {visitor->visit(this);}
    void print(std::ostream &os) const {os << "(" << pathExpr << ")";}
  };

  struct And : public PredicateImpl {
    Predicate l, r;

    static Predicate make(const Predicate &l, const Predicate &r) {
      And *pred = new And;
      pred->l = l;
      pred->r = r;
      return pred;
    }

    void accept(PredicateVisitor *visitor) const {visitor->visit(this);}
    void print(std::ostream &os) const {
      os << "(" << l << " \u2227 " << r << ")";
    }
  };

  struct Or : public PredicateImpl {
    Predicate l, r;

    static Predicate make(const Predicate &l, const Predicate &r) {
      Or *pred = new Or;
      pred->l = l;
      pred->r = r;
      return pred;
    }

    void accept(PredicateVisitor *visitor) const {visitor->visit(this);}
    void print(std::ostream &os) const {
      os << "(" << l << " \u2228 " << r << ")";
    }
  };

  struct QuantifiedVar {
    enum Quantifier { Existential };
    Quantifier quantifier;
    Var var;
    QuantifiedVar(Quantifier quantifier, const Var &var)
        : quantifier(quantifier), var(var) {};

    friend std::ostream &operator<<(std::ostream &os, const QuantifiedVar &q) {
      std::string typeStr;
      switch (q.quantifier) {
        case Existential:
          typeStr = "\u2203";
          break;
      }
      return os << typeStr << q.var;
    }
  };

  static PathExpression make(const std::vector<Var> &freeVars,
                             const std::vector<QuantifiedVar> &quantifiedVars,
                             const Predicate &predicate);

  const Predicate &getPredicate() const {return predicate;}
  const std::vector<QuantifiedVar> &getQuantifiedVars() const {
    return quantifiedVars;
  }

  Var getPathEndpoint(unsigned i) const;
  void accept(PathExpressionVisitor *visitor) const;

private:
  std::vector<Var> freeVars;
  std::vector<QuantifiedVar> quantifiedVars;
  Predicate predicate;

  Formula(const std::vector<Var> &freeVars,
          const std::vector<QuantifiedVar> &quantifiedVars,
          const Predicate &predicate);
  void print(std::ostream &os) const;
};

class PathExpressionVisitor {
public:
  virtual void visit(const EV *) {}
  virtual void visit(const VE *) {}
  virtual void visit(const Formula *) {}
};

}}

#endif
