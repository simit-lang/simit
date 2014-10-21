#ifndef SIMIT_SIG_H
#define SIMIT_SIG_H

#include <vector>
#include <map>
#include <set>
#include <list>
#include <iostream>

#include "types.h"
#include "indexvar.h"
#include "ir.h"

namespace simit {
namespace ir {

struct SIGEdge;
class SIGVisitor;

struct SIGVertex {
  IndexVar iv;
  std::vector<SIGEdge*> connectors;

  SIGVertex(const IndexVar &iv) : iv(iv) {}
};
std::ostream &operator<<(std::ostream &os, const SIGVertex &);

struct SIGEdge {
  Expr edgeSet;
  std::vector<SIGVertex*> endpoints;

  SIGEdge(Expr edgeSet, const std::vector<SIGVertex*> &endpoints)
      : edgeSet(edgeSet), endpoints(endpoints) {
    for (auto v : endpoints) {
      v->connectors.push_back(this);
    }
  }
};
std::ostream &operator<<(std::ostream &os, const SIGEdge &);


/// Implementation of Sparse Iteration Graphs.
class SIG {
public:
  enum MergeOp { Union, Intersection };

  SIG() : content(new SIG::Content) {}
  explicit SIG(const std::vector<IndexVar> &ivs, Expr setExpr=Expr());

  friend SIG merge(SIG&, SIG&, SIG::MergeOp);

private:
  struct Content {
    std::map<IndexVar, std::unique_ptr<SIGVertex>> vertices;
    std::map<Expr, std::unique_ptr<SIGEdge>> edges;
  };
  std::shared_ptr<Content> content;

  friend SIGVisitor;
};

std::ostream &operator<<(std::ostream &os, const SIG &);


class SIGVisitor {
public:
  virtual void apply(const SIG &sig);

protected:
  virtual void visit(const SIGVertex *v);
  virtual void visit(const SIGEdge *e);

  std::set<const SIGVertex*> visitedVertices;
  std::set<const SIGEdge*> visitedEdges;

  std::list<const SIGVertex*> vertexPath;
  std::list<const SIGEdge*> edgePath;

  const SIGVertex *getPreviousVertex();
  const SIGEdge *getPreviousEdge();
};

}} // namespace
#endif
