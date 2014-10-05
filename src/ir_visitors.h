#ifndef SIMIT_IR_VISITOR_H
#define SIMIT_IR_VISITOR_H

#include <set>
#include "macros.h"

namespace simit {
namespace ir {

class IRNode;
class Function;
class Argument;
class Result;
class Literal;
class IndexExpr;
class Call;
class FieldRead;
class TensorRead;
class FieldWrite;
class TensorWrite;
class Test;

namespace {
class IRVisitorBase {
public:
  IRVisitorBase() { reset(); }
  ~IRVisitorBase() {}
protected:
  void abort() { aborted = true; }
  bool isAborted() { return aborted; }
  void reset() { aborted = false; }
private:
  bool aborted;
};
}

/// Visitor where the iteration order is specified in the visitor instead of
/// the accept methods.  This design is chosen to allow different visitors to
/// specify different traversal orders.  As a consequence the visit methods are
/// called to start a traversal, while handle methods are called to perform
/// actions on objects as specified by visitor subclasses.
///
/// The default IRVisitor visits each tensor in a function once in forward order
/// starting with arguments and literals and ending with the results.
class IRVisitor : protected IRVisitorBase {
public:
  virtual ~IRVisitor();

  virtual void visit(Function *);
  virtual void visit(Argument *);
  virtual void visit(Result *);
  virtual void visit(Literal *);
  virtual void visit(IndexExpr *);
  virtual void visit(Call *);
  virtual void visit(FieldRead *);
  virtual void visit(FieldWrite *);
  virtual void visit(TensorRead *);
  virtual void visit(TensorWrite *);

  virtual void handle(Function *);
  virtual void handle(Argument *);
  virtual void handle(Result *);
  virtual void handle(Literal *);
  virtual void handle(IndexExpr *);
  virtual void handle(Call *);
  virtual void handle(FieldRead *);
  virtual void handle(FieldWrite *);
  virtual void handle(TensorRead *);
  virtual void handle(TensorWrite *);

  virtual void handleDefault(IRNode *);
};


class IRConstVisitor : private IRVisitorBase {
public:
  virtual ~IRConstVisitor();

  virtual void visit(const Function *);
  virtual void visit(const Argument *);
  virtual void visit(const Result *);
  virtual void visit(const Literal *);
  virtual void visit(const IndexExpr *);
  virtual void visit(const Call *);
  virtual void visit(const FieldRead *);
  virtual void visit(const FieldWrite *);
  virtual void visit(const TensorRead *);
  virtual void visit(const TensorWrite *);

  virtual void handle(const Function *);
  virtual void handle(const Argument *);
  virtual void handle(const Result *);
  virtual void handle(const Literal *);
  virtual void handle(const IndexExpr *);
  virtual void handle(const Call *);
  virtual void handle(const FieldRead *);
  virtual void handle(const FieldWrite *);
  virtual void handle(const TensorRead *);
  virtual void handle(const TensorWrite *);

  virtual void handleDefault(const IRNode *);
};


/// Visits each tensor in a function once in backward order starting with
/// results and ending with the arguments and literals.
class IRBackwardVisitor : public IRVisitor {
public:
  virtual ~IRBackwardVisitor();

  virtual void visit(Function *);
  virtual void visit(Argument *);
  virtual void visit(Result *);
  virtual void visit(Literal *);
  virtual void visit(IndexExpr *);
  virtual void visit(Call *);
  virtual void visit(FieldRead *);
  virtual void visit(FieldWrite *);
  virtual void visit(TensorRead *);
  virtual void visit(TensorWrite *);
};

}} // namespace simit::internal
#endif
