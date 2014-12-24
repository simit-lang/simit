#include "sig.h"

#include <algorithm>
#include <iostream>
#include <set>

#include "util.h"

using namespace std;
using namespace simit::util;

namespace simit {
namespace ir {

// class SIG
SIG::SIG(const std::vector<IndexVar> &ivs, Var tensor) : SIG() {
  set<IndexVar> added;
  vector<SIGVertex*> endpoints;

  for (auto &iv : ivs) {
    if (added.find(iv) == added.end()) {
      added.insert(iv);

      SIGVertex *v = new SIGVertex(iv);
      content->vertices[iv] = unique_ptr<SIGVertex>(v);
      endpoints.push_back(v);
    }
  }

  if (tensor.defined()) {
    SIGEdge *e = new SIGEdge(tensor, endpoints);
    content->edges[tensor] = unique_ptr<SIGEdge>(e);
  }
}

std::vector<const SIGEdge *> SIG::getEdges() const {
  std::vector<const SIGEdge *> edges;
  for (auto &edge : content->edges) {
    edges.push_back(edge.second.get());
  }
  return edges;
}

SIG merge(SIG &g1, SIG &g2, SIG::MergeOp mop) {
  // This can be optimized by exploiting immutability to preserve substructures
  SIG merged = SIG();

  for (auto &v : g1.content->vertices) {
    const IndexVar &iv = v.first;
    SIGVertex *newVertex = new SIGVertex(iv);
    merged.content->vertices[iv] = unique_ptr<SIGVertex>(newVertex);
  }

  for (auto &v : g2.content->vertices) {
    const IndexVar &iv = v.first;
    SIGVertex *newVertex = new SIGVertex(iv);
    merged.content->vertices[iv] = unique_ptr<SIGVertex>(newVertex);
  }

  map<Var,vector<SIGVertex*>> edges;

  for (auto &e : g1.content->edges) {
    Var edgeSet = e.first;
    vector<SIGVertex*> endpoints;
    for (SIGVertex *g1v : e.second->endpoints) {
      endpoints.push_back(merged.content->vertices[g1v->iv].get());
    }
    edges[edgeSet] = endpoints;
  }

  for (auto &e : g2.content->edges) {
    Var edgeSet = e.first;
    vector<SIGVertex*> endpoints;
    for (SIGVertex *g1v : e.second->endpoints) {
      endpoints.push_back(merged.content->vertices[g1v->iv].get());
    }

    if (edges.find(edgeSet) == edges.end()) {
      edges[edgeSet] = endpoints;
    }
    else {
      // Vertex identification/contraction
      vector<SIGVertex*> &currEps = edges[edgeSet];
      currEps.insert(currEps.begin(), endpoints.begin(), endpoints.end());
    }
  }

  for (auto &edge : edges) {
    SIGEdge *newEdge = new SIGEdge(edge.first, edge.second);
    merged.content->edges[edge.first] = unique_ptr<SIGEdge>(newEdge);
  }

  return merged;
}

bool ReductionVarsBeforefree(SIGVertex *i, SIGVertex *j) {
  return (i->iv.isFreeVar() && j->iv.isReductionVar()) ? true : false;
}

void SIGVisitor::apply(const SIG &sig) {
  std::vector<SIGEdge*> edgeIterationOrder;
  for (auto &e : sig.content->edges) {
    edgeIterationOrder.push_back(e.second.get());
  }

  std::vector<SIGVertex*> vertexIterationOrder;
  for (auto &v : sig.content->vertices) {
    vertexIterationOrder.push_back(v.second.get());
  }

  // Sort reduction variables before free vars because we do codegen bottom-up
  sort(vertexIterationOrder.begin(), vertexIterationOrder.end(),
       ReductionVarsBeforefree);

  for (SIGEdge *e : edgeIterationOrder) {
    if (visitedEdges.find(e) == visitedEdges.end()) {
      visit(e);
    }
  }

  for (SIGVertex *v : vertexIterationOrder) {
    if (visitedVertices.find(v) == visitedVertices.end()) {
      visit(v);
    }
  }
}

void SIGVisitor::visit(const SIGVertex *v) {
  visitedVertices.insert(v);
  for (auto &e : v->connectors) {
    if (visitedEdges.find(e) == visitedEdges.end()) {
      visit(e);
    }
  }
}

void SIGVisitor::visit(const SIGEdge *e) {
  visitedEdges.insert(e);
  for (auto &v : e->endpoints) {
    if (visitedVertices.find(v) == visitedVertices.end()) {
      visit(v);
    }
  }
}


/// Class that builds a Sparse Iteration Graph from an expression.
class SIGBuilder : public IRVisitor {
public:
  SIGBuilder(const Storage &storage) : storage(storage) {}

  SIG create(Stmt stmt) {
    stmt.accept(this);
    SIG result = sig;
    sig = SIG();
    return result;
  }

private:
  Storage storage;
  SIG sig;

  SIG create(Expr expr) {
    expr.accept(this);
    SIG result = sig;
    sig = SIG();
    return result;
  }

