#include "lower_scatter_workspace.h"

#include <vector>
#include <set>
#include <map>
#include <string>

#include "loops.h"

#include "indexvar.h"
#include "ir.h"
#include "ir_visitor.h"
#include "ir_codegen.h"
#include "util.h"

using namespace std;

namespace simit {
namespace ir {

typedef vector<IndexVar> IndexTuple;
typedef map<IndexTuple, vector<const IndexedTensor *>> IndexTupleUses;
typedef map<IndexVar, vector<const IndexedTensor *>> IndexUses;
typedef map<IndexVar, vector<IndexVar>> IndexVarGraph;
typedef map<IndexVar, pair<Var,vector<TensorIndexVar>>> InductionVars;

ostream &operator<<(ostream &os, const IndexVarGraph &ivGraph) {
  os << "Index Variable Graph:"  << endl;
  for (auto &ij : ivGraph) {
    auto i = ij.first;
    for (auto &j : ij.second) {
      os << i << " -> " << j << endl;
    }
  }
  return os;
}

ostream &operator<<(ostream &os, const IndexUses &indexUses) {
  os << "Index Variable Uses:" << std::endl;
  for (auto &itu : indexUses) {
    for (auto &it : itu.second) {
      os << itu.first << " -> " << *it << endl;
    }
  }
  return os;
}

ostream &operator<<(ostream &os, const IndexTupleUses &indexTupleUses) {
  os << "Index Variable Tuple Uses:" << endl;
  for (auto &itu : indexTupleUses) {
    for (auto &it : itu.second) {
      os << "(" << util::join(itu.first, ",") << ")" << " -> " << *it << endl;
    }
  }
  return os;
}

ostream &operator<<(ostream &os, const InductionVars &inductionVars) {
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
                           const IndexVariableLoop &linkedLoop,
                           set<IndexVar> *visited,
                           vector<IndexVariableLoop> *loops) {
  for (const IndexVar &sink : ivGraph.at(linkedLoop.getIndexVar())) {
    if (!util::contains(*visited, sink)) {
      visited->insert(sink);
      loops->push_back(IndexVariableLoop(sink, linkedLoop));
      createLoopNest(ivGraph, sink, visited, loops);
    }
  }
}

static vector<IndexVariableLoop> createLoopNest(const IndexVarGraph &ivGraph,
                                   const vector<IndexVar> &sources){
  vector<IndexVariableLoop> loops;
  set<IndexVar> visited;
  for (auto &source : sources) {
    if (!util::contains(visited, source)) {
      visited.insert(source);
      IndexVariableLoop loop(source);
      loops.push_back(loop);
      createLoopNest(ivGraph, loop, &visited, &loops);
    }
  }
  return loops;
}

/// Build a map from index variables to the IndexTensors they access.
/// - B+C  i -> B(i,j), C(i,j)
///        j -> B(i,j), C(i,j)
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

static
InductionVars createInductionVariables(const vector<IndexVariableLoop> &loops,
                                       const IndexTupleUses &indexTupleUses) {
  IndexUses indexUses = getIndexUses(indexTupleUses);

  InductionVars inductionVars;
  for (auto &loop : loops) {
    IndexVar indexVar = loop.getIndexVar();

    Var inductionVar = loop.getInductionVar();
    inductionVars[indexVar].first = inductionVar;

    if (loop.isLinked()) {
      IndexVar linkedIndexVar = loop.getLinkedLoop().getIndexVar();
      Var linkedInductionVar  = loop.getLinkedLoop().getInductionVar();

      vector<const IndexedTensor *> uses = indexUses.at(indexVar);
      for (auto &indexedTensor : uses) {
        iassert(isa<VarExpr>(indexedTensor->tensor))
            << "at this point the index expressions should have been flattened";
        Var tensor = to<VarExpr>(indexedTensor->tensor)->var;
        vector<IndexVar> indexVars = indexedTensor->indexVars;

        TensorIndexVar indexInductionVar(inductionVar, linkedInductionVar,
                                         tensor,
                                         util::locate(indexVars,linkedIndexVar),
                                         util::locate(indexVars,indexVar));
        inductionVars.at(indexVar).second.push_back(indexInductionVar);
      }
    }
  }
  return inductionVars;
}

static Expr compareToNextIndexLocation(const TensorIndexVar &inductionVar) {
  return Lt::make(inductionVar.getCoordinateVar(),
                  inductionVar.loadCoordinate(1));
}

static
Expr sparseLoopCondition(const vector<TensorIndexVar> &inductionVars) {
  auto it = inductionVars.begin();
  auto end = inductionVars.end();
  Expr condition = compareToNextIndexLocation(*it++);
  for (; it != end; ++it) {
    condition = And::make(condition, compareToNextIndexLocation(*it));
  }
  return condition;
}

static
Stmt sparseLoop(const vector<TensorIndexVar> &inductionVars, Stmt body,
                bool initCoordVars=true) {
  // Create while loop condition. Sparse while loops simultaneously
  // iterate over the coordinate variables of one or more tensors
  Expr condition = sparseLoopCondition(inductionVars);


  // Initialize sink induction variables
  vector<Stmt> initSinkInductionVars;
  for (auto &inductionVar : inductionVars) {
    initSinkInductionVars.push_back(inductionVar.initSinkVar());
  }
  body = Block::make(Block::make(initSinkInductionVars), body);


  // Increment coordinate induction variables at the end of the loop body
  vector<Stmt> incCoordVarStmts;
  for (auto &inductionVar : inductionVars) {
    incCoordVarStmts.push_back(increment(inductionVar.getCoordinateVar()));
  }
  body = Block::make(body, Block::make(incCoordVarStmts));


  // Create loop
  Stmt loop = While::make(condition, body);


  // Initialize coordinate induction variable
  if (initCoordVars) {
    vector<Stmt> initCoordVarsStmts;
    for (auto &inductionVar : inductionVars) {
      initCoordVarsStmts.push_back(inductionVar.initCoordinateVar());
    }
    loop = Block::make(Block::make(initCoordVarsStmts), loop);
  }

  return loop;
}

string tensorSliceString(const vector<Var> &vars, const Var &sliceVar) {
  unsigned sliceDimension = util::locate(vars, sliceVar);
  string result = "(";
  for (size_t i=0; i < vars.size(); ++i) {
    result += (i == sliceDimension) ? ":" : toString(vars[i]);
    if (i < vars.size()-1) {
      result += ",";
    }
  }
  result += ")";
  return result;
}

Stmt lowerScatterWorkspace(Expr target, const IndexExpr *indexExpression) {
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
  vector<IndexVariableLoop> loops = createLoopNest(indexVariableGraph,
                                                   indexExpression->resultVars);


  // Create Loop Inducation Variables and Coordinate Induction Variables:
  // - B+C  i
  //        j: zip(ijB in nbr(B), ijC in nbr(C))
  InductionVars inductionVars = createInductionVariables(loops, indexTupleUses);
  std::cout << inductionVars << std::endl;



  // Emit loops
  Stmt loopNest;
  for (IndexVariableLoop &loop : util::reverse_iterator(loops)) {

    // Dense loops
    if (!loop.isLinked()) {
      IndexVar indexVar = loop.getIndexVar();
      auto &loopInductionVar = inductionVars.at(indexVar).first;
      auto &domain = indexVar.getDomain().getIndexSets()[0];
      loopNest = For::make(loopInductionVar, domain, loopNest);
    }
    // Sparse loops
    else {
      auto loopInductionVars = inductionVars.at(loop.getIndexVar());
      Var inductionVar = loopInductionVars.first;
      vector<TensorIndexVar> indexInductionVars = loopInductionVars.second;

      vector<SubsetLoop> subsetLoops =
          createSubsetLoops(target, indexExpression, loop);
      std::cout << "Subset Loops:" << std::endl;
      for (auto &subsetLoop : subsetLoops) {
        std::cout << subsetLoop << std::endl;
      }
      std::cout << std::endl;


      // Create loops that add row i of each operand to the workspace. Note
      // that for union-conforming operators each operand is added in in a
      // separate loop nest. For example, if A=B+C then, for each row, all the
      // values of B are added before the values of C are added.
      vector<Stmt> loopStatements;
      for (const TensorIndexVar &inductionVar : indexInductionVars) {
        // TODO OPT: The first loop can use = instead of +=.
        Stmt loopStatement = sparseLoop({inductionVar}, loopNest, true);

        string comment = "workspace += ...";
        //              inductionVar.getTensorIndex().getTensor().getName() +
        //              matrixSliceString(inductionVar.getSourceVar(),
        //                  inductionVar.getTensorIndex().getSourceDimension());
        loopStatements.push_back(Comment::make(comment, loopStatement));
      }
      iassert(loops.size() > 0);


      // Copy the workspace to the index expression target.
      std::function<Var(IndexVar)> iv2v = [&inductionVars](IndexVar v) -> Var{
        return inductionVars.at(v).first;
      };
      vector<Var> resultVars = util::map(indexExpression->resultVars, iv2v);

      Stmt loopStatement = Pass::make();
      //        Stmt body = Pass::make();
      //        Stmt loopStatement = sparseLoop({}, body, true);

      string comment = toString(target) +
      tensorSliceString(resultVars, inductionVar) + " = workspace";
      loopStatements.push_back(Comment::make(comment, loopStatement));

      loopNest = Block::make(loopStatements);
    }
  }

  stringstream comment;
  comment << util::toString(target)
          << "(" + util::join(indexExpression->resultVars, ",") << ") = ";
  IRPrinter printer(comment);
  printer.skipTopExprParenthesis();
  printer.print(indexExpression->value);
  return Comment::make(comment.str(), loopNest);
}

}}
