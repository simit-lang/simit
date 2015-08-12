#ifndef SIMIT_IR_PRINTER_H
#define SIMIT_IR_PRINTER_H

#include "ir_visitor.h"

#include <ostream>
#include <map>
#include <set>
#include <memory>

namespace simit {
namespace ir {

struct Environment;
struct IRNode;
struct ForDomain;
enum class CompoundOperator;

std::ostream &operator<<(std::ostream &os, const Environment &);
std::ostream &operator<<(std::ostream &os, const Func &);

std::ostream &operator<<(std::ostream &os, const Expr &);
std::ostream &operator<<(std::ostream &os, const Stmt &);
std::ostream &operator<<(std::ostream &os, const IRNode &);
std::ostream &operator<<(std::ostream &os, const ForDomain &);
std::ostream &operator<<(std::ostream &os, const CompoundOperator &);

class IRPrinter : private IRVisitor {
public:
  IRPrinter(std::ostream &os, signed indent=0);
  virtual ~IRPrinter() {}

  void print(const Func &);
  void print(const Expr &);
  void print(const Stmt &);
  void print(const IRNode &);

  void skipTopExprParenthesis();

private:
  using IRVisitor::visit;
  virtual void visit(const Literal *);
  virtual void visit(const VarExpr *);
  virtual void visit(const FieldRead *);
  virtual void visit(const TensorRead *);
  virtual void visit(const TupleRead *);
  virtual void visit(const IndexRead *);
  virtual void visit(const TensorIndexRead *);
  virtual void visit(const Length *);
  virtual void visit(const Load *);
  virtual void visit(const IndexedTensor *);
  virtual void visit(const IndexExpr *);
  virtual void visit(const Call *);
  virtual void visit(const Neg *);
  virtual void visit(const Add *);
  virtual void visit(const Sub *);
  virtual void visit(const Mul *);
  virtual void visit(const Div *);

  virtual void visit(const Eq *);
  virtual void visit(const Ne *);
  virtual void visit(const Gt *);
  virtual void visit(const Lt *);
  virtual void visit(const Ge *);
  virtual void visit(const Le *);
  virtual void visit(const And *);
  virtual void visit(const Or *);
  virtual void visit(const Not *);
  virtual void visit(const Xor *);

  virtual void visit(const VarDecl *);
  virtual void visit(const AssignStmt *);
  virtual void visit(const CallStmt *);
  virtual void visit(const Map *);
  virtual void visit(const FieldWrite *);
  virtual void visit(const TensorWrite *);
  virtual void visit(const Store *);
  virtual void visit(const ForRange *);
  virtual void visit(const For *);
  virtual void visit(const While *);
  virtual void visit(const IfThenElse *);
  virtual void visit(const Block *);
  virtual void visit(const Print *);
  virtual void visit(const Comment *);
  virtual void visit(const Pass *);

#ifdef GPU
  virtual void visit(const GPUKernel *);
#endif

  virtual void visit(const Func *);

  void indent();

  struct ParenPrinter {
    std::ostream &os;
    bool skipParen;
    ParenPrinter(IRPrinter *irPrinter) : os(irPrinter->os) {
      skipParen = irPrinter->skipTopExprParen;
      irPrinter->skipTopExprParen = false;
      if (!skipParen) os << "(";
    };
    ~ParenPrinter() {
      if (!skipParen) os << ")";
    }
  };
  friend ParenPrinter;

  ParenPrinter paren() {return ParenPrinter(this); }
  void clearSkipParen() {skipTopExprParen = false;}

  std::ostream &os;
  unsigned indentation;
  bool skipTopExprParen = false;
};

class IRPrinterCallGraph : public IRVisitor {
public:
  IRPrinterCallGraph(std::ostream &os) : os(os) {}
  void print(const Func &);

private:
  std::set<ir::Func> visited;
  std::ostream &os;
  
  using IRVisitor::visit;

  virtual void visit(const Call *);
  virtual void visit(const CallStmt *);
  virtual void visit(const Map *);
  virtual void visit(const Func *);
};

void printCallGraph(const Func &f, std::ostream &os);

}} // namespace simit::ir
#endif
