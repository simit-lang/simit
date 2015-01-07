#ifndef SIMIT_IR_VISITOR_H
#define SIMIT_IR_VISITOR_H

#include <set>
#include "macros.h"

namespace simit {
namespace ir {

struct Literal;
struct VarExpr;
struct FieldRead;
struct TensorRead;
struct TupleRead;
struct IndexRead;
struct Length;
struct Map;
struct IndexedTensor;
struct IndexExpr;
struct Call;
struct Load;
struct Neg;
struct Add;
struct Sub;
struct Mul;
struct Div;
struct Eq;
struct Ne;
struct Gt;
struct Lt;
struct Ge;
struct Le;
struct And;
struct Or;
struct Not;
struct Xor;

struct VarDecl;
struct AssignStmt;
struct FieldWrite;
struct TensorWrite;
struct Store;
struct ForRange;
struct For;
struct While;
struct IfThenElse;
struct Block;
struct Pass;
struct Print;

class Func;
class Stmt;
class Expr;

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
  virtual void visit(const VarExpr *op);
  virtual void visit(const FieldRead *op);
  virtual void visit(const TensorRead *op);
  virtual void visit(const TupleRead *op);
  virtual void visit(const IndexRead *op);
  virtual void visit(const Length *op);
  virtual void visit(const Load *op);
  virtual void visit(const IndexedTensor *op);
  virtual void visit(const IndexExpr *op);
  virtual void visit(const Call *op);

  virtual void visit(const Neg *op);
  virtual void visit(const Add *op);
  virtual void visit(const Sub *op);
  virtual void visit(const Mul *op);
  virtual void visit(const Div *op);

  virtual void visit(const Eq *op);
  virtual void visit(const Ne *op);
  virtual void visit(const Gt *op);
  virtual void visit(const Lt *op);
  virtual void visit(const Ge *op);
  virtual void visit(const Le *op);
  virtual void visit(const And *op);
  virtual void visit(const Or *op);
  virtual void visit(const Not *op);
  virtual void visit(const Xor *op);

  virtual void visit(const VarDecl *op);
  virtual void visit(const AssignStmt *op);
  virtual void visit(const Map *op);
  virtual void visit(const FieldWrite *op);
  virtual void visit(const TensorWrite *op);
  virtual void visit(const Store *op);
  virtual void visit(const ForRange *op);
  virtual void visit(const For *op);
  virtual void visit(const While *op);
  virtual void visit(const IfThenElse *op);
  virtual void visit(const Block *op);
  virtual void visit(const Pass *op);
  virtual void visit(const Print *op);

  virtual void visit(const Func *f);
};

/// Visits a whole call graph.
class IRVisitorCallGraph : public IRVisitor {
public:
  std::set<ir::Func> visited;
  virtual void visit(const Call *op);
};

/// Query class to make it easier to write visitors to answer a yes/no question.
class IRQuery : public IRVisitor {
public:
  IRQuery(bool init) : init(init) {}
  IRQuery() : IRQuery(false) {}

  bool query(const Expr &expr);
  bool query(const Stmt &stmt);

protected:
  bool result;

private:
  bool init;
};

}} // namespace simit::internal
#endif
