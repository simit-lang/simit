#ifndef SIMIT_IR_PRINTER_H
#define SIMIT_IR_PRINTER_H

#include "ir_visitor.h"

#include <ostream>
#include <map>
#include <set>
#include <memory>

namespace simit {
namespace ir {

struct IRNode;
struct ForDomain;
enum class CompoundOperator;

class IRPrinter : private IRVisitorStrict {
public:
  IRPrinter(std::ostream &os, signed indent=0);
  virtual ~IRPrinter() {}

  void print(const Func &);
  void print(const Expr &);
  void print(const Stmt &);
  void print(const IRNode &);

  void skipTopExprParenthesis();

private:
  using IRVisitorStrict::visit;
  virtual void visit(const Literal *op);
  virtual void visit(const VarExpr *op);
  virtual void visit(const Load *op);
  virtual void visit(const FieldRead *op);
  virtual void visit(const Call *op);
  virtual void visit(const Length *op);
  virtual void visit(const IndexRead *op);

  virtual void visit(const Neg *op);
  virtual void visit(const Add *op);
  virtual void visit(const Sub *op);
  virtual void visit(const Mul *op);
  virtual void visit(const Div *op);

  virtual void visit(const Not *op);
  virtual void visit(const Eq *op);
  virtual void visit(const Ne *op);
  virtual void visit(const Gt *op);
  virtual void visit(const Lt *op);
  virtual void visit(const Ge *op);
  virtual void visit(const Le *op);
  virtual void visit(const And *op);
  virtual void visit(const Or *op);
  virtual void visit(const Xor *op);

  virtual void visit(const VarDecl *op);
  virtual void visit(const AssignStmt *op);
  virtual void visit(const CallStmt *op);
  virtual void visit(const Store *op);
  virtual void visit(const FieldWrite *op);
  virtual void visit(const Scope *op);
  virtual void visit(const IfThenElse *op);
  virtual void visit(const ForRange *op);
  virtual void visit(const For *op);
  virtual void visit(const While *op);
  virtual void visit(const Kernel *op);
  virtual void visit(const Block *op);
  virtual void visit(const Print *op);
  virtual void visit(const Comment *op);
  virtual void visit(const Pass *op);

  virtual void visit(const TupleRead *op);
  virtual void visit(const SetRead *op);
  virtual void visit(const TensorRead *op);
  virtual void visit(const TensorWrite *op);
  virtual void visit(const IndexedTensor *op);
  virtual void visit(const IndexExpr *op);
  virtual void visit(const Map *op);

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
