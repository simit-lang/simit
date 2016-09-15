#ifndef SIMIT_IR_VISITOR_H
#define SIMIT_IR_VISITOR_H

#include <set>
#include <functional>
#include "error.h"

namespace simit {
namespace ir {

class Func;
class Stmt;
class Expr;

struct Literal;
struct VarExpr;
struct Load;
struct FieldRead;
struct Length;
struct IndexRead;

struct UnaryExpr;
struct BinaryExpr;

struct Neg;
struct Add;
struct Sub;
struct Mul;
struct Div;
struct Rem;

struct Not;
struct Eq;
struct Ne;
struct Gt;
struct Lt;
struct Ge;
struct Le;
struct And;
struct Or;
struct Xor;

struct VarDecl;
struct AssignStmt;
struct CallStmt;
struct Store;
struct FieldWrite;
struct Scope;
struct IfThenElse;
struct ForRange;
struct For;
struct While;
struct Kernel;
struct Block;
struct Print;
struct Comment;
struct Pass;

struct UnnamedTupleRead;
struct NamedTupleRead;
struct SetRead;
struct TensorRead;
struct TensorWrite;
struct IndexedTensor;
struct IndexExpr;
struct Map;

#ifdef GPU
struct GPUKernel;
#endif

/// IR visitor without default implementations. Sub-classes must therefore
/// override all Expr and Stmt visit methods. For a visitor with default
/// implementations see IRVisitor.
class IRVisitorStrict {
public:
  virtual void visit(const Literal* op) = 0;
  virtual void visit(const VarExpr* op) = 0;
  virtual void visit(const Load* op) = 0;
  virtual void visit(const FieldRead* op) = 0;
  virtual void visit(const Length* op) = 0;
  virtual void visit(const IndexRead* op) = 0;

  virtual void visit(const Neg* op) = 0;
  virtual void visit(const Add* op) = 0;
  virtual void visit(const Sub* op) = 0;
  virtual void visit(const Mul* op) = 0;
  virtual void visit(const Div* op) = 0;
  virtual void visit(const Rem* op) = 0;

  virtual void visit(const Not* op) = 0;
  virtual void visit(const Eq* op) = 0;
  virtual void visit(const Ne* op) = 0;
  virtual void visit(const Gt* op) = 0;
  virtual void visit(const Lt* op) = 0;
  virtual void visit(const Ge* op) = 0;
  virtual void visit(const Le* op) = 0;
  virtual void visit(const And* op) = 0;
  virtual void visit(const Or* op) = 0;
  virtual void visit(const Xor* op) = 0;

  virtual void visit(const VarDecl* op) = 0;
  virtual void visit(const AssignStmt* op) = 0;
  virtual void visit(const CallStmt* op) = 0;
  virtual void visit(const Store* op) = 0;
  virtual void visit(const FieldWrite* op) = 0;
  virtual void visit(const Scope* op) = 0;
  virtual void visit(const IfThenElse* op) = 0;
  virtual void visit(const ForRange* op) = 0;
  virtual void visit(const For* op) = 0;
  virtual void visit(const While* op) = 0;
  virtual void visit(const Kernel *op) = 0;
  virtual void visit(const Block* op) = 0;
  virtual void visit(const Print* op) = 0;
  virtual void visit(const Comment* op) = 0;
  virtual void visit(const Pass* op) = 0;

  /// High-level IRNodes that are lowered and never reach the backend
  virtual void visit(const UnnamedTupleRead* op) = 0;
  virtual void visit(const NamedTupleRead* op) = 0;
  virtual void visit(const SetRead* op) = 0;
  virtual void visit(const TensorRead* op) = 0;
  virtual void visit(const TensorWrite* op) = 0;
  virtual void visit(const IndexedTensor* op) = 0;
  virtual void visit(const IndexExpr* op) = 0;
  virtual void visit(const Map* op) = 0;

  virtual void visit(const Func* f) {}

#ifdef GPU
  // NOTE: GPU-specific visitors are not declared pure virtual,
  // because they are backend-specific. This should be removed when
  // switching to a more general parallel IR representation.
  virtual void visit(const GPUKernel* op) {
    ierror << "GPUKernel visitor must be implemented.";
  }
#endif
};

/// IR visitor with default implementations that recursively visits the IR.
class IRVisitor : public IRVisitorStrict {
public:
  virtual ~IRVisitor();

  using IRVisitorStrict::visit;
  virtual void visit(const Literal *op);
  virtual void visit(const VarExpr *op);
  virtual void visit(const Load *op);
  virtual void visit(const FieldRead *op);
  virtual void visit(const Length *op);
  virtual void visit(const IndexRead *op);

  virtual void visit(const UnaryExpr* op);
  virtual void visit(const BinaryExpr* op);

