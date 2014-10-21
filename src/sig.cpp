#include "sig.h"

#include <iostream>
#include <set>

#include "util.h"

using namespace std;

namespace simit {
namespace ir {

std::ostream &operator<<(std::ostream &os, const SIGVertex &v) {
  return os << v.iv;
}

std::ostream &operator<<(std::ostream &os, const SIGEdge &e) {
  return os << e.edgeSet << " (" << util::join(e.endpoints) << ")";
}

SIG::SIG(const std::vector<IndexVar> &ivs, Expr setExpr) : SIG() {
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

  if (setExpr.defined()) {
    SIGEdge *e = new SIGEdge(setExpr, endpoints);
    content->edges[setExpr] = unique_ptr<SIGEdge>(e);
  }
}

// This can be optimized by exploiting immutability to preserve substructures
SIG merge(SIG &g1, SIG &g2, SIG::MergeOp mop) {
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

  map<Expr,vector<SIGVertex*>> edges;

  for (auto &e : g1.content->edges) {
    Expr edgeSet = e.first;
    vector<SIGVertex*> endpoints;
    for (SIGVertex *g1v : e.second->endpoints) {
      endpoints.push_back(merged.content->vertices[g1v->iv].get());
    }
    edges[edgeSet] = endpoints;
  }

  for (auto &e : g2.content->edges) {
    Expr edgeSet = e.first;
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

class SIGPrinter : public SIGVisitor {
public:
  SIGPrinter(std::ostream &os) : os(os) {}
  void print(const SIG &g) {
    apply(g);
  }

private:
  std::ostream &os;

  void visit(const SIGVertex *v) {
    os << "Vert: " << *v << endl;
    SIGVisitor::visit(v);
  }

  void visit(const SIGEdge *e) {
    os << "Edge: " << *e << endl;
    SIGVisitor::visit(e);
  }
};

std::ostream &operator<<(std::ostream &os, const SIG &g) {
  SIGPrinter(os).print(g);
  return os;
}

bool freeBeforeReductionVars(SIGVertex *i, SIGVertex *j) {
  if (i->iv.isFreeVar() && j->iv.isReductionVar()) {
    return true;
  }
  else if (i->iv.isReductionVar() && j->iv.isFreeVar()) {
    return false;
  }
  else {
    return (i->iv.getName() < j->iv.getName());
  }
}

void SIGVisitor::apply(const SIG &sig) {
  std::vector<SIGVertex*> iterationOrder;
  for (auto &v : sig.content->vertices) {
    iterationOrder.push_back(v.second.get());
  }

  sort(iterationOrder.begin(), iterationOrder.end(), freeBeforeReductionVars);

  // Iterate backwards to allow codegen to be done inside out
  for (auto it = iterationOrder.rbegin(); it != iterationOrder.rend(); ++it) {
    if (visitedVertices.find(*it) == visitedVertices.end()) {
      visit(*it);
    }
  }
}

void SIGVisitor::visit(const SIGVertex *v) {
  visitedVertices.insert(v);
  vertexPath.push_front(v);
  for (auto &e : v->connectors) {
    if (visitedEdges.find(e) == visitedEdges.end()) {
      visit(e);
    }
  }
  vertexPath.pop_front();
}

void SIGVisitor::visit(const SIGEdge *e) {
  visitedEdges.insert(e);
  edgePath.push_front(e);


  for (auto &v : e->endpoints) {
    if (visitedVertices.find(v) == visitedVertices.end()) {
      visit(v);
    }
  }
  edgePath.pop_front();
}

const SIGVertex *SIGVisitor::getPreviousVertex() {
  if (vertexPath.size() > 0) {
    return vertexPath.front();
  }
  else {
    return nullptr;
  }
}

const SIGEdge *SIGVisitor::getPreviousEdge() {
  if (edgePath.size() > 0) {
    return edgePath.front();
  }
  else {
    return nullptr;
  }
}

}} // namespace simit::ir
