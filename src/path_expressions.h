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


class ElementVar : public interfaces::Comparable<ElementVar> {
public:
  ElementVar();
  explicit ElementVar(std::string setName);

  bool defined();
  std::string getSetName() const;

  friend bool operator==(const ElementVar&, const ElementVar&);
  friend bool operator<(const ElementVar&, const ElementVar&);
  friend std::ostream &operator<<(std::ostream&, const ElementVar&);

private:
  struct ElementVarContent;
  std::shared_ptr<ElementVarContent> content;

  bool eq(const ElementVar &other) {return content == other.content;}
  bool le(const ElementVar &other) {return content < other.content;}
};


class PathExpressionImpl : public interfaces::Printable {
public:
  virtual ~PathExpressionImpl() {}

  virtual ElementVar getPathEndpoint(unsigned pathEndpoint) const = 0;
  virtual void accept(PathExpressionVisitor *visitor) const = 0;

  mutable long ref = 0;
  friend inline void aquire(PathExpressionImpl *p) {++p->ref;}
  friend inline void release(PathExpressionImpl *p) {if (--p->ref==0) delete p;}
};


class PathExpression : public util::IntrusivePtr<PathExpressionImpl> {
public:
  typedef std::vector<ElementVar> Path;

  PathExpression() : IntrusivePtr() {}
  PathExpression(PathExpressionImpl *impl) : IntrusivePtr(impl) {}

  ElementVar getPathEndpoint(unsigned pathEndpoint) const;

  void accept(PathExpressionVisitor*) const;
};
std::ostream &operator<<(std::ostream&, const PathExpression&);


class EV : public PathExpressionImpl {
public:
  static PathExpression make(ElementVar E, ElementVar V, unsigned edgeEndpoint);

  ElementVar getPathEndpoint(unsigned pathEndpoint) const;
  void accept(PathExpressionVisitor *visitor) const;

private:
  ElementVar E;
  ElementVar V;
  unsigned edgeEndpoint;

  EV(ElementVar E, ElementVar V, unsigned edgeEndpoint);
  void print(std::ostream &os) const;
};


class Predicate : public PathExpressionImpl {
public:
  Predicate();

  ElementVar getPathEndpoint(unsigned pathEndpoint) const;
  void accept(PathExpressionVisitor *visitor) const;

private:
  void print(std::ostream &os) const;
};


class PathExpressionVisitor {
public:
  virtual void visit(const EV *) {};
  virtual void visit(const Predicate *) {};
};

}}

#endif
