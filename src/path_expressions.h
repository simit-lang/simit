#ifndef SIMIT_PATH_EXPRESSIONS_H
#define SIMIT_PATH_EXPRESSIONS_H

#include <memory>
#include <vector>
#include <map>
#include <typeinfo>
#include <typeindex>

#include "intrusive_ptr.h"
#include "util/scopedmap.h"
#include "util/name_generator.h"
#include "interfaces/printable.h"
#include "interfaces/comparable.h"
#include "stencils.h"

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
/// PathExpression := Link
///                 | QuantifiedConnective
///
/// QuantifiedConnective := (Var,Var) | [Quantifier Var] Connective
///
/// Connective := PathExpression and PathExpression
///             | PathExpression  or Expression
///
/// Quantifier := exist

namespace simit {
class Set; // TODO REMOVE
namespace pe {
class PathExpressionVisitor;
class PathExpression;

struct SetContent {
  std::string name;

  SetContent(std::string name) : name(name) {}
  mutable long ref = 0;
  friend inline void aquire(const SetContent *v) {++v->ref;}
  friend inline void release(const SetContent *v) {if (--v->ref==0) delete v;}
};

class Set : public util::IntrusivePtr<SetContent> {
public:
  Set() : util::IntrusivePtr<SetContent>() {}

  Set(std::string name) : util::IntrusivePtr<SetContent>(new SetContent(name)){}

  const std::string &getName() const {return ptr->name;}
};


struct VarContent {
  std::string name;
  Set set;
  mutable long ref = 0;
  friend inline void aquire(const VarContent *v) {++v->ref;}
  friend inline void release(const VarContent *v) {if (--v->ref==0) delete v;}
};

/// A path expression variable. Variables correspond to elements in sets, which
/// is specified by binding them in the context of some Link (see Link::bind).
class Var : public util::IntrusivePtr<VarContent> {
public:
  Var() : Var("", Set()) {}

  Var(std::string name, Set set=Set())
      : util::IntrusivePtr<VarContent>(new VarContent) {
    ptr->name = name;
    ptr->set = set;
  }

  const std::string &getName() const {return ptr->name;}
  const Set& getSet() const {return ptr->set;}
};


class QuantifiedVar : public interfaces::Comparable<QuantifiedVar> {
public:
  enum Quantifier { Exist };

  QuantifiedVar(Quantifier quantifier, const Var &var)
      : quantifier(quantifier), var(var) {}

  QuantifiedVar(const std::pair<Quantifier,Var> &QuantifiedVar)
      : quantifier(QuantifiedVar.first), var(QuantifiedVar.second) {}

  Var getVar() const {return var;}
  Quantifier getQuantifier() const {return quantifier;}

private:
  Quantifier quantifier;
  Var var;
};

bool operator==(const QuantifiedVar&, const QuantifiedVar&);
bool operator<(const QuantifiedVar&, const QuantifiedVar&);


class PathExpressionImpl {
public:
  virtual ~PathExpressionImpl() {}

  unsigned getNumPathEndpoints() const {return 2;}
  virtual const Var &getPathEndpoint(unsigned i) const = 0;

  Set getSet(const Var &var) const;
  std::map<Var,Set> getSets() const;

  virtual void accept(PathExpressionVisitor *visitor) const = 0;

  friend bool operator==(const PathExpressionImpl&, const PathExpressionImpl&);
  friend bool operator<(const PathExpressionImpl&, const PathExpressionImpl&);

  mutable long ref = 0;
  friend inline void aquire(const PathExpressionImpl *p) {++p->ref;}
  friend inline void release(const PathExpressionImpl *p) {
    if (--p->ref==0) delete p;
  }

private:
  virtual bool eq(const PathExpressionImpl &o) const = 0;
  virtual bool lt(const PathExpressionImpl &o) const = 0;
};


class PathExpression : public util::IntrusivePtr<const PathExpressionImpl> {
public:
  PathExpression() : IntrusivePtr() {}
  PathExpression(const PathExpressionImpl *impl) : IntrusivePtr(impl) {}

  // TODO REMOVE
  void bind(const simit::Set &lhsBinding, const simit::Set &rhsBinding){}

  /// Returns the binding set of `var`, or nullptr if it could not be found.
  Set getSet(const Var &var) const;

  /// Returns a map from Vars to set bindings;
  std::map<Var,Set> getSets() const;

