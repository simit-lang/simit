#ifndef SIMIT_IR_VISITOR_H
#define SIMIT_IR_VISITOR_H

namespace simit {

class Argument;
class Result;
class LiteralTensor;
class Merge;
class VariableStore;


/** Visitor where the iteration order is specified in the visitor instead of
  * the accept methods.  This design is chosen to allow different visitors to
  * specify different traversal orders.  As a consequence the visit methods are
  * called to start a traversal, while handle methods are called to perform
  * actions on objects as specified by visitor subclasses.
  */
class IRVisitor {
 public:
  virtual ~IRVisitor();

  virtual void visit(LiteralTensor *t);
  virtual void visit(Argument      *t);
  virtual void visit(Result        *t);
  virtual void visit(Merge         *t);
  virtual void visit(VariableStore *t);

  virtual void handle(LiteralTensor *t);
  virtual void handle(Argument      *t);
  virtual void handle(Result        *t);
  virtual void handle(Merge         *t);
  virtual void handle(VariableStore *t);
};



}
#endif
