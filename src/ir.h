#ifndef SIMIT_IR_H
#define SIMIT_IR_H

#include <string>

#include "intrusive_ptr.h"
#include "uncopyable.h"
#include "var.h"
#include "types.h"
#include "func.h"
#include "ir_visitor.h"
#include "indexvar.h"

namespace simit {
namespace ir {

/// The base class of all nodes in the Simit Intermediate Representation
/// (Simit IR)
struct IRNode : private simit::interfaces::Uncopyable {
public:
  IRNode() {}
  virtual ~IRNode() {}
  virtual void accept(IRVisitorStrict *visitor) const = 0;

private:
  mutable long ref = 0;
  friend void aquire(const IRNode *node) {++node->ref;}
  friend void release(const IRNode *node) {if (--node->ref == 0) delete node;}
};

std::ostream &operator<<(std::ostream &os, const IRNode &);

struct ExprNodeBase : public IRNode {
  Type type;
};

struct StmtNodeBase : public IRNode {
};

template <typename T>
struct ExprNode : public ExprNodeBase {
  void accept(IRVisitorStrict *v) const { v->visit((const T *)this); }
};

template <typename T>
struct StmtNode : public StmtNodeBase {
  void accept(IRVisitorStrict *v) const { v->visit((const T *)this); }
};

struct IRHandle : util::IntrusivePtr<const IRNode> {
  IRHandle() : util::IntrusivePtr<const IRNode>() {}
  IRHandle(const IRNode *n) : util::IntrusivePtr<const IRNode>(n) {}
};

class Expr : public IRHandle {
public:
  Expr() : IRHandle() {}
  Expr(const ExprNodeBase *expr) : IRHandle(expr) {}
  Expr(const Var &var);

  Expr(int val);
  Expr(double val);

  Type type() const {return static_cast<const ExprNodeBase*>(ptr)->type;}

  void accept(IRVisitorStrict *v) const {ptr->accept(v);}

  Expr operator()(const std::vector<IndexVar> &indexVars) const;

  template <typename ...IndexVars>
  Expr operator()(const IndexVars& ...indexVars) const {
    return this->operator()({indexVars...});
  }

  template <typename E> friend bool isa(Expr);
  template <typename E> friend const E* to(Expr);
};

std::ostream &operator<<(std::ostream &os, const Expr &);

Expr operator-(Expr);
Expr operator+(Expr, Expr);
Expr operator-(Expr, Expr);
Expr operator*(Expr, Expr);
Expr operator/(Expr, Expr);

template <typename E>
inline bool isa(Expr e) {
  return e.defined() && dynamic_cast<const E*>(e.ptr) != nullptr;
}

template <typename E>
inline const E* to(Expr e) {
  iassert(isa<E>(e)) << "Wrong Expr type";
  return static_cast<const E*>(e.ptr);
}


class Stmt : public IRHandle {
public:
  Stmt() : IRHandle() {}
  Stmt(const StmtNodeBase *stmt) : IRHandle(stmt) {}

  void accept(IRVisitorStrict *v) const {ptr->accept(v);}

  template <typename S> friend bool isa(Stmt);
  template <typename S> friend const S* to(Stmt);
};

std::ostream &operator<<(std::ostream &os, const Stmt &);

template <typename S>
inline bool isa(Stmt s) {
  return s.defined() && dynamic_cast<const S*>(s.ptr) != nullptr;
}

template <typename S>
inline const S* to(Stmt s) {
  iassert(isa<S>(s)) << "Wrong Expr type " << s;
  return static_cast<const S*>(s.ptr);
}


// Type compute functions
Type getFieldType(Expr elementOrSet, std::string fieldName);
Type getBlockType(Expr tensor);
Type getIndexExprType(std::vector<IndexVar> lhsIndexVars, Expr expr);


/// CompoundOperator used with AssignStmt, TensorWrite, FieldWrite and Store.
enum class CompoundOperator { None, Add };
std::ostream &operator<<(std::ostream &os, const CompoundOperator &);


/// Represents a \ref Tensor that is defined as a constant or loaded.  Note
/// that it is only possible to define dense tensor literals.
struct Literal : public ExprNode<Literal> {
  void *data;
  size_t size;

  void cast(Type type);
  double getFloatVal(int index) const;

