#include "sig.h"

#include <algorithm>
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
  return os << e.tensor << "(" << util::join(e.endpoints) << ")";
}

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

bool ReductionVarsBeforefree(SIGVertex *i, SIGVertex *j) {
  if (i->iv.isFreeVar() && j->iv.isReductionVar()) {
    return false;
  }
  else if (i->iv.isReductionVar() && j->iv.isFreeVar()) {
    return true;
  }
  else {
    return (i->iv.getName() < j->iv.getName());
  }
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
  sort(vertexIterationOrder.begin(), vertexIterationOrder.end(), ReductionVarsBeforefree);

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

}} // namespace simit::ir
