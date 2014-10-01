#ifndef SIMIT_SIR_PRINTER_H
#define SIMIT_SIR_PRINTER_H

#include "sir_visitor.h"
#include "sir.h"

#include <ostream>
#include <memory>

namespace simit {
namespace ir {

struct SetIRNode;

std::ostream &operator<<(std::ostream &os, const Stmt &stmt);
std::ostream &operator<<(std::ostream &os, const Expr &expr);

class SetIRPrinter : public SetIRConstVisitor {
public:
  SetIRPrinter(std::ostream &os, signed indent=0) : os(os), indentation(0) {}
  virtual ~SetIRPrinter();

  void print(const Expr &);
  void print(const Stmt &);

private:
  std::ostream &os;
  unsigned indentation;

  void indent();

  void visit(const IntLiteral *);
  void visit(const Variable *);
  void visit(const Load *);
  void visit(const Neg *);
  void visit(const Add *);
  void visit(const Sub *);
  void visit(const Mul *);
  void visit(const Div *);
  void visit(const Block *);
  void visit(const Foreach *);
  void visit(const Store *);
  void visit(const StoreMatrix *);
  void visit(const Pass *);
};

}} // namespace simit::ir
#endif