  void visit(const IndexedTensor *op) {
    iassert(!isa<IndexExpr>(op->tensor))
        << "IndexExprs should have been flattened by now:" << toString(*op);

    Var tensorVar;
    if (isa<VarExpr>(op->tensor) && !isScalar(op->tensor.type())) {
      const Var &var = to<VarExpr>(op->tensor)->var;
      iassert(storage.hasStorage(var)) << "No storage descriptor found for"
                                       << var << "in" << util::toString(*op);
      if (storage.get(var).isSystem()) {
        tensorVar = var;
      }
    }
    sig = SIG(op->indexVars, tensorVar);
  }

  void visit(const Add *op) {
    SIG ig1 = create(op->a);
    SIG ig2 = create(op->b);
    sig = merge(ig1, ig2, SIG::Union);
  }

  void visit(const Sub *op) {
    SIG ig1 = create(op->a);
    SIG ig2 = create(op->b);
    sig = merge(ig1, ig2, SIG::Union);
  }

  void visit(const Mul *op) {
    SIG ig1 = create(op->a);
    SIG ig2 = create(op->b);
    sig = merge(ig1, ig2, SIG::Intersection);
  }

  void visit(const Div *op) {
    SIG ig1 = create(op->a);
    SIG ig2 = create(op->b);
    sig = merge(ig1, ig2, SIG::Intersection);
  }
};

SIG createSIG(Stmt stmt, const Storage &storage) {
  return SIGBuilder(storage).create(stmt);
}

/// Get the number of block levels of the index variables in SIG
size_t getNumBlockLevels(const SIG &sig) {
  class GetNumBlockLevelsVisitor : public SIGVisitor {
  public:
    size_t get(const SIG &sig) {
      numBlockLevels = 0;
      apply(sig);
      return numBlockLevels;
    }

  private:
    size_t numBlockLevels;

    void visit(const SIGVertex *v) {
      size_t ivNumBlockLevels = v->iv.getNumBlockLevels();
      if (numBlockLevels < ivNumBlockLevels) {
        numBlockLevels = ivNumBlockLevels;
      }
    }
  };
  return GetNumBlockLevelsVisitor().get(sig);
}


// class LoopVars
LoopVars LoopVars::create(const SIG &sig) {
  class LoopVarsBuilder : private SIGVisitor {
  public:
    LoopVars build(const SIG &sig) {
      vertexLoopVars.clear();

      // We create one set of loop nests per block level in the SIG.
      numBlockLevels = getNumBlockLevels(sig);
      for (currBlockLevel=0; currBlockLevel<numBlockLevels; ++currBlockLevel) {
        apply(sig);
      }

      return LoopVars(loopVars, vertexLoopVars);
    }

  private:
    std::map<IndexVar, std::vector<LoopVar>> vertexLoopVars;
    std::vector<LoopVar> loopVars;
    UniqueNameGenerator nameGenerator;

    /// We create one loop variable per block level per index variable. The loop
    /// variables are ordered by block level. This variable keeps track of which
    /// block level we are currently creating loop variables for.
    size_t currBlockLevel;

    /// The number of block levels in the index variables in the SIG.
    size_t numBlockLevels;

    void visit(const SIGVertex *v) {
      const IndexVar &indexVar = v->iv;

      if (currBlockLevel < indexVar.getNumBlockLevels()) {
        Var var(nameGenerator.getName(indexVar.getName()), Int);
        ForDomain domain = indexVar.getDomain().getIndexSets()[currBlockLevel];

        // We only need to reduce w.r.t. to the outer loop variable variable.
        ReductionOperator rop = (currBlockLevel==0)
                                ? indexVar.getOperator()
                                : ReductionOperator::Undefined;
                                
        addVertexLoopVar(indexVar, LoopVar(var, domain, rop));
      }

      SIGVisitor::visit(v);
    }

    void addVertexLoopVar(const IndexVar &indexVar, const LoopVar &loopVar) {
      loopVars.push_back(loopVar);

      // Add entry for the indexVar
      if (vertexLoopVars.find(indexVar) == vertexLoopVars.end()) {
        vertexLoopVars[indexVar] = std::vector<LoopVar>();
      }
      vertexLoopVars[indexVar].push_back(loopVar);
    }
  }; // class LoopVarsBuilder

  return LoopVarsBuilder().build(sig);
}

LoopVars::LoopVars(const vector<LoopVar> &loopVars,
                   const map<IndexVar,vector<LoopVar>> &vertexLoopVars)
    : loopVars(loopVars), vertexLoopVars(vertexLoopVars) {
}


// Free functions
std::ostream &operator<<(std::ostream &os, const SIGVertex &v) {
  return os << v.iv;
}

std::ostream &operator<<(std::ostream &os, const SIGEdge &e) {
  return os << e.tensor << "(" << util::join(e.endpoints) << ")";
}

class SIGPrinter : public SIGVisitor {
public:
  SIGPrinter(std::ostream &os) : os(os) {}
  void print(const SIG &g) {
    apply(g);
  }

private:
  std::ostream &os;

  void visit(const SIGVertex *v) {
    os << *v << ", ";
    SIGVisitor::visit(v);
  }

  void visit(const SIGEdge *e) {
    os << *e << ", ";
    SIGVisitor::visit(e);
  }
};

std::ostream &operator<<(std::ostream &os, const SIG &g) {
  SIGPrinter(os).print(g);
  return os;
}

std::ostream &operator<<(std::ostream &os, const LoopVars &lvs) {
  return os << util::join(lvs);
}

}} // namespace simit::ir
