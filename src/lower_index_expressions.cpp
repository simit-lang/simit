#include "lower_index_expressions.h"

#include <vector>
#include <set>
#include <map>

#include "indexvar.h"
#include "ir.h"
#include "ir_visitor.h"
#include "util.h"

using namespace std;

namespace simit {
namespace ir {

struct Loop {
  enum Type { Dense, Sparse };
  Type type;
  IndexVar indexVar;
  IndexVar parent;

  Loop (const IndexVar &indexVar)
      : type(Dense), indexVar(indexVar) {}

  Loop (const IndexVar &indexVar, IndexVar parent)
      : type(Sparse), indexVar(indexVar), parent(parent) {}
};

struct IndexDescriptor {
  Var tensor;
  // TODO: Build in direction (row-major or col-major, etc.)
};

struct CoordinateVar {
  Var var;

  // Index
  IndexDescriptor index;

  CoordinateVar(const IndexedTensor *indexedTensor) {
    iassert(isa<VarExpr>(indexedTensor->tensor))
        << "at this point the index expressions should have been flattened";
    index.tensor = to<VarExpr>(indexedTensor->tensor)->var;

    string name = util::join(indexedTensor->indexVars, "") +
                  index.tensor.getName();
    var = Var(name, Int);
  }

  friend ostream &operator<<(ostream &os, const CoordinateVar &cv) {
    os << cv.var.getName() << " in nbr(" << cv.index.tensor.getName() << ")" ;
    return os;
  }
};

typedef vector<IndexVar> IndexTuple;
typedef map<IndexTuple, vector<const IndexedTensor *>> IndexTupleUses;
typedef map<IndexVar, vector<const IndexedTensor *>> IndexUses;
typedef map<IndexVar, vector<IndexVar>> IndexVarGraph;
typedef map<IndexVar, pair<Var,vector<CoordinateVar>>> InductionVars;

static ostream &operator<<(ostream &os, const IndexVarGraph &ivGraph) {
  os << "Index variable graph"  << endl;
  for (auto &ij : ivGraph) {
    auto i = ij.first;
    for (auto &j : ij.second) {
      os << i << " -> " << j << endl;
    }
  }
  return os;
}

static ostream &operator<<(ostream &os, const IndexUses &indexUses) {
  os << "Index Variable Uses:" << std::endl;
  for (auto &itu : indexUses) {
    for (auto &it : itu.second) {
      os << itu.first << " -> " << *it << endl;
    }
  }
  return os;
}

static ostream &operator<<(ostream &os, const IndexTupleUses &indexTupleUses) {
  os << "Index Variable Tuple Uses:" << endl;
  for (auto &itu : indexTupleUses) {
    for (auto &it : itu.second) {
      os << "(" << util::join(itu.first, ",") << ")" << " -> " << *it << endl;
    }
  }
  return os;
}

static ostream &operator<<(ostream &os, const InductionVars &inductionVars) {
  os << "Loop induction variables:" << endl;
  for (auto &inductionVar : inductionVars) {
    os << inductionVar.second.first;
    if (inductionVar.second.second.size() > 0) {
      os << ": zip(" << util::join(inductionVar.second.second) << ")";
    }
    os << endl;
  }
  return os;
}


static IndexTupleUses getIndexTupleUses(const IndexExpr *indexExpr) {
  struct GetIndexTupleUsesVisitor : public IRVisitor {
    IndexTupleUses indexTupleUses;
    void visit(const IndexedTensor *indexedTensor) {
      indexTupleUses[indexedTensor->indexVars].push_back(indexedTensor);
    }
  };
  GetIndexTupleUsesVisitor visitor;
  indexExpr->accept(&visitor);
  return visitor.indexTupleUses;
}

static IndexUses getIndexUses(IndexTupleUses indexTupleUses) {
  IndexUses indexUses;
  for (auto &itu : indexTupleUses) {
    for (auto &indexVar : itu.first) {
      for (auto &indexedTensor : itu.second) {
        indexUses[indexVar].push_back(indexedTensor);
      }
    }
  }
  return indexUses;
}

static IndexVarGraph createIndexVarGraph(IndexTupleUses indexTupleUses) {
  IndexVarGraph indexVarGraph;
  for (auto &itu : indexTupleUses) {
    IndexTuple it = itu.first;
    for (size_t i=0; i < it.size() - 1; ++i) {
      for (size_t j=i+1; j < it.size(); ++j) {
        indexVarGraph[it[i]].push_back(it[j]);
        indexVarGraph[it[j]].push_back(it[i]);
      }
    }
  }
  return indexVarGraph;
}

static void createLoopNest(const IndexVarGraph &ivGraph,
                           const IndexVar &source,
                           set<IndexVar> *visited, vector<Loop> *loops) {
  for (auto &sink : ivGraph.at(source)) {
    if (!util::contains(*visited, sink)) {
      visited->insert(sink);
      loops->push_back(Loop(sink, source));
      createLoopNest(ivGraph, sink, visited, loops);
    }
  }
}

static vector<Loop> createLoopNest(const IndexVarGraph &ivGraph,
                                   const vector<IndexVar> &sources){
  vector<Loop> loops;
  set<IndexVar> visited;
  for (auto &source : sources) {
    if (!util::contains(visited, source)) {
      visited.insert(source);
      loops.push_back(Loop(source));
      createLoopNest(ivGraph, source, &visited, &loops);
    }
  }
  return loops;
}

static InductionVars createInductionVariables(const vector<Loop> &loops,
                                              const IndexUses &indexUses) {


  InductionVars inductionVars;
  for (auto &loop : loops) {
    Var inductionVar(loop.indexVar.getName(), Int);
    inductionVars[loop.indexVar].first = inductionVar;

    if (loop.type == Loop::Sparse) {
      vector<const IndexedTensor *> uses = indexUses.at(loop.indexVar);
      for (auto &use : uses) {
        CoordinateVar coordVar(use);
        inductionVars.at(loop.indexVar).second.push_back(coordVar);
      }
    }
  }
  return inductionVars;
}

static Expr compareToIndex(const CoordinateVar &coordVar) {
  return Lt::make(coordVar.var, Literal::make(1));
}

Stmt lower(Expr target, const IndexExpr *indexExpression) {
  // Build a map from index variable tuples to the IndexTensors they access:
  // - B+C   (i,j) -> B(i,j), C(i,j)
  // - B+C'  (i,j) -> B(i,j)
  //         (j,i) -> C(j,i)
  // - B*C:  (i,k) -> B(i,k)
  //         (k,j) -> C(k,j)
  IndexTupleUses indexTupleUses = getIndexTupleUses(indexExpression);
  std::cout << indexTupleUses << std::endl;


  // Build a map from index variables to index variables they can reach through
  // a usage. This map encodes a directed index variable graph where vertices
  // are index variables, and where there exist an edge (i,j) if i and j are
  // ever used together to index a tensor that has an index from i to j. For now
  // we will assume we always have available all indices, but we may later want
  // to optimize for memory by computing a minimum set of indices we need.
  // - B+C: i -> j and j -> i
  // - B*C: i -> k and k -> i
  //        k -> j and j -> k
  IndexVarGraph indexVariableGraph = createIndexVarGraph(indexTupleUses);
  std::cout << indexVariableGraph << std::endl;


  // Order the index variables into one loop per index variable, by traversing
  // the index variable graph
  vector<Loop> loops = createLoopNest(indexVariableGraph,
                                      indexExpression->resultVars);


  // Build a map from index variables to the IndexTensors they access.
  // - B+C  i -> B(i,j), C(i,j)
  //        j -> B(i,j), C(i,j)
  IndexUses indexUses = getIndexUses(indexTupleUses);
  std::cout << indexUses << std::endl;


  // Create Loop Inducation Variables and Coordinate Induction Variables:
  // - B+C  i
  //        j: zip(ijB in nbr(B), ijC in nbr(C))
  InductionVars inductionVars = createInductionVariables(loops, indexUses);
  std::cout << inductionVars << std::endl;


  // Emit loops
  Stmt loopNest = Pass::make();
  for (auto &loop : util::reverse(loops)) {
    switch (loop.type) {
      case Loop::Dense: {
        auto &loopInductionVar = inductionVars.at(loop.indexVar).first;
        auto &domain = loop.indexVar.getDomain().getIndexSets()[0];
        loopNest = For::make(loopInductionVar, domain, loopNest);
        break;
      }
      case Loop::Sparse: {
        auto loopInductionVars = inductionVars.at(loop.indexVar);
        Var inductionVar = loopInductionVars.first;
        auto &coordVars = loopInductionVars.second;

        // Sparse while loops simultaneously iterate over the coordinate
        // variables of one or more tensors
        auto coordIt = coordVars.begin();
        auto coordEnd = coordVars.end();

        Expr condition = compareToIndex(*coordIt);
        ++coordIt;

        for (; coordIt != coordEnd; ++coordIt) {
          condition = And::make(condition, compareToIndex(*coordIt));
        }

        loopNest = While::make(condition, loopNest);
        break;
      }
    }
  }

  std::cout << loopNest << std::endl;

  return loopNest;
}

}}
