#include "loops.h"

namespace simit {
namespace ir {

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

}}
