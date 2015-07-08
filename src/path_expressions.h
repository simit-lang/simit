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
/// This file defines the classes that make up path expressions (EV and
/// Predicate) as well as Path Expression visitors.

namespace simit {
namespace pe {
class PathExpressionVisitor;

struct VarContent {
  std::string setName;
  mutable long ref = 0;
  friend inline void aquire(VarContent *vc) {++vc->ref;}
  friend inline void release(VarContent *vc) {if (--vc->ref==0) delete vc;}
};

class Var : public util::IntrusivePtr<VarContent> {
public:
  Var();
  explicit Var(std::string setName);

  bool defined();
  std::string getSetName() const;

  friend std::ostream &operator<<(std::ostream&, const Var&);
};


class PathExpressionImpl : public interfaces::Printable {
public:
  virtual ~PathExpressionImpl() {}

  virtual Var getPathEndpoint(unsigned pathEndpoint) const = 0;
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

  Var getPathEndpoint(unsigned pathEndpoint) const;

  void accept(PathExpressionVisitor*) const;
};
std::ostream &operator<<(std::ostream&, const PathExpression&);


/// EV are path expression atoms, that connect an edge to its endpoints
class EV : public PathExpressionImpl {
public:
  static PathExpression make(Var E, Var V);

  Var getPathEndpoint(unsigned pathEndpoint) const;
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

  Var getPathEndpoint(unsigned pathEndpoint) const;
  void accept(PathExpressionVisitor *visitor) const;

private:
  Var V;
  Var E;

  VE(Var V, Var E);
  void print(std::ostream &os) const;
};


class Predicate : public PathExpressionImpl {
public:
  Predicate();

  Var getPathEndpoint(unsigned pathEndpoint) const;
  void accept(PathExpressionVisitor *visitor) const;

private:
  void print(std::ostream &os) const;
};


class PathExpressionVisitor {
public:
  virtual void visit(const EV *) {}
  virtual void visit(const VE *) {}
  virtual void visit(const Predicate *) {}
};

}}

#endif
