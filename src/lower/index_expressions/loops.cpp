#include "loops.h"

using namespace std;

namespace simit {
namespace ir {

// class IndexVariableLoop
struct IndexVariableLoop::Content {
  IndexVar indexVar;
  Var inductionVar;
  IndexVariableLoop linkedLoop;
};

IndexVariableLoop::IndexVariableLoop() {
}

IndexVariableLoop::IndexVariableLoop(const IndexVar &indexVar)
    : IndexVariableLoop(indexVar, IndexVariableLoop()) {
}

IndexVariableLoop::IndexVariableLoop(const IndexVar &indexVar,
                                     IndexVariableLoop linkedLoop)
    : content(new Content) {
  content->indexVar = indexVar;
  content->inductionVar = Var(indexVar.getName(), Int);
  content->linkedLoop = linkedLoop;
}

const IndexVar &IndexVariableLoop::getIndexVar() const {
  return content->indexVar;
}

const Var &IndexVariableLoop::getInductionVar() const {
  return content->inductionVar;
}

bool IndexVariableLoop::isLinked() const {
  return content->linkedLoop.defined();
}

IndexVariableLoop IndexVariableLoop::getLinkedLoop() const {
  return content->linkedLoop;
}


// class TensorIndexVar
TensorIndexVar::TensorIndexVar(Var inductionVar, Var sourceVar, Var tensor,
                 unsigned sourceDim, unsigned sinkDim) {
  this->sinkVar = Var(inductionVar.getName() + tensor.getName(), Int);
  this->sourceVar = sourceVar;
  this->coordinateVar =
      Var(sourceVar.getName()+inductionVar.getName()+tensor.getName(), Int);
  this->tensorIndex = TensorIndex(tensor, sourceDim);
}

ostream &operator<<(ostream &os, const TensorIndexVar &tiv) {
  os << tiv.sinkVar
     << " in "      << tiv.tensorIndex
     << ".sinks["   << tiv.coordinateVar
     << " in "      << tiv.tensorIndex
     << ".sources[" << tiv.sourceVar << "]]";
  return os;
}

}}
