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
  static LoopVars create(const SIG &sig) {
    class LoopVarsBuilder : private SIGVisitor {
    public:
      LoopVars build(const SIG &sig) {
        loopVars.clear();
        apply(sig);
        return loopVars;
      }
    private:
      std::map<IndexVar,LoopVar> loopVars;
      UniqueNameGenerator names;
      void visit(const SIGVertex *v) {
        Var var(names.getName(v->iv.getName()), Int);
        ForDomain domain = v->iv.getDomain().getIndexSets()[0];
        LoopVar lvar(var, domain);
        loopVars.insert(std::pair<IndexVar,LoopVar>(v->iv, lvar));
      }
    };
    return LoopVarsBuilder().build(sig);
  }

  class Iterator : public std::iterator<std::forward_iterator_tag, int> {
  public:
    Iterator(std::map<IndexVar,LoopVar>::const_iterator it) : it(it) {}
    Iterator& operator++() { ++it; return *this;}
    Iterator operator++(int) {Iterator tmp(*this); operator++(); return tmp;}
    bool operator!=(const Iterator& rhs) {return it != rhs.it;}
    const LoopVar& operator*() {return (*it).second; }
  private:
    std::map<IndexVar,LoopVar>::const_iterator it;
  };

  Iterator begin() const { return Iterator(loopVars.begin()); }
  Iterator end() const { return Iterator(loopVars.end()); }

private:
  std::map<IndexVar,LoopVar> loopVars;
  LoopVars(std::map<IndexVar,LoopVar> loopVars) : loopVars(loopVars) {}
};


std::ostream &operator<<(std::ostream &os, const SIGVertex &);
std::ostream &operator<<(std::ostream &os, const SIGEdge &);
std::ostream &operator<<(std::ostream &os, const SIG &);
std::ostream &operator<<(std::ostream &os, const LoopVars &lvs);

}} // namespace
#endif
