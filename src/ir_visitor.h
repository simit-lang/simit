#ifndef SIMIT_IR_VISITOR_H
#define SIMIT_IR_VISITOR_H

#include <set>
#include "macros.h"

namespace simit {
namespace ir {

class Func;

struct Literal;
struct Variable;
struct Result;
struct FieldRead;
struct TensorRead;
struct TupleRead;
struct Map;
struct IndexedTensor;
struct IndexExpr;
struct Call;
struct Neg;
struct Add;
struct Sub;
struct Mul;
struct Div;

struct AssignStmt;
struct FieldWrite;
struct TensorWrite;
struct For;
struct IfThenElse;
struct Block;
struct Pass;

/// Visitor where the iteration order is specified in the visitor instead of
/// the accept methods.  This design is chosen to allow different visitors to
/// specify different traversal orders.  As a consequence the visit methods are
/// called to start a traversal, while handle methods are called to perform
/// actions on objects as specified by visitor subclasses.
///
/// The default IRVisitor visits each tensor in a function once in forward order
/// starting with arguments and literals and ending with the results.
class IRVisitor {
public:
  virtual ~IRVisitor();

  virtual void visit(const Func *);

  virtual void visit(const Literal *);
  virtual void visit(const Variable *);
  virtual void visit(const Result *);
  virtual void visit(const FieldRead *);
  virtual void visit(const TensorRead *);
  virtual void visit(const TupleRead *);
  virtual void visit(const Map *);
  virtual void visit(const IndexedTensor *);
  virtual void visit(const IndexExpr *);
  virtual void visit(const Call *);
  virtual void visit(const Neg *);
  virtual void visit(const Add *);
  virtual void visit(const Sub *);
  virtual void visit(const Mul *);
  virtual void visit(const Div *);

  virtual void visit(const AssignStmt *);
  virtual void visit(const FieldWrite *);
  virtual void visit(const TensorWrite *);
  virtual void visit(const For *);
  virtual void visit(const IfThenElse *);
  virtual void visit(const Block *);
  virtual void visit(const Pass *);
};

}} // namespace simit::internal
#endif
