#ifndef SIMIT_SIR_VISITOR_H
#define SIMIT_SIR_VISITOR_H

namespace simit {
namespace ir {

struct IntLiteral;
struct Variable;
struct Load;
struct Neg;
struct Add;
struct Sub;
struct Mul;
struct Div;
struct Block;
struct Foreach;
struct Store;
struct StoreMatrix;
struct Pass;

class SetIRVisitor {
public:
  virtual ~SetIRVisitor();
  virtual void visit(IntLiteral *);
  virtual void visit(Variable *);
  virtual void visit(Load *);
  virtual void visit(Neg *);
  virtual void visit(Add *);
  virtual void visit(Sub *);
  virtual void visit(Mul *);
  virtual void visit(Div *);
  virtual void visit(Block *);
  virtual void visit(Foreach *);
  virtual void visit(Store *);
  virtual void visit(StoreMatrix *);
  virtual void visit(Pass *);
};

class SetIRConstVisitor {
public:
  virtual ~SetIRConstVisitor();
  virtual void visit(const IntLiteral *);
  virtual void visit(const Variable *);
  virtual void visit(const Load *);
  virtual void visit(const Neg *);
  virtual void visit(const Add *);
  virtual void visit(const Sub *);
  virtual void visit(const Mul *);
  virtual void visit(const Div *);
  virtual void visit(const Block *);
  virtual void visit(const Foreach *);
  virtual void visit(const Store *);
  virtual void visit(const StoreMatrix *);
  virtual void visit(const Pass *);
};

}} // namespace simit::ir

#endif
