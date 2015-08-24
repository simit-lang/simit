#ifndef SIMIT_IR_VISITOR_H
#define SIMIT_IR_VISITOR_H

#include <set>
#include "macros.h"

namespace simit {
namespace ir {

struct IRNode;

struct Literal;
struct VarExpr;
struct FieldRead;
struct TensorRead;
struct TupleRead;
struct IndexRead;
struct TensorIndexRead;
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
struct CallStmt;
struct ForRange;
struct For;
struct While;
struct IfThenElse;
struct Block;
struct Print;
struct Comment;
struct Pass;

#ifdef GPU
struct GPUKernel;
#endif

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
  virtual void visit(const Load *op);
  virtual void visit(const FieldRead *op);
  virtual void visit(const Call *op);
  virtual void visit(const Length *op);
  virtual void visit(const IndexRead *op);
  virtual void visit(const TensorIndexRead *op);

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
  virtual void visit(const Block *op);
  virtual void visit(const IfThenElse *op);
  virtual void visit(const ForRange *op);
  virtual void visit(const For *op);
  virtual void visit(const While *op);
  virtual void visit(const Print *op);
  virtual void visit(const Comment *op);
  virtual void visit(const Pass *op);

  /// High-level IRNodes that are lowered and never reach the backend
  virtual void visit(const TupleRead *op);
  virtual void visit(const TensorRead *op);
  virtual void visit(const TensorWrite *op);
  virtual void visit(const IndexedTensor *op);
  virtual void visit(const IndexExpr *op);
  virtual void visit(const Map *op);

#ifdef GPU
  virtual void visit(const GPUKernel *op);
#endif

  virtual void visit(const Func *f);
};

/// Visits a whole call graph.
class IRVisitorCallGraph : public IRVisitor {
public:
  std::set<ir::Func> visited;

  using IRVisitor::visit;

  virtual void visit(const Call *op);
  virtual void visit(const CallStmt *op);
  virtual void visit(const Map *op);
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


#define RULE(Rule)                                                             \
std::function<void(const Rule*)> Rule##Func;                                   \
void unpack(std::function<void(const Rule*)> pattern) {                        \
  Rule##Func = pattern;                                                        \
}                                                                              \
void visit(const Rule* op) {                                                   \
  if (Rule##Func) {                                                            \
    Rule##Func(op);                                                            \
  }                                                                            \
  IRVisitor::visit(op);                                                        \
}

class MatchVisitor : public IRVisitor {
public:

  template <class IR, class... Patterns>
  void process(IR ir, Patterns... patterns) {
    unpack(patterns...);
    ir.accept(this);
  }

  template <class First, class... Rest>
  void unpack(First first, Rest... rest) {
    unpack(first);
    unpack(rest...);
  }

  RULE(Literal)
  RULE(VarExpr)
  RULE(Load)
  RULE(FieldRead)
  RULE(Call)
  RULE(Length)
  RULE(IndexRead)
  RULE(TensorIndexRead)

  RULE(Neg)
  RULE(Add)
  RULE(Sub)
  RULE(Mul)
  RULE(Div)

  RULE(Not)
  RULE(Eq)
  RULE(Ne)
  RULE(Gt)
  RULE(Lt)
  RULE(Ge)
  RULE(Le)
  RULE(And)
  RULE(Or)
  RULE(Xor)

  RULE(VarDecl)
  RULE(AssignStmt)
  RULE(Store)
  RULE(FieldWrite)
  RULE(CallStmt)
  RULE(Block)
  RULE(IfThenElse)
  RULE(ForRange)
  RULE(For)
  RULE(While)
  RULE(Print)
  RULE(Comment)
  RULE(Pass)

  RULE(TupleRead)
  RULE(TensorRead)
  RULE(TensorWrite)
  RULE(IndexedTensor)
  RULE(IndexExpr)
  RULE(Map)

  RULE(Func)
};

/**
\example Print all Add and AssignStmt objects in func. Use closures to
          capture environment variables (e.g. [&]).
\code{.cpp}
match(func,
  std::function<void(const Add*)>([](const Add* op) {
    std::cout << *op << std::endl;
  })
  ,
  std::function<void(const AssignStmt*)>([](const AssignStmt* op) {
    std::cout << *op << std::endl;
  })
);
\endcode
**/
template <class IR, class... Patterns>
void match(IR ir, Patterns... patterns) {
  MatchVisitor().process(ir, patterns...);
}

}} // namespace simit::internal
#endif
