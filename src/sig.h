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
  Var tensor;
  std::vector<SIGVertex*> endpoints;

  SIGEdge(Var tensor, const std::vector<SIGVertex*> &endpoints)
      : tensor(tensor), endpoints(endpoints) {
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
  explicit SIG(const std::vector<IndexVar> &ivs, Var tensor=Var());

  friend SIG merge(SIG&, SIG&, SIG::MergeOp);

private:
  struct Content {
    std::map<IndexVar, std::unique_ptr<SIGVertex>> vertices;
    std::map<Var, std::unique_ptr<SIGEdge>> edges;
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
};


class SIGBuilder : public IRVisitor {
public:
  SIGBuilder(const UseDef *ud) : ud(ud) {}

  SIG create(const IndexExpr *expr) {
    return create(Expr(expr));
  }

private:
  const UseDef *ud;
  SIG sig;

  SIG create(Expr expr) {
    expr.accept(this);
    SIG result = sig;
    sig = SIG();
    return result;
  }

  void visit(const IndexedTensor *op) {
    if (op->indexVars.size() == 1) {
      sig = SIG({op->indexVars[0]});
    }
    else if (op->indexVars.size() >= 2) {
      assert(!isa<IndexExpr>(op->tensor) &&
             "IndexExprs should have been flattened by now");

      Var tensorVar;
      if (isa<VarExpr>(op->tensor)) {
        const Var &var = to<VarExpr>(op->tensor)->var;
        if (ud->getDef(var).getKind() == VarDef::Map) {
          tensorVar = var;
        }
      }

      sig = SIG(op->indexVars, tensorVar);
    }
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

}} // namespace
#endif