  unsigned getNumPathEndpoints() const {return ptr->getNumPathEndpoints();}
  const Var &getPathEndpoint(unsigned i) const {return ptr->getPathEndpoint(i);}

  void accept(PathExpressionVisitor*) const;

  PathExpression operator()(Var v0, Var v1);

  /// Two path expressions are equal if they have the same form and the same
  /// path variables.
  friend bool operator==(const PathExpression& l, const PathExpression& r) {
    if (l.defined() ^ r.defined()) return false;
    return (l.ptr == r.ptr) || (*l.ptr == *r.ptr);
  }

  friend bool operator<(const PathExpression& l, const PathExpression& r) {
    if ((l.defined() ^ r.defined()) || (!l.defined() && !r.defined())) {
      return false;
    }
    return (l.ptr != r.ptr) && *l.ptr < *r.ptr;
  }

  friend bool operator!=(const PathExpression& lhs, const PathExpression& rhs) {
    return !(lhs == rhs);
  }

  friend bool operator>(const PathExpression& lhs, const PathExpression& rhs) {
    return !(lhs < rhs || lhs == rhs);
  }

  friend bool operator<=(const PathExpression& lhs, const PathExpression& rhs) {
    return !(lhs > rhs);
  }

  friend bool operator>=(const PathExpression& lhs, const PathExpression& rhs) {
    return !(lhs < rhs);
  }
};


template <typename T>
inline bool isa(PathExpression pe) {
  return pe.defined() && dynamic_cast<const T*>(pe.ptr) != nullptr;
}

template <typename T>
inline const T* to(PathExpression e) {
  iassert(isa<T>(e)) << "Wrong PathExpression type";
  return static_cast<const T*>(e.ptr);
}


/// A link is a logical predicate that maps two set elements to true if one of
/// set elements is an endpoint of the other.
class Link : public PathExpressionImpl {
public:
  enum Type {ev, ve, vv};

  static PathExpression make(const Var &lhs, const Var &rhs, Type type,
                             ir::StencilLayout stencil=ir::StencilLayout());

  Type getType() const {return type;}
  bool hasStencil() const {return stencil.defined();}

  const Var &getLhs() const {return lhs;}
  const Var &getRhs() const {return rhs;}

  const Var &getVertexVar(int i=0) const {
    iassert((type != vv && i < 1) || i < 2);
    if (type != vv) return (type==ev) ? rhs : lhs;
    else return (i==0) ? lhs : rhs;
  }
  const Var &getEdgeVar() const {
    iassert(type != vv);
    return (type==ev) ? lhs : rhs;
  }

  const Var &getPathEndpoint(unsigned i) const;

  const ir::StencilLayout &getStencil() const {return stencil;}

  void bind(Set lhsBinding, Set rhsBinding) const;

  Set getLhsSet() const;
  Set getRhsSet() const;

  Set getVertexSet(int i=0) const;
  Set getEdgeSet() const;

  void accept(PathExpressionVisitor *visitor) const;

private:
  Type type;
  Var lhs;
  Var rhs;
  ir::StencilLayout stencil;

  Link(const Var &lhs, const Var &rhs, Type type, ir::StencilLayout stencil);

  bool eq(const PathExpressionImpl &o) const;
  bool lt(const PathExpressionImpl &o) const;
};


/// A quantified connective combines a logical connective with one or more
/// quantified variables. Connectives and quantifications are combined in
/// the same path expression class for convenienec, since they must be
/// evaluated together for efficiency.
class QuantifiedConnective : public PathExpressionImpl {
public:
  bool isQuantified() const {return quantifiedVars.size() > 0;}

  const std::vector<Var> &getFreeVars() const {return freeVars;}

  const std::vector<QuantifiedVar> &getQuantifiedVars() const {
    return quantifiedVars;
  }
  
  PathExpression getLhs() const {return lhs;}
  PathExpression getRhs() const {return rhs;}

  const Var &getPathEndpoint(unsigned i) const;

protected:
  QuantifiedConnective(const std::vector<Var> &freeVars,
                       const std::vector<QuantifiedVar> &quantifiedVars,
                       const PathExpression &lhs,
                       const PathExpression &rhs);

private:
  std::vector<Var> freeVars;
  std::vector<QuantifiedVar> quantifiedVars;

  // The free and quantified variables in the connective rename variables
  // in the sub-expressions.
  std::map<Var, std::vector<Var>> renames;

