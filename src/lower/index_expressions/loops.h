#ifndef SIMIT_LOOPS_H
#define SIMIT_LOOPS_H

#include <vector>
#include <ostream>

#include "ir.h"
#include "tensor_index.h"

namespace simit {
namespace ir {

class IndexVariableLoop {
public:
  IndexVariableLoop();
  IndexVariableLoop(const IndexVar &indexVar);
  IndexVariableLoop(const IndexVar &indexVar, IndexVariableLoop linkedLoop);

  const IndexVar &getIndexVar() const;
  const Var &getInductionVar() const;

  bool isLinked() const;
  IndexVariableLoop getLinkedLoop() const;

  bool defined() const {return content != nullptr;}

private:
  struct Content;
  std::shared_ptr<Content> content;
};


/// A TensorIndexVar is a pair of loop induction variables, a coordinate
/// variable and a sink variable, that are retrieved from a tensor index using a
/// source variable. That is, the mapping:
///     (tensorIndex, sourceVar) -> (coordinateVar, sinkVar).
///
/// For example, (A.row2col, i) -> (ijA, jA) is evaluated as follows:
///     ijA = A.row2col.sources[i];
///      jA = A.row2col.sinks[ijA];
///
/// Given the expression c=A*b, ijA can be used to retrieve the matrix component
/// at location (i,j) in A, while i can index into c and j into b. For example,
///     c[i] += A[ijA] * b[j];
class TensorIndexVar {
public:
  TensorIndexVar(Var inductionVar, Var sourceVar, Var tensor,
                 unsigned sourceDim, unsigned sinkDim);

  const Var &getSourceVar() const {return sourceVar;}
  const Var &getCoordinateVar() const {return coordinateVar;}
  const Var &getSinkVar() const {return sinkVar;}

  const TensorIndex &getTensorIndex() const {return tensorIndex;}

  Expr loadCoordinate(int offset=0) const;
  Expr loadSink() const;
  Stmt initCoordinateVar() const;
  Stmt initSinkVar() const;

  friend std::ostream &operator<<(std::ostream&, const TensorIndexVar&);

private:
  Var sourceVar;
  Var coordinateVar;
  Var sinkVar;
  TensorIndex tensorIndex;
};


class SubsetLoop {
public:
  SubsetLoop(const std::vector<TensorIndexVar> &tensorIndexVars,
             const Expr computeExpr = Expr())
      : tensorIndexVars(tensorIndexVars), computeExpr(computeExpr){}

  void setCompoundOperator(CompoundOperator cop) {this->cop = cop;}

  const std::vector<TensorIndexVar> &getTensorIndexVars() const {
    return tensorIndexVars;
  }

  CompoundOperator getCompoundOperator() const {return cop;}
  const Expr &getComputeExpression() const {return computeExpr;}

  friend std::ostream &operator<<(std::ostream&, const SubsetLoop&);

private:
  std::vector<TensorIndexVar> tensorIndexVars;
  CompoundOperator cop = CompoundOperator::None;
  Expr computeExpr;
};


std::vector<SubsetLoop> createSubsetLoops(Expr target,
                                          const IndexExpr *indexExpression,
                                          IndexVariableLoop loop);

}}
#endif
