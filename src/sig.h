#ifndef SIMIT_SIG_H
#define SIMIT_SIG_H
#include <algorithm>
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
  Var tensor;  // We might not need the tensor variable here
  Expr set;

  std::vector<SIGVertex*> endpoints;

  SIGEdge(Var tensor, Expr set, const std::vector<SIGVertex*> &endpoints)
      : tensor(tensor), set(set), endpoints(endpoints) {
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
  SIG(const std::vector<IndexVar> &ivs, Var tensor, Expr set);

  bool isSparse() const {return content->edges.size() > 0;}

  friend SIG merge(SIG&, SIG&, SIG::MergeOp);

private:
  struct Content {
    std::map<IndexVar, std::unique_ptr<SIGVertex>> vertices;
    std::vector<std::unique_ptr<SIGEdge>> edges;
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
  typedef std::vector<LoopVar>::const_reverse_iterator ReverseIterator;

  static LoopVars create(const SIG &sig);

  /// Get the loop variables that correspond to the given index variable. There
  /// are one loop variable per block level in the indexvar domain.
  const std::vector<LoopVar> &getLoopVars(const IndexVar &var) const {
    return vertexLoopVars.at(var);
  }

  bool contains(const IndexVar &var) const {
    return vertexLoopVars.find(var) != vertexLoopVars.end();
  }

  Var getCoordVar(std::vector<Var> coord) const {
    std::sort(coord.begin(), coord.end());
    return (coordVars.find(coord) != coordVars.end())
        ? coordVars.at(coord) : Var();
  }

  Iterator begin() const { return loopVars.begin(); }
  Iterator end() const { return loopVars.end(); }

  ReverseIterator rbegin() const { return loopVars.rbegin(); }
  ReverseIterator rend() const { return loopVars.rend(); }

private:
  std::vector<LoopVar> loopVars;
  std::map<IndexVar, std::vector<LoopVar>> vertexLoopVars;
  std::map<std::vector<Var>, Var> coordVars;

  LoopVars(const std::vector<LoopVar>&,
           const std::map<IndexVar,std::vector<LoopVar>>&,
           const std::map<std::vector<Var>, Var>&);
};


std::ostream &operator<<(std::ostream &os, const SIGVertex &);
std::ostream &operator<<(std::ostream &os, const SIGEdge &);
std::ostream &operator<<(std::ostream &os, const SIG &);
std::ostream &operator<<(std::ostream &os, const LoopVars &lvs);

}} // namespace
#endif
