#ifndef SIMIT_LOOPS_H
#define SIMIT_LOOPS_H

#include "ir.h"

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

}}
#endif
