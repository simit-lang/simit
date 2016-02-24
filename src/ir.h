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
#include "complex_types.h"

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

struct ExprNode : public IRNode {
  Type type;
};

struct StmtNode : public IRNode {
};

struct IRHandle : util::IntrusivePtr<const IRNode> {
  IRHandle() : util::IntrusivePtr<const IRNode>() {}
  IRHandle(const IRNode *n) : util::IntrusivePtr<const IRNode>(n) {}
};

class Expr : public IRHandle {
public:
  Expr() : IRHandle() {}
  Expr(const ExprNode *expr) : IRHandle(expr) {}
  Expr(const Var &var);

  Expr(int val);
  Expr(double val);
  Expr(double_complex val);

  Type type() const {return static_cast<const ExprNode*>(ptr)->type;}

  void accept(IRVisitorStrict *v) const {
    try {
      ptr->accept(v);
    }
    catch (SimitException &ex) {
      ex.addContext("... accepting: ");
      ex.errStream << *ptr;
      throw;
    }
  }

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
  Stmt(const StmtNode *stmt) : IRHandle(stmt) {}

  void accept(IRVisitorStrict *v) const {
    try {
      ptr->accept(v);
    }
    catch (SimitException &ex) {
      if (!isa<Block>(*this) && !isa<Scope>(*this)) {
        ex.addContext("... accepting: ");
        ex.errStream << *ptr;
      }
      throw;
    }
  }

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
Type getIndexExprType(std::vector<IndexVar> lhsIndexVars, Expr expr, 
                      bool isColumnVector);


/// CompoundOperator used with AssignStmt, TensorWrite, FieldWrite and Store.
enum class CompoundOperator { None, Add };
std::ostream &operator<<(std::ostream &os, const CompoundOperator &);


/// Represents a \ref Tensor that is defined as a constant or loaded.  Note
/// that it is only possible to define dense tensor literals.
struct Literal : public ExprNode {
  void *data;
  size_t size;

  void cast(Type type);
  double getFloatVal(int index) const;
  double_complex getComplexVal(int index) const;

