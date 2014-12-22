#ifndef SIMIT_IR_PRINTER_H
#define SIMIT_IR_PRINTER_H

#include "ir_visitor.h"

#include <ostream>
#include <map>
#include <set>
#include <memory>

namespace simit {
namespace ir {

class Func;
class Expr;
class Stmt;
struct IRNode;
struct ForDomain;

std::ostream &operator<<(std::ostream &os, const Func &);
std::ostream &operator<<(std::ostream &os, const Expr &);
std::ostream &operator<<(std::ostream &os, const Stmt &);
std::ostream &operator<<(std::ostream &os, const IRNode &);
std::ostream &operator<<(std::ostream &os, const ForDomain &);

class IRPrinter : public IRVisitor {
public:
  IRPrinter(std::ostream &os, signed indent=0);
  virtual ~IRPrinter() {}

  void print(const Func &);
  void print(const Expr &);
  void print(const Stmt &);
  void print(const IRNode &);

private:
  virtual void visit(const Literal *);
  virtual void visit(const VarExpr *);
  virtual void visit(const FieldRead *);
  virtual void visit(const TensorRead *);
  virtual void visit(const TupleRead *);
  virtual void visit(const IndexRead *op);
  virtual void visit(const Length *op);
  virtual void visit(const Load *);
  virtual void visit(const IndexedTensor *);
  virtual void visit(const IndexExpr *);
  virtual void visit(const Call *);
  virtual void visit(const Neg *);
  virtual void visit(const Add *);
  virtual void visit(const Sub *);
  virtual void visit(const Mul *);
  virtual void visit(const Div *);

  virtual void visit(const AssignStmt *);
  virtual void visit(const Map *);
  virtual void visit(const FieldWrite *);
  virtual void visit(const TensorWrite *);
  virtual void visit(const Store *);
  virtual void visit(const For *);
  virtual void visit(const IfThenElse *);
  virtual void visit(const Block *);
  virtual void visit(const Pass *);

  virtual void visit(const Func *);

  void indent();

  std::ostream &os;
  unsigned indentation;
};

}} // namespace simit::ir
#endif
