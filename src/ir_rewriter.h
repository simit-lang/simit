#ifndef SIMIT_IR_REWRITER_H
#define SIMIT_IR_REWRITER_H

#include "ir.h"

namespace simit {
namespace ir {

class IRRewriter : public IRVisitor {
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

  virtual void visit(const Literal *op);
  virtual void visit(const VarExpr *op);
  virtual void visit(const Result *op);
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

  virtual void visit(const AssignStmt *op);
  virtual void visit(const Map *op);
  virtual void visit(const FieldWrite *op);
  virtual void visit(const TensorWrite *op);
  virtual void visit(const Store *op);
  virtual void visit(const For *op);
  virtual void visit(const IfThenElse *op);
  virtual void visit(const Block *op);
  virtual void visit(const Pass *op);

  virtual void visit(const Func *f);
};

}}

#endif