  PathExpression lhs, rhs;

  bool eq(const PathExpressionImpl &o) const;
  bool lt(const PathExpressionImpl &o) const;
};


/// A logical conjunction that is quantified by one or more variables (see
/// QuantifiedConnective).
class And : public QuantifiedConnective {
public:
  static PathExpression make(const std::vector<Var> &freeVars,
                             const std::vector<QuantifiedVar> &quantifiedVars,
                             const PathExpression &lhs,
                             const PathExpression &rhs);

  void accept(PathExpressionVisitor *visitor) const;

private:
  And(const std::vector<Var> &freeVars,
      const std::vector<QuantifiedVar> &quantifiedVars,
      const PathExpression &lhs, const PathExpression &rhs)
      : QuantifiedConnective(freeVars, quantifiedVars, lhs, rhs) {}
};


/// A logical disjunction that is quantified by one or more variables (see
/// QuantifiedConnective).
class Or : public QuantifiedConnective {
public:
  static PathExpression make(const std::vector<Var> &freeVars,
                             const std::vector<QuantifiedVar> &quantifiedVars,
                             const PathExpression &lhs,
                             const PathExpression &rhs);

  void accept(PathExpressionVisitor *visitor) const;

private:
  Or(const std::vector<Var> &freeVars,
      const std::vector<QuantifiedVar> &quantifiedVars,
      const PathExpression &lhs, const PathExpression &rhs)
      : QuantifiedConnective(freeVars, quantifiedVars, lhs, rhs) {}
};


class RenamedPathExpression : public PathExpressionImpl {
public:
  const Var &getPathEndpoint(unsigned i) const;

  const PathExpression &getPathExpression() const {return pe;}
  const std::map<Var,Var> &getRenames() const {return renames;}

  void accept(PathExpressionVisitor *visitor) const;

private:
  PathExpression pe;
  std::map<Var,Var> renames;

  // Path expressions can be renamed by using PathExpression::operator()
  RenamedPathExpression(const PathExpression &pe,
                        const std::map<Var,Var> &renames)
      : pe(pe), renames(renames) {}
  friend PathExpression;

  bool eq(const PathExpressionImpl &o) const;
  bool lt(const PathExpressionImpl &o) const;
};


class PathExpressionVisitor {
public:
  virtual ~PathExpressionVisitor() {}

  virtual void visit(const Link *pe);
  virtual void visit(const And *pe);
  virtual void visit(const Or *pe);

  virtual void visit(const RenamedPathExpression *pe);

  /// Records renamings and delegates to regular visit method. Called by
  /// RenamedPathExpression::accept().
  virtual void visitRename(const RenamedPathExpression *pe);

protected:
  const Var &rename(const Var &var) const;

private:
  util::ScopedMap<Var,Var> renames;
};


class PathExpressionRewriter : public PathExpressionVisitor {
public:
  virtual ~PathExpressionRewriter() {}

  virtual PathExpression rewrite(PathExpression e);

protected:
  using PathExpressionVisitor::visit;

  Var var;
  PathExpression expr;

  virtual void visit(const Link *pe);
  virtual void visit(const And *pe);
  virtual void visit(const Or *pe);

  virtual void visit(const RenamedPathExpression *pe);
};


class PathExpressionPrinter : public PathExpressionVisitor {
public:
  static const std::string ELEMENTOF;
  static const std::string OR;
  static const std::string AND;
  static const std::string EXIST;

  PathExpressionPrinter(std::ostream &os) : os(os) {}
  virtual ~PathExpressionPrinter() {}

  void print(const Var &v);
  void print(const QuantifiedVar &v);
  void print(const PathExpression &pe);

protected:
  std::ostream &os;
  util::NameGenerator nameGenerator;
  std::map<Var,std::string> names;
  std::map<Set,std::string> setNames;

  virtual void visit(const Link *pe);
  virtual void visit(const And *pe);
  virtual void visit(const Or *pe);

  void printConnective(const QuantifiedConnective *pe);
  void print(Set binding);
  void print(const ir::StencilLayout &s);
};

std::ostream &operator<<(std::ostream&, const Set&);
std::ostream &operator<<(std::ostream&, const Var&);
std::ostream &operator<<(std::ostream&, const PathExpressionImpl&);
std::ostream &operator<<(std::ostream &os, const PathExpression &pe);

}}

#endif
