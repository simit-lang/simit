#ifndef SIMIT_IR_VISITOR_H
#define SIMIT_IR_VISITOR_H

#include <set>
#include "macros.h"

namespace simit {
namespace ir {

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

class Func;

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

  virtual void visit(const Literal *op);
  virtual void visit(const Variable *op);
  virtual void visit(const Result *op);
  virtual void visit(const FieldRead *op);
  virtual void visit(const TensorRead *op);
  virtual void visit(const TupleRead *op);
  virtual void visit(const Map *op);
  virtual void visit(const IndexedTensor *op);
  virtual void visit(const IndexExpr *op);
  virtual void visit(const Call *op);
  virtual void visit(const Neg *op);
  virtual void visit(const Add *op);
  virtual void visit(const Sub *op);
  virtual void visit(const Mul *op);
  virtual void visit(const Div *op);

  virtual void visit(const AssignStmt *op);
  virtual void visit(const FieldWrite *op);
  virtual void visit(const TensorWrite *op);
  virtual void visit(const For *op);
  virtual void visit(const IfThenElse *op);
  virtual void visit(const Block *op);
  virtual void visit(const Pass *op);

  virtual void visit(const Func *f);
};

}} // namespace simit::internal
#endif