  static Expr make(Type type);
  static Expr make(int val);
  static Expr make(double val);
  static Expr make(bool val);
  static Expr make(Type type, void* values);
  static Expr make(Type type, std::vector<double> values);
  ~Literal();
};
bool operator==(const Literal& l, const Literal& r);
bool operator!=(const Literal& l, const Literal& r);


struct VarExpr : public ExprNode<VarExpr> {
  Var var;
  static Expr make(Var var);
};

/// Expression that loads a scalar from a buffer. A buffer is a one-dimensional
/// tensor that is indexed by an integer range.
struct Load : public ExprNode<Load> {
  Expr buffer;
  Expr index;
  static Expr make(Expr buffer, Expr index);
};

/// Expression that reads a tensor from an element or set field.
struct FieldRead : public ExprNode<FieldRead> {
  Expr elementOrSet;
  std::string fieldName;
  static Expr make(Expr elementOrSet, std::string fieldName);
};

struct Call : public ExprNode<Call> {
  Func func;
  std::vector<Expr> actuals;
  static Expr make(Func func, std::vector<Expr> actuals);
};

struct Length : public ExprNode<Length> {
  IndexSet indexSet;
  static Expr make(IndexSet indexSet);
};

/// An IndexRead retrieves an index from an edge set.  An example of an index
/// is the endpoints of the edges in the set.
/// TODO DEPRECATED: This node has been deprecated with the old lowering pass
struct IndexRead : public ExprNode<IndexRead> {
  enum Kind { Endpoints=0, NeighborsStart=1, Neighbors=2 };
  Expr edgeSet;
  Kind kind;
  static Expr make(Expr edgeSet, Kind kind);
};

struct UnaryExpr {
  Expr a;
};

struct BinaryExpr {
  Expr a, b;
};

struct Neg : public UnaryExpr, public ExprNode<Neg> {
  static Expr make(Expr a);
};

struct Add : public BinaryExpr, public ExprNode<Add> {
  static Expr make(Expr a, Expr b);
};

struct Sub : public BinaryExpr, public ExprNode<Sub> {
  static Expr make(Expr a, Expr b);
};

struct Mul : public BinaryExpr, public ExprNode<Mul> {
  static Expr make(Expr a, Expr b);
};

struct Div : public BinaryExpr, public ExprNode<Div> {
  static Expr make(Expr a, Expr b);
};

struct Not : public UnaryExpr, public ExprNode<Not> {
  static Expr make(Expr a);
};

struct Eq : public BinaryExpr, public ExprNode<Eq> {
  static Expr make(Expr a, Expr b);
};

struct Ne : public BinaryExpr, public ExprNode<Ne> {
  static Expr make(Expr a, Expr b);
};

struct Gt : public BinaryExpr, public ExprNode<Gt> {
  static Expr make(Expr a, Expr b);
};

struct Lt : public BinaryExpr, public ExprNode<Lt> {
  static Expr make(Expr a, Expr b);
};

struct Ge : public BinaryExpr, public ExprNode<Ge> {
  static Expr make(Expr a, Expr b);
};

struct Le : public BinaryExpr, public ExprNode<Le> {
  static Expr make(Expr a, Expr b);
};

struct And : public BinaryExpr, public ExprNode<And> {
  static Expr make(Expr a, Expr b);
};

struct Or : public BinaryExpr, public ExprNode<Or> {
  static Expr make(Expr a, Expr b);
};

struct Xor : public BinaryExpr, public ExprNode<Xor> {
  static Expr make(Expr a, Expr b);
};

struct VarDecl : public StmtNode<VarDecl> {
  Var var;
  static Stmt make(Var var);
};

struct AssignStmt : public StmtNode<AssignStmt> {
  Var var;
  Expr value;
  CompoundOperator cop;
  static Stmt make(Var var, Expr value);
  static Stmt make(Var var, Expr value, CompoundOperator cop);
};

struct Store : public StmtNode<Store> {
  Expr buffer;
  Expr index;
  Expr value;
  CompoundOperator cop;
  static Stmt make(Expr buffer, Expr index, Expr value,
                   CompoundOperator cop=CompoundOperator::None);
};

struct FieldWrite : public StmtNode<FieldWrite> {
  Expr elementOrSet;
  std::string fieldName;
  Expr value;
  CompoundOperator cop;
  static Stmt make(Expr elementOrSet, std::string fieldName, Expr value,
                   CompoundOperator cop=CompoundOperator::None);
};

struct CallStmt : public StmtNode<CallStmt> {
  std::vector<Var> results;
  Func callee;
  std::vector<Expr> actuals;
  static Stmt make(std::vector<Var> results,
                   Func callee, std::vector<Expr> actuals);
};

struct Scope : public StmtNode<Scope> {
  Stmt scopedStmt;
  static Stmt make(Stmt scopedStmt);
};

struct IfThenElse : public StmtNode<IfThenElse> {
  Expr condition;
  Stmt thenBody, elseBody;
  static Stmt make(Expr condition, Stmt thenBody);
  static Stmt make(Expr condition, Stmt thenBody, Stmt elseBody);
};

struct ForRange : public StmtNode<ForRange> {
  Var var;
  Expr start;
  Expr end;
  Stmt body;
  static Stmt make(Var var, Expr start, Expr end, Stmt body);
};

struct ForDomain {
  enum Kind { IndexSet, Endpoints, Edges, Neighbors, NeighborsOf, Diagonal };
  Kind kind;

