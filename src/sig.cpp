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
  return os << e.name << " (" << util::join(e.endpoints) << ")";
}

SIG::SIG(const IndexVar &iv) : SIG() {

}

SIG::SIG(string name, const vector<IndexVar> &ivs)
    : SIG() {
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

  SIGEdge *e = new SIGEdge(name, endpoints);
  content->edges[name] = unique_ptr<SIGEdge>(e);
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

  map<string, vector<SIGVertex*>> edges;

  for (auto &e : g1.content->edges) {
    const std::string &name = e.first;
    vector<SIGVertex*> endpoints;
    for (SIGVertex *g1v : e.second->endpoints) {
      endpoints.push_back(merged.content->vertices[g1v->iv].get());
    }
    edges[name] = endpoints;
  }

  for (auto &e : g2.content->edges) {
    const std::string &name = e.first;
    vector<SIGVertex*> endpoints;
    for (SIGVertex *g1v : e.second->endpoints) {
      endpoints.push_back(merged.content->vertices[g1v->iv].get());
    }

    if (edges.find(name) == edges.end()) {
      edges[name] = endpoints;
    }
    else {
      // Vertex identification/contraction
      vector<SIGVertex*> &currEps = edges[name];
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

void SIGVisitor::apply(const SIG &sig) {
  for (auto &v : sig.content->vertices) {
    if (visitedVertices.find(v.second.get()) == visitedVertices.end()) {
      visit(v.second.get());
    }
  }
}

void SIGVisitor::apply(const SIG &sig, const IndexVar &first) {
  assert(sig.content->vertices.find(first) != sig.content->vertices.end());
  visit(sig.content->vertices[first].get());
  for (auto &v : sig.content->vertices) {
    if (visitedVertices.find(v.second.get()) == visitedVertices.end()) {
      visit(v.second.get());
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
