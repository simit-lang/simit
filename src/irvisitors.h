#ifndef SIMIT_IR_VISITOR_H
#define SIMIT_IR_VISITOR_H

#include <set>
#include <map>
#include <list>

#include "macros.h"

namespace simit {
namespace internal {

class Tensor;
class Function;
class Argument;
class Result;
class LiteralTensor;
class IndexExpr;
class VariableStore;

/** Visitor where the iteration order is specified in the visitor instead of
  * the accept methods.  This design is chosen to allow different visitors to
  * specify different traversal orders.  As a consequence the visit methods are
  * called to start a traversal, while handle methods are called to perform
  * actions on objects as specified by visitor subclasses.
  *
  * The default IRVisitor visits each tensor in a function once in forward order
  * starting with arguments and literals and ending with the results.
  */
class IRVisitor {
 public:
  IRVisitor() { reset(); }
  virtual ~IRVisitor();

  virtual void visit(Function        *f);
  virtual void visit(Argument        *t);
  virtual void visit(Result          *t);
  virtual void visit(LiteralTensor   *t);
  virtual void visit(IndexExpr *t);
  virtual void visit(VariableStore   *t);

  virtual void handle(Function        *f) { UNUSED(f); };
  virtual void handle(Argument        *t) { UNUSED(t); };
  virtual void handle(Result          *t) { UNUSED(t); };
  virtual void handle(LiteralTensor   *t) { UNUSED(t); };
  virtual void handle(IndexExpr *t)       { UNUSED(t); };
  virtual void handle(VariableStore   *t) { UNUSED(t); };

 protected:
  void abort() { aborted = true; }
  bool isAborted() { return aborted; }
  void reset() {
    aborted = false;
    visited.clear();
  }

 private:
  std::set<Tensor*> visited;
  bool aborted;
};

}} // namespace simit::internal
#endif
