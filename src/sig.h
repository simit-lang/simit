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
#include "usedef.h"
#include "storage.h"
#include "ir_builder.h"

namespace simit {
namespace ir {

struct SIGEdge;
class SIGVisitor;

/// A vertex in a sparse iteration graph represents index variables.
struct SIGVertex {
  IndexVar iv;
  std::vector<SIGEdge*> connectors;

  SIGVertex(const IndexVar &iv) : iv(iv) {}
};

/// An edge in a sparse iteration graph represents restrictions on the iteration
/// domain of two or more index variables. These restrictions make the iteration
/// space sparse and occur when two or more iteration variables are connected
/// by a sparse matrix.
struct SIGEdge {
  Var tensor;
  std::vector<SIGVertex*> endpoints;

  SIGEdge(Var tensor, const std::vector<SIGVertex*> &endpoints)
      : tensor(tensor), endpoints(endpoints) {
    for (auto v : endpoints) {
      v->connectors.push_back(this);
    }
  }
};

/// Implementation of Sparse Iteration Graphs.
class SIG {
public:
  enum MergeOp { Union, Intersection };

  SIG() : content(new SIG::Content) {}
  explicit SIG(const std::vector<IndexVar> &ivs, Var tensor=Var());

  bool isSparse() const {return content->edges.size() > 0;}
  std::vector<const SIGEdge *> getEdges() const;

  friend SIG merge(SIG&, SIG&, SIG::MergeOp);

private:
  struct Content {
    std::map<IndexVar, std::unique_ptr<SIGVertex>> vertices;
    std::map<Var, std::unique_ptr<SIGEdge>> edges;
  };
  std::shared_ptr<Content> content;

  friend SIGVisitor;
};

/// Visitor class for Sparse Iteration Graphs.
class SIGVisitor {
public:
  virtual void apply(const SIG &sig);

protected:
  virtual void visit(const SIGVertex *v);
  virtual void visit(const SIGEdge *e);

  std::set<const SIGVertex*> visitedVertices;
  std::set<const SIGEdge*> visitedEdges;
};

/// Create a Sparse Iteration Graph for the given statement
SIG createSIG(Stmt stmt, const Storage &storage);

/// Container for the loop variables created from a Sparse Iteration Graph (SIG)
class LoopVars {
public:
  typedef std::vector<LoopVar>::const_iterator Iterator;

  static LoopVars create(const SIG &sig);

  const LoopVar &getLoopVar(const IndexVar &var) const {
    return vertexLoopVars.at(var);
  }

  bool contains(const IndexVar &var) const {
    return vertexLoopVars.find(var) != vertexLoopVars.end();
  }

  Iterator begin() const { return Iterator(loopVars.begin()); }
  Iterator end() const { return Iterator(loopVars.end()); }

private:
  std::vector<LoopVar> loopVars;
  std::map<IndexVar,LoopVar> vertexLoopVars;

  LoopVars(const std::vector<LoopVar> &, const std::map<IndexVar,LoopVar> &);
};


std::ostream &operator<<(std::ostream &os, const SIGVertex &);
std::ostream &operator<<(std::ostream &os, const SIGEdge &);
std::ostream &operator<<(std::ostream &os, const SIG &);
std::ostream &operator<<(std::ostream &os, const LoopVars &lvs);

}} // namespace
#endif
