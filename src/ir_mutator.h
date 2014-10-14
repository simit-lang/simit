#ifndef SIMIT_IR_MUTATOR_H
#define SIMIT_IR_MUTATOR_H

#include "ir.h"

namespace simit {
namespace ir {

class IRMutator : public IRVisitor {
public:
  virtual Expr mutate(Expr expr);
  virtual Stmt mutate(Stmt stmt);
  virtual Func mutate(Func func);

protected:
  /// visit methods that take Exprs assign to this to return their value.
  Expr expr;

  /// visit methods that take Stmts assign to this to return their value.
  Stmt stmt;

  /// visit methods that take Func assign to this to return their value.
  Func func;

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

  virtual void visit(const Func *);
};

}}

#endif