  static Expr make(Type type);
  static Expr make(int val);
  static Expr make(double val);
  static Expr make(bool val);
  static Expr make(std::string val);
  static Expr make(double_complex val);
  static Expr make(Type type, void* values);
  static Expr make(Type type, std::vector<double> values);
  ~Literal();
  void accept(IRVisitorStrict *v) const {v->visit((const Literal*)this);}
};
bool operator==(const Literal& l, const Literal& r);
bool operator!=(const Literal& l, const Literal& r);


struct VarExpr : public ExprNode {
  Var var;
  static Expr make(Var var);
  void accept(IRVisitorStrict *v) const {v->visit((const VarExpr*)this);}
};

/// Expression that loads a scalar from a buffer. A buffer is a one-dimensional
/// tensor that is indexed by an integer range.
struct Load : public ExprNode {
  Expr buffer;
  Expr index;
  static Expr make(Expr buffer, Expr index);
  void accept(IRVisitorStrict *v) const {v->visit((const Load*)this);}
};

/// Expression that reads a tensor from an element or set field.
struct FieldRead : public ExprNode {
  Expr elementOrSet;
  std::string fieldName;
  static Expr make(Expr elementOrSet, std::string fieldName);
  void accept(IRVisitorStrict *v) const {v->visit((const FieldRead*)this);}
};

struct Call : public ExprNode {
  Func func;
  std::vector<Expr> actuals;
  static Expr make(Func func, std::vector<Expr> actuals);
  void accept(IRVisitorStrict *v) const {v->visit((const Call*)this);}
};

struct Length : public ExprNode {
  IndexSet indexSet;
  static Expr make(IndexSet indexSet);
  void accept(IRVisitorStrict *v) const {v->visit((const Length*)this);}
};

/// An IndexRead retrieves an index from an edge set.  An example of an index
/// is the endpoints of the edges in the set.
/// TODO DEPRECATED: This node has been deprecated with the old lowering pass
struct IndexRead : public ExprNode {
  enum Kind { Endpoints=0, NeighborsStart=1, Neighbors=2 };
  Expr edgeSet;
  Kind kind;
  static Expr make(Expr edgeSet, Kind kind);
  void accept(IRVisitorStrict *v) const {v->visit((const IndexRead*)this);}
};

struct UnaryExpr : public ExprNode {
  Expr a;
};

struct BinaryExpr : public ExprNode {
  Expr a, b;
};

struct Neg : public UnaryExpr {
  static Expr make(Expr a);
  void accept(IRVisitorStrict *v) const {v->visit((const Neg*)this);}
};

struct Add : public BinaryExpr {
  static Expr make(Expr a, Expr b);
  void accept(IRVisitorStrict *v) const {v->visit((const Add*)this);}
};

struct Sub : public BinaryExpr {
  static Expr make(Expr a, Expr b);
  void accept(IRVisitorStrict *v) const {v->visit((const Sub*)this);}
};

struct Mul : public BinaryExpr {
  static Expr make(Expr a, Expr b);
  void accept(IRVisitorStrict *v) const {v->visit((const Mul*)this);}
};

struct Div : public BinaryExpr {
  static Expr make(Expr a, Expr b);
  void accept(IRVisitorStrict *v) const {v->visit((const Div*)this);}
};

struct Not : public UnaryExpr {
  static Expr make(Expr a);
  void accept(IRVisitorStrict *v) const {v->visit((const Not*)this);}
};

struct Eq : public BinaryExpr {
  static Expr make(Expr a, Expr b);
  void accept(IRVisitorStrict *v) const {v->visit((const Eq*)this);}
};

struct Ne : public BinaryExpr {
  static Expr make(Expr a, Expr b);
  void accept(IRVisitorStrict *v) const {v->visit((const Ne*)this);}
};

struct Gt : public BinaryExpr {
  static Expr make(Expr a, Expr b);
  void accept(IRVisitorStrict *v) const {v->visit((const Gt*)this);}
};

struct Lt : public BinaryExpr {
  static Expr make(Expr a, Expr b);
  void accept(IRVisitorStrict *v) const {v->visit((const Lt*)this);}
};

struct Ge : public BinaryExpr {
  static Expr make(Expr a, Expr b);
  void accept(IRVisitorStrict *v) const {v->visit((const Ge*)this);}
};

struct Le : public BinaryExpr {
  static Expr make(Expr a, Expr b);
  void accept(IRVisitorStrict *v) const {v->visit((const Le*)this);}
};

struct And : public BinaryExpr {
  static Expr make(Expr a, Expr b);
  void accept(IRVisitorStrict *v) const {v->visit((const And*)this);}
};

struct Or : public BinaryExpr {
  static Expr make(Expr a, Expr b);
  void accept(IRVisitorStrict *v) const {v->visit((const Or*)this);}
};

struct Xor : public BinaryExpr {
  static Expr make(Expr a, Expr b);
  void accept(IRVisitorStrict *v) const {v->visit((const Xor*)this);}
};

struct VarDecl : public StmtNode {
  Var var;
  static Stmt make(Var var);
  void accept(IRVisitorStrict *v) const {v->visit((const VarDecl*)this);}
};

struct AssignStmt : public StmtNode {
  Var var;
  Expr value;
  CompoundOperator cop;
  static Stmt make(Var var, Expr value);
  static Stmt make(Var var, Expr value, CompoundOperator cop);
  void accept(IRVisitorStrict *v) const {v->visit((const AssignStmt*)this);}
};

struct Store : public StmtNode {
  Expr buffer;
  Expr index;
  Expr value;
  CompoundOperator cop;
  static Stmt make(Expr buffer, Expr index, Expr value,
                   CompoundOperator cop=CompoundOperator::None);
  void accept(IRVisitorStrict *v) const {v->visit((const Store*)this);}
};

struct FieldWrite : public StmtNode {
  Expr elementOrSet;
  std::string fieldName;
  Expr value;
  CompoundOperator cop;
  static Stmt make(Expr elementOrSet, std::string fieldName, Expr value,
                   CompoundOperator cop=CompoundOperator::None);
  void accept(IRVisitorStrict *v) const {v->visit((const FieldWrite*)this);}
};

struct CallStmt : public StmtNode {
  std::vector<Var> results;
  Func callee;
  std::vector<Expr> actuals;
  static Stmt make(std::vector<Var> results,
                   Func callee, std::vector<Expr> actuals);
  void accept(IRVisitorStrict *v) const {v->visit((const CallStmt*)this);}
};

struct Scope : public StmtNode {
  Stmt scopedStmt;
  static Stmt make(Stmt scopedStmt);
  void accept(IRVisitorStrict *v) const {v->visit((const Scope*)this);}
};

struct IfThenElse : public StmtNode {
  Expr condition;
  Stmt thenBody, elseBody;
  static Stmt make(Expr condition, Stmt thenBody);
  static Stmt make(Expr condition, Stmt thenBody, Stmt elseBody);
  void accept(IRVisitorStrict *v) const {v->visit((const IfThenElse*)this);}
};

struct ForRange : public StmtNode {
  Var var;
  Expr start;
  Expr end;
  Stmt body;
  static Stmt make(Var var, Expr start, Expr end, Stmt body);
  void accept(IRVisitorStrict *v) const {v->visit((const ForRange*)this);}
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
struct For : public StmtNode {
  Var var;
  ForDomain domain;
  Stmt body;
  static Stmt make(Var var, ForDomain domain, Stmt body);
  void accept(IRVisitorStrict *v) const {v->visit((const For*)this);}
};

struct While : public StmtNode {
  Expr condition;
  Stmt body;
  static Stmt make(Expr condition, Stmt body);
  void accept(IRVisitorStrict *v) const {v->visit((const While*)this);}
};

struct Kernel : public StmtNode {
  Var var;
  IndexDomain domain;
  Stmt body;
  static Stmt make(Var var, IndexDomain domain, Stmt body);
  void accept(IRVisitorStrict *v) const {v->visit((const Kernel*)this);}
};

struct Block : public StmtNode {
  Stmt first, rest;
  static Stmt make(Stmt first, Stmt rest);
  static Stmt make(std::vector<Stmt> stmts);
  void accept(IRVisitorStrict *v) const {v->visit((const Block*)this);}
};

struct Print : public StmtNode {
  Expr expr;
  std::string format;
  static Stmt make(Expr expr, std::string format="");
  static Stmt make(std::string str);
  void accept(IRVisitorStrict *v) const {v->visit((const Print*)this);}
};

/// A comment, that can optionally be applied to a statement with footer and
/// header space.
struct Comment : public StmtNode {
  std::string comment;
  Stmt commentedStmt;
  bool footerSpace;
  bool headerSpace;
  static Stmt make(std::string comment, Stmt commentedStmt=Stmt(),
                   bool footerSpace=false, bool headerSpace=false);
  void accept(IRVisitorStrict *v) const {v->visit((const Comment*)this);}
};

/// Empty statement that is convenient during code development.
struct Pass : public StmtNode {
  static Stmt make();
  void accept(IRVisitorStrict *v) const {v->visit((const Pass*)this);}
};

struct TupleRead : public ExprNode {
  Expr tuple, index;
  static Expr make(Expr tuple, Expr index);
  void accept(IRVisitorStrict *v) const {v->visit((const TupleRead*)this);}
};

/// Expression that reads a tensor from an n-dimensional tensor location.
struct TensorRead : public ExprNode {
  Expr tensor;
  std::vector<Expr> indices;

