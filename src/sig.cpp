#include "sig.h"

#include <algorithm>
#include <iostream>
#include <set>

#include "util/util.h"
#include "util/name_generator.h"

using namespace std;

namespace simit {
namespace ir {

// appease GCC
bool ReductionVarsAfterFree(SIGVertex *i, SIGVertex *j);
size_t getNumBlockLevels(const SIG &sig);

// class SIG
SIG::SIG(const std::vector<IndexVar> &ivs, Var tensor, Expr set) : SIG() {
  std::set<IndexVar> added;
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
    SIGEdge *e = new SIGEdge(tensor, set, endpoints);
    content->edges.push_back(unique_ptr<SIGEdge>(e));
  }
}

SIG merge(SIG &sig1, SIG &sig2, SIG::MergeOp mop, Storage& storage) {
   
  // This can be optimized by exploiting immutability to preserve substructures
  SIG merged = SIG();

  // Add the union of the index variables from the input sigs to the merged sig.
  for (auto &v : sig1.content->vertices) {
    const IndexVar &iv = v.first;
    if (merged.content->vertices.find(iv) == merged.content->vertices.end()) {
      merged.content->vertices[iv] = unique_ptr<SIGVertex>(new SIGVertex(iv));
    }
  }
  for (auto &v : sig2.content->vertices) {
    const IndexVar &iv = v.first;
    if (merged.content->vertices.find(iv) == merged.content->vertices.end()) {
      merged.content->vertices[iv] = unique_ptr<SIGVertex>(new SIGVertex(iv));
    }
  }

  // Add the edges from the input sigs to the merged sig. If two edges share the
  // same endpoints then they are merged.
  for (auto &e : sig1.content->edges) {
    // Get pointers to the endpoints in the new sig
    vector<SIGVertex*> endpoints;
    for (SIGVertex *eps : e->endpoints) {
      endpoints.push_back(merged.content->vertices[eps->iv].get());
    }

    SIGEdge *edge = new SIGEdge(e->tensor, e->set, endpoints);
    merged.content->edges.push_back(unique_ptr<SIGEdge>(edge));
  }

  for (auto &e : sig2.content->edges) {
    // Get pointers to the endpoints in the new sig
    vector<SIGVertex*> endpoints;
    for (SIGVertex *eps : e->endpoints) {
      endpoints.push_back(merged.content->vertices[eps->iv].get());
    }
    // this predicate tests if the vertices have the same indexvar
    auto vert_pred = [] (decltype(*e->endpoints.begin()) a, decltype(a) b)
      {return a->iv == b->iv; };
    
    // predicate for edges checks if all endpoints are the same
    auto edge_pred = [&] (unique_ptr<SIGEdge>& a) {
        return a.get()->endpoints.size() == e->endpoints.size() &&
        std::equal(a->endpoints.begin(),
                   a->endpoints.end(),
                   e->endpoints.begin(),
                   vert_pred);
      };
    
    // find an edge in the merged set that is "equal" to the edge we are
    // adding.  this means it has all the same vertices.
    auto exists = find_if(merged.content->edges.begin(),
                       merged.content->edges.end(),
                       edge_pred);
    
    if (exists  != merged.content->edges.end()) {
      // if it exists, we want to keep the dominating version of the
      // edge: the one that is a sysreduced tensor
      auto storageKind = storage.getStorage(e->tensor).getKind();
  
      if (storageKind == TensorStorage::Kind::Indexed) {
        exists->get()->tensor = e->tensor;
        exists->get()->set = e->set;
      }
      // otherwise, leave it alone.
      
      
    } else {
      SIGEdge *edge = new SIGEdge(e->tensor, e->set, endpoints);

      merged.content->edges.push_back(unique_ptr<SIGEdge>(edge));
    }
  }

  return merged;
}

bool ReductionVarsAfterFree(SIGVertex *i, SIGVertex *j) {
  return (i->iv.isFreeVar() && j->iv.isReductionVar()) ? true : false;
}

void SIGVisitor::apply(const SIG &sig) {
  visitedVertices.clear();
  visitedEdges.clear();

  std::vector<SIGEdge*> edgeIterationOrder;
  for (auto &e : sig.content->edges) {
    edgeIterationOrder.push_back(e.get());
  }

  std::vector<SIGVertex*> vertexIterationOrder;
  for (auto &v : sig.content->vertices) {
    vertexIterationOrder.push_back(v.second.get());
  }

  // Sort reduction variables after free variables
  sort(vertexIterationOrder.begin(), vertexIterationOrder.end(),
       ReductionVarsAfterFree);

  for (SIGVertex *v : vertexIterationOrder) {
    if (visitedVertices.find(v) == visitedVertices.end()) {
      visit(v);
    }
  }

  for (SIGEdge *e : edgeIterationOrder) {
    if (visitedEdges.find(e) == visitedEdges.end()) {
      visit(e);
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

SIG createSIG(Stmt stmt, const Storage &storage) {
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
    
    using IRVisitor::visit;

    SIG create(Expr expr) {
      expr.accept(this);
      SIG result = sig;
      sig = SIG();
      return result;
    }

    void visit(const IndexedTensor *op) {
      iassert(!isa<IndexExpr>(op->tensor))
          << "IndexExprs should have been flattened by now:"
          << util::toString(*op);

      iassert(op->tensor.type().isTensor());
      auto type = op->tensor.type().toTensor();

      Var tensorVar;
      Expr setExpr;
      if (isa<VarExpr>(op->tensor) && !isScalar(op->tensor.type())) {
        const Var &var = to<VarExpr>(op->tensor)->var;
        iassert(var.getType().isTensor())
            << "Index expression result must be a tensor";
        iassert(storage.hasStorage(var))
            << "No storage descriptor found for " << var << " in "
            << util::toString(*op);

        if (type->order() == 2 && type->hasSystemDimensions()) {
          tensorVar = var;

          // Assumes all indexed matrices are (B)CSR
          auto type = var.getType().toTensor();
          IndexSet dimension = type->getOuterDimensions()[0];
          iassert(dimension.getKind() == IndexSet::Set)
              << "Assumes first dimension is sparse";
          setExpr = dimension.getSet();
        }
      }
      sig = SIG(op->indexVars, tensorVar, setExpr);
    }

    void visit(const Add *op) {
      SIG ig1 = create(op->a);
      SIG ig2 = create(op->b);
      sig = merge(ig1, ig2, SIG::Union, storage);
    }

    void visit(const Sub *op) {
      SIG ig1 = create(op->a);
      SIG ig2 = create(op->b);
      sig = merge(ig1, ig2, SIG::Union, storage);
    }

    void visit(const Mul *op) {
      SIG ig1 = create(op->a);
      SIG ig2 = create(op->b);
      sig = merge(ig1, ig2, SIG::Intersection, storage);
    }

    void visit(const Div *op) {
      SIG ig1 = create(op->a);
      SIG ig2 = create(op->b);
      sig = merge(ig1, ig2, SIG::Intersection, storage);
    }

    void visit(const Rem *op) {
      not_supported_yet;
    }
  };

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
    
    using SIGVisitor::visit;

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
LoopVars LoopVars::create(const SIG &sig, const Storage &storage) {
  class LoopVarsBuilder : protected SIGVisitor {
  public:
    LoopVarsBuilder(const Storage &storage) : storage(storage) {}
    LoopVars build(const SIG &sig) {
      vertexLoopVars.clear();

      // We create one set of loop nests per block level in the SIG expression.
      numBlockLevels = getNumBlockLevels(sig);
      for (currBlockLevel=0; currBlockLevel<numBlockLevels; ++currBlockLevel) {
        apply(sig);
      }

      return LoopVars(loopVars, vertexLoopVars, coordVars);
    }

  private:
    const Storage& storage;
    std::vector<LoopVar> loopVars;
    std::map<IndexVar, std::vector<LoopVar>> vertexLoopVars;
    std::map<std::vector<Var>, Var> coordVars;

    util::NameGenerator nameGenerator;

    /// We create one loop variable per block level per index variable. The loop
    /// variables are ordered by block level. This variable keeps track of which
    /// block level we are currently creating loop variables for.
    size_t currBlockLevel;

    /// The number of block levels in the index variables in the SIG.
    size_t numBlockLevels;
    
    using SIGVisitor::visit;

    void visit(const SIGVertex *v) {
      const IndexVar &indexVar = v->iv;
      size_t numBlockLevels = indexVar.getNumBlockLevels();

      if (currBlockLevel < numBlockLevels) {
        // If this vertex connects to any lattice links, we must impose lattice
        // structure on the loop.
        Expr latticeSet;
        for (auto &e : v->connectors) {
          const TensorStorage& ts = storage.getStorage(e->tensor);
          auto storageKind = ts.getKind();
          if (storageKind == TensorStorage::Kind::Stencil &&
              ts.hasTensorIndex()) {
            const TensorIndex& ti = ts.getTensorIndex();
            latticeSet = ti.getStencilLayout().getLatticeSet();
            break;
          }
        }
        if (!latticeSet.defined()) {
          Var var(nameGenerator.getName(indexVar.getName()), Int);
          ForDomain domain = indexVar.getDomain().getIndexSets()[currBlockLevel];

          // We only reduce w.r.t. to the inner loop variable.
          ReductionOperator rop = (currBlockLevel == numBlockLevels-1)
                                  ? indexVar.getOperator()
                                  : ReductionOperator::Undefined;
      
          // If the index set we are traversing over is fixed,
          // we construct the corresponding domain & index set
          if (indexVar.isFixed()) {
            domain = ForDomain(IndexSet(*indexVar.ptr->fixedExpr, IndexSet::Single));
          }

          addVertexLoopVar(indexVar, LoopVar(var, domain, rop));
        }
        else {
          iassert(latticeSet.type().isLatticeLinkSet());
          int ndims = latticeSet.type().toLatticeLinkSet()->dimensions;
          string varName = nameGenerator.getName(indexVar.getName());
          Var var(varName, Int);
          // We only reduce w.r.t. to the inner loop variable.
          ReductionOperator rop = (currBlockLevel == numBlockLevels-1)
              ? indexVar.getOperator()
              : ReductionOperator::Undefined;
          ForDomain domain(latticeSet, var, ndims, varName);
          addVertexLoopVar(indexVar, LoopVar(var, domain, rop));
        }

      }

      SIGVisitor::visit(v);
    }

    void visit(const SIGEdge *e) {
      tassert(e->endpoints.size() <= 2)
          << "This code does not support higher-order tensors yet";

      // There are three cases:
      // - Case 1: The edge was visited before any of it's endpoints
      // - Case 2: The edge was visited after one of its endpoints
      // - Case 3: The edge was visited after more than one of its endpoints

      /// Get an endpoint that has been visited.
      vector<const SIGVertex*> visited;
      vector<const SIGVertex*> notVisited;
      for (const SIGVertex *ep : e->endpoints) {
        if (visitedVertices.find(ep) != visitedVertices.end()) {
          visited.push_back(ep);
        }
        else {
          notVisited.push_back(ep);
        }
      }

      // We currently only support case 2.
      tassert(visited.size() == 1);
      
      auto storageKind = storage.getStorage(e->tensor).getKind();
     
      auto loopVars = vertexLoopVars[visited[0]->iv];
      LoopVar linkLoop = loopVars[loopVars.size()-1];
      Var link = linkLoop.getVar();

      // Link the not visited variable(s) to the visited variable.
      for (auto &veps : notVisited) {
        const IndexVar &indexVar = veps->iv;

        if (currBlockLevel == 0) {
          Var var(nameGenerator.getName(indexVar.getName()), Int);
          
          ForDomain::Kind domainKind = ForDomain::Neighbors;
          
          if (storageKind == TensorStorage::Kind::Indexed) {
            // if we have a fixed index var, then we need
            // a NeigborsOf domain
            if (indexVar.isFixed()) {
              domainKind = ForDomain::NeighborsOf;
            }
            else {
              domainKind = ForDomain::Neighbors;
            }
          } else if (storageKind == TensorStorage::Kind::Diagonal) {
            domainKind = ForDomain::Diagonal;
          } else if (storageKind == TensorStorage::Kind::Stencil) {
            domainKind = ForDomain::Neighbors;
          }
          else {
            ierror << "Unknown storage kind for tensor " << e->tensor;
          }
          
          ForDomain domain;
          
          if (domainKind == ForDomain::NeighborsOf)
            domain = ForDomain(e->set, link, domainKind,
              IndexSet(*indexVar.ptr->fixedExpr, IndexSet::Single));
          else
            domain = ForDomain(e->set, link, domainKind);
          
          // We only need to reduce w.r.t. to the inner loop variable.
          ReductionOperator rop = (currBlockLevel == numBlockLevels-1)
                                  ? indexVar.getOperator()
                                  : ReductionOperator::Undefined;
          
          addVertexLoopVar(indexVar, LoopVar(var, domain, rop));

          if (storageKind == TensorStorage::Kind::Indexed ||
              storageKind == TensorStorage::Kind::Stencil) {
            // The ij var links i to j through the neighbors indices. E.g.
            // for i in points:
            //   pointsi = 0;
            //   for ij in edges.neighbors.start[i]:edges.neighbors.start[(i+1)]:
            //     j = springs.neighbors[ij];
            //     pointsi = (pointsi + (A[ij] * points.b[j]));
            //   points.c[i] = pointsi;
            addCoordVar({link, var}, Var(link.getName()+indexVar.getName(), Int));
          }
          visitedVertices.insert(veps);
        }
      }

      SIGVisitor::visit(e);
    }

    void addVertexLoopVar(const IndexVar &indexVar, const LoopVar &loopVar) {
      loopVars.push_back(loopVar);

      // Add entry for the indexVar
      if (vertexLoopVars.find(indexVar) == vertexLoopVars.end()) {
        vertexLoopVars[indexVar] = std::vector<LoopVar>();
      }
      vertexLoopVars[indexVar].push_back(loopVar);
    }

    void addCoordVar(std::vector<Var> coord, const Var &var) {
      sort(coord.begin(), coord.end());
      coordVars[coord] = var;
    }
  }; // class LoopVarsBuilder

  return LoopVarsBuilder(storage).build(sig);
}

LoopVars::LoopVars(const vector<LoopVar> &loopVars,
                   const map<IndexVar, vector<LoopVar>> &vertexLoopVars,
                   const map<vector<Var>, Var> &coordVars)
    : loopVars(loopVars), vertexLoopVars(vertexLoopVars), coordVars(coordVars) {
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
  
  using SIGVisitor::visit;

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