  /// An index set
  class IndexSet indexSet;

  /// A lookup in the index structures of an edge set
  Expr set;
  Var var;

  ForDomain() {}
  ForDomain(class IndexSet indexSet) : kind(IndexSet), indexSet(indexSet) {}
  ForDomain(Expr set, Var var, Kind kind) : kind(kind), set(set), var(var) {
    iassert(kind != IndexSet);
  }
  ForDomain(Expr set, Var var, Kind kind, class IndexSet indexSet) : kind(kind),
      indexSet(indexSet), set(set), var(var)  {
    iassert(kind == NeighborsOf);
  }
};
std::ostream &operator<<(std::ostream &os, const ForDomain &);

// TODO DEPRECATED: Remove when new index system is in place.
struct For : public StmtNode<For> {
  Var var;
  ForDomain domain;
  Stmt body;
  static Stmt make(Var var, ForDomain domain, Stmt body);
};

struct While : public StmtNode<While> {
  Expr condition;
  Stmt body;
  static Stmt make(Expr condition, Stmt body);
};

struct Kernel : public StmtNode<Kernel> {
  Var var;
  IndexDomain domain;
  Stmt body;
  static Stmt make(Var var, IndexDomain domain, Stmt body);
};

struct Block : public StmtNode<Block> {
  Stmt first, rest;
  static Stmt make(Stmt first, Stmt rest);
  static Stmt make(std::vector<Stmt> stmts);
};

struct Print : public StmtNode<Print> {
  Expr expr;
  std::string str;
  std::string format;
  static Stmt make(Expr expr, std::string format="");
  static Stmt make(std::string str);
};

/// A comment, that can optionally be applied to a statement with footer and
/// header space.
struct Comment : public StmtNode<Comment> {
  std::string comment;
  Stmt commentedStmt;
  bool footerSpace;
  bool headerSpace;
  static Stmt make(std::string comment, Stmt commentedStmt=Stmt(),
                   bool footerSpace=false, bool headerSpace=false);
};

/// Empty statement that is convenient during code development.
struct Pass : public StmtNode<Pass> {
  static Stmt make();
};

struct TupleRead : public ExprNode<TupleRead> {
  Expr tuple, index;
  static Expr make(Expr tuple, Expr index);
};

/// Expression that reads a tensor from an n-dimensional tensor location.
struct TensorRead : public ExprNode<TensorRead> {
  Expr tensor;
  std::vector<Expr> indices;

  /// Construct a tensor read that reads a block from the location in `tensor`
  /// specified by `indices`. The caller must either provide one or n indices,
  /// where n is the tensor order. If one index is provided then the tensor read
  /// has already been flattened, and will be directly lowered to a load.
  static Expr make(Expr tensor, std::vector<Expr> indices);
};

struct TensorWrite : public StmtNode<TensorWrite> {
  // TODO: Consider whether to make tensor a Var
  Expr tensor;
  std::vector<Expr> indices;
  Expr value;
  CompoundOperator cop;
  static Stmt make(Expr tensor, std::vector<Expr> indices, Expr value,
                   CompoundOperator cop=CompoundOperator::None);
};

struct IndexedTensor : public ExprNode<IndexedTensor> {
  Expr tensor;
  std::vector<IndexVar> indexVars;

  static Expr make(Expr tensor, std::vector<IndexVar> indexVars);
};

struct IndexExpr : public ExprNode<IndexExpr> {
  std::vector<IndexVar> resultVars;
  Expr value;
  std::vector<IndexVar> domain() const;

  static Expr make(std::vector<IndexVar> resultVars, Expr value);
};

struct Map : public StmtNode<Map> {
  std::vector<Var> vars;
  Func function;
  Expr target;
  Expr neighbors;
  std::vector<Expr> partial_actuals;
  ReductionOperator reduction;

  static Stmt make(std::vector<Var> vars,
                   Func function, std::vector<Expr> partial_actuals,
                   Expr target, Expr neighbors=Expr(),
                   ReductionOperator reduction=ReductionOperator());
};

}} // namespace simit::ir

#endif
