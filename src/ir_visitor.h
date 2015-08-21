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
  virtual void visit(const FieldRead *op);
  virtual void visit(const TensorRead *op);
  virtual void visit(const TupleRead *op);
  virtual void visit(const IndexRead *op);
  virtual void visit(const TensorIndexRead *op);
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
  virtual void visit(const CallStmt *op);
  virtual void visit(const Map *op);
  virtual void visit(const FieldWrite *op);
  virtual void visit(const TensorWrite *op);
  virtual void visit(const Store *op);
  virtual void visit(const ForRange *op);
  virtual void visit(const For *op);
  virtual void visit(const While *op);
  virtual void visit(const IfThenElse *op);
  virtual void visit(const Block *op);
  virtual void visit(const Print *op);
  virtual void visit(const Comment *op);
  virtual void visit(const Pass *op);

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


#define VISIT(Type) \
void visit(const Type *op) { \
  if (patterns.find(#Type) != patterns.end()) { \
    bool cont = patterns.at(#Type)(op); \
    if (cont) { \
      IRVisitor::visit(op);\
    } \
  } \
  else { \
    IRVisitor::visit(op); \
  } \
}

typedef std::map<std::string, std::function<bool(const IRNode*)>> MatchPatterns;

/// @example Print all AssignStmts in Func
///   match<Func>(func, {
///     {"AssignStmt", [&storage](const IRNode* irNode) -> bool {
///       const AssignStmt* op = static_cast<const AssignStmt*>(op);
///       std::cout << *op << std::endl;
///       return false;
///     }},
///   });
template <class IR>
void match(IR ir, const MatchPatterns &patterns) {

  class MatchVisitor : public IRVisitor {
  public:
    void process(const IR& ir, const MatchPatterns &patterns) {
#ifdef SIMIT_ASSERTS
      std::set<std::string> patternStrings = {
        "Literal",
        "VarExpr",
        "FieldRead",
        "TensorRead",
        "TupleRead",
        "IndexRead",
        "TensorIndexRead",
        "Length",
        "Load",
        "IndexedTensor",
        "IndexExpr",
        "Call",
        "Neg",
        "Add",
        "Sub",
        "Mul",
        "Div",
        "Eq",
        "Ne",
        "Gt",
        "Lt",
        "Ge",
        "Le",
        "And",
        "Or",
        "Not",
        "Xor",
        "VarDecl",
        "AssignStmt",
        "CallStmt",
        "Map",
        "FieldWrite",
        "TensorWrite",
        "Store",
        "ForRange",
        "For",
        "While",
        "IfThenElse",
        "Block",
        "Print",
        "Comment",
        "Pass",
        "GPUKernel"};
      for (auto &pattern : patterns) {
        iassert(patternStrings.find(pattern.first) != patternStrings.end())
            << "undefined pattern";
      }
#endif

      this->patterns = patterns;
      ir.accept(this);
    }

  private:
    using IRVisitor::visit;

    MatchPatterns patterns;
    VISIT(Literal)
    VISIT(VarExpr)
    VISIT(FieldRead)
    VISIT(TensorRead)
    VISIT(TupleRead)
    VISIT(IndexRead)
    VISIT(TensorIndexRead)
    VISIT(Length)
    VISIT(Load)
    VISIT(IndexedTensor)
    VISIT(IndexExpr)
    VISIT(Call)

    VISIT(Neg)
    VISIT(Add)
    VISIT(Sub)
    VISIT(Mul)
    VISIT(Div)

    VISIT(Eq)
    VISIT(Ne)
    VISIT(Gt)
    VISIT(Lt)
    VISIT(Ge)
    VISIT(Le)
    VISIT(And)
    VISIT(Or)
    VISIT(Not)
    VISIT(Xor)

    VISIT(VarDecl)
    VISIT(AssignStmt)
    VISIT(CallStmt)
    VISIT(Map)
    VISIT(FieldWrite)
    VISIT(TensorWrite)
    VISIT(Store)
    VISIT(ForRange)
    VISIT(For)
    VISIT(While)
    VISIT(IfThenElse)
    VISIT(Block)
    VISIT(Print)
    VISIT(Comment)
    VISIT(Pass)
#ifdef GPU
    VISIT(GPUKernel)
#endif
  };

  MatchVisitor().process(ir, patterns);
};

}} // namespace simit::internal
#endif
