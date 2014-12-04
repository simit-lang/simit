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
#include "tensor_storage.h"

namespace simit {
namespace ir {

struct SIGEdge;
class SIGVisitor;

struct SIGVertex {
  IndexVar iv;
  std::vector<SIGEdge*> connectors;

  SIGVertex(const IndexVar &iv) : iv(iv) {}
};


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

  friend SIG merge(SIG&, SIG&, SIG::MergeOp);

  bool isSparse() const {return content->edges.size() > 0;}

  std::vector<const SIGEdge *> getEdges() const {
    std::vector<const SIGEdge *> edges;
    for (auto &edge : content->edges) {
      edges.push_back(edge.second.get());
    }
    return edges;
  }

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


/// Class that builds a Sparse Iteration Graph from an expression.
class SIGBuilder : public IRVisitor {
public:
  SIGBuilder(const TensorStorages &storageDescriptors)
      : storageDescriptors(storageDescriptors) {}

  SIG create(const IndexExpr *expr) {
    return create(Expr(expr));
  }

private:
  TensorStorages storageDescriptors;

  SIG sig;

  SIG create(Expr expr) {
    expr.accept(this);
    SIG result = sig;
    sig = SIG();
    return result;
  }

  void visit(const IndexedTensor *op) {

    iassert(!isa<IndexExpr>(op->tensor))
        << "IndexExprs should have been flattened by now";

    Var tensorVar;
    if (isa<VarExpr>(op->tensor) && !isScalar(op->tensor.type())) {
      const Var &var = to<VarExpr>(op->tensor)->var;
      iassert(storageDescriptors.find(var) != storageDescriptors.end())
          << "No storage descriptor found for" << var << "in"
          << util::toString(*op);
      if (storageDescriptors.at(var).isSystem()) {
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


std::ostream &operator<<(std::ostream &os, const SIGVertex &);
std::ostream &operator<<(std::ostream &os, const SIGEdge &);
std::ostream &operator<<(std::ostream &os, const SIG &);

}} // namespace
#endif