  /// Construct a tensor read that reads a block from the location in `tensor`
  /// specified by `indices`. The caller must either provide one or n indices,
  /// where n is the tensor order. If one index is provided then the tensor read
  /// has already been flattened, and will be directly lowered to a load.
  static Expr make(Expr tensor, std::vector<Expr> indices);
  void accept(IRVisitorStrict *v) const {v->visit((const TensorRead*)this);}
};

struct TensorWrite : public StmtNode {
  // TODO: Consider whether to make tensor a Var
  Expr tensor;
  std::vector<Expr> indices;
  Expr value;
  CompoundOperator cop;
  static Stmt make(Expr tensor, std::vector<Expr> indices, Expr value,
                   CompoundOperator cop=CompoundOperator::None);
  void accept(IRVisitorStrict *v) const {v->visit((const TensorWrite*)this);}
};

struct IndexedTensor : public ExprNode {
  Expr tensor;
  std::vector<IndexVar> indexVars;

  static Expr make(Expr tensor, std::vector<IndexVar> indexVars);
  void accept(IRVisitorStrict *v) const {v->visit((const IndexedTensor*)this);}
};

struct IndexExpr : public ExprNode {
  std::vector<IndexVar> resultVars;
  Expr value;
  std::vector<IndexVar> domain() const;

  static Expr make(std::vector<IndexVar> resultVars, Expr value, 
                   bool isColumnVector = false);
  void accept(IRVisitorStrict *v) const {v->visit((const IndexExpr*)this);}
};

struct Map : public StmtNode {
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
  void accept(IRVisitorStrict *v) const {v->visit((const Map*)this);}
};

}} // namespace simit::ir

#endif