  virtual void visit(const Neg *op);
  virtual void visit(const Add *op);
  virtual void visit(const Sub *op);
  virtual void visit(const Mul *op);
  virtual void visit(const Div *op);
  virtual void visit(const Rem *op);

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
  virtual void visit(const Scope* op);
  virtual void visit(const IfThenElse *op);
  virtual void visit(const ForRange *op);
  virtual void visit(const For *op);
  virtual void visit(const While *op);
  virtual void visit(const Kernel *op);
  virtual void visit(const Block *op);
  virtual void visit(const Print *op);
  virtual void visit(const Comment *op);
  virtual void visit(const Pass *op);

  /// High-level IRNodes that are lowered and never reach the backend
  virtual void visit(const UnnamedTupleRead *op);
  virtual void visit(const NamedTupleRead *op);
  virtual void visit(const SetRead *op);
  virtual void visit(const TensorRead *op);
  virtual void visit(const TensorWrite *op);
  virtual void visit(const IndexedTensor *op);
  virtual void visit(const IndexExpr *op);
  virtual void visit(const Map *op);

  virtual void visit(const Func *f);

#ifdef GPU
  virtual void visit(const GPUKernel* op);
#endif
};

/// Visits a whole call graph.
class IRVisitorCallGraph : public IRVisitor {
public:
  std::set<ir::Func> visited;

  using IRVisitor::visit;

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
std::function<void(const Rule*, Matcher*)> Rule##CtxFunc;                      \
void unpack(std::function<void(const Rule*)> pattern) {                        \
  iassert(!Rule##CtxFunc && !Rule##Func);                                      \
  Rule##Func = pattern;                                                        \
}                                                                              \
void unpack(std::function<void(const Rule*, Matcher*)> pattern) {              \
  iassert(!Rule##CtxFunc && !Rule##Func);                                      \
  Rule##CtxFunc = pattern;                                                     \
}                                                                              \
void visit(const Rule* op) {                                                   \
  if (Rule##Func) {                                                            \
    Rule##Func(op);                                                            \
  }                                                                            \
  else if (Rule##CtxFunc) {                                                    \
    Rule##CtxFunc(op, this);                                                   \
    return;                                                                    \
  }                                                                            \
  IRVisitor::visit(op);                                                        \
}

class Matcher : public IRVisitor {
public:
  template <class IR>
  void match(IR ir) {
    ir.accept(this);
  }

  template <class IR, class... Patterns>
  void process(IR ir, Patterns... patterns) {
    unpack(patterns...);
    ir.accept(this);
  }

private:
  template <class First, class... Rest>
  void unpack(First first, Rest... rest) {
    unpack(first);
    unpack(rest...);
  }

  RULE(Literal)
  RULE(VarExpr)
  RULE(Load)
  RULE(FieldRead)
  RULE(Length)
  RULE(IndexRead)

  RULE(UnaryExpr)
  RULE(BinaryExpr)

  RULE(Neg)
  RULE(Add)
  RULE(Sub)
  RULE(Mul)
  RULE(Div)
  RULE(Rem)

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
  RULE(Scope)
  RULE(IfThenElse)
  RULE(ForRange)
  RULE(For)
  RULE(While)
  RULE(Kernel)
  RULE(Block)
  RULE(Print)
  RULE(Comment)
  RULE(Pass)

  RULE(UnnamedTupleRead)
  RULE(NamedTupleRead)
  RULE(SetRead)
  RULE(TensorRead)
  RULE(TensorWrite)
  RULE(IndexedTensor)
  RULE(IndexExpr)
  RULE(Map)

  RULE(Func)

#ifdef GPU
  RULE(GPUKernel)
#endif
};

/**
Match patterns to the IR.

For example, to print all Add and AssignStmt objects in func. Use closures to
capture environment variables (e.g. [&]):
~~~~~~~~~~~~~~~{.cpp}
match(func,
  std::function<void(const Add*)>([](const Add* op) {
    // ...
  })
  ,
  std::function<void(const AssignStmt*)>([](const AssignStmt* op) {
    // ...
  })
);
~~~~~~~~~~~~~~~

Alternatively, mathing rules can also accept a Context to be used to match
sub-expressions:
~~~~~~~~~~~~~~~{.cpp}
match(func,
  std:;function<void(const Add*,Matcher* ctx)>([&](const Add* op, Matcher* ctx){
    ctx->match(op->a);
  })
);
~~~~~~~~~~~~~~~

function<void(const Add*, Matcher* ctx)>([&](const Add* op, Matcher* ctx) {
**/
template <class IR, class... Patterns>
void match(IR ir, Patterns... patterns) {
  Matcher().process(ir, patterns...);
}

}} // namespace simit::internal
#endif
