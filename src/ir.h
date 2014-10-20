#ifndef SIMIT_IR_H
#define SIMIT_IR_H

#include <cassert>
#include <string>
#include <list>
#include <cstring>

#include <iostream>
#include "ir_printer.h"

#include "intrusive_ptr.h"
#include "interfaces.h"
#include "types.h"
#include "indexvar.h"

namespace simit {
namespace ir {

struct Var {
  std::string name;
  Type type;

  Var() {}
  Var(std::string name, Type type) : name(name), type(type) {}
};

inline std::ostream &operator<<(std::ostream &os, const Var &v) {
  return os << v.name;
}

inline bool operator==(const Var &v1, const Var &v2) {
  return v1.name == v2.name;
}

inline bool operator!=(const Var &v1, const Var &v2) {
  return v1.name != v2.name;
}

inline bool operator<(const Var &v1, const Var &v2) {
  return v1.name < v2.name;
}

inline bool operator>(const Var &v1, const Var &v2) {
  return v1.name > v2.name;
}


/// The base class of all nodes in the Simit Intermediate Representation
/// (Simit IR)
struct IRNode : private simit::interfaces::Uncopyable {
public:
  IRNode() : ref(0) {}
  virtual ~IRNode() {}
  virtual void accept(IRVisitor *visitor) const = 0;

private:
  mutable long ref;
  friend inline void aquire(const IRNode *node) {
    ++node->ref;
  }
  
  friend inline void release(const IRNode *node) {
    if (--node->ref == 0) {
      delete node;
    }
  }
};

struct ExprNodeBase : public IRNode {
  Type type;
};

struct StmtNodeBase : public IRNode {
};

template <typename T>
struct ExprNode : public ExprNodeBase {
  void accept(IRVisitor *v) const { v->visit((const T *)this); }
};

template <typename T>
struct StmtNode : public StmtNodeBase {
  void accept(IRVisitor *v) const { v->visit((const T *)this); }
};

class Expr : public util::IntrusivePtr<const ExprNodeBase> {
public:
  Expr() : IntrusivePtr() {}
  Expr(const ExprNodeBase *expr) : IntrusivePtr(expr) {}
  Expr(const Var &var);

  Type type() const {return ptr->type;}

  const ExprNodeBase *expr() const {return ptr;}

  void accept(IRVisitor *v) const {ptr->accept(v);}
};

template <typename E>
inline bool isa(Expr e) {
  return e.defined() && dynamic_cast<const E*>(e.expr()) != nullptr;
}

template <typename E>
inline const E* to(Expr e) {
  assert(isa<E>(e) && "Wrong Expr type");
  return static_cast<const E*>(e.expr());
}

class Stmt : public util::IntrusivePtr<const StmtNodeBase> {
public:
  Stmt() : IntrusivePtr() {}
  Stmt(const StmtNodeBase *stmt) : IntrusivePtr(stmt) {}

  const StmtNodeBase *stmt() const {return ptr;}

  void accept(IRVisitor *v) const {ptr->accept(v);}
};

template <typename S>
inline bool isa(Stmt s) {
  return s.defined() && dynamic_cast<const S*>(s.stmt()) != nullptr;
}

template <typename S>
inline const S* to(Stmt s) {
  assert(isa<S>(s) && "Wrong Expr type");
  return static_cast<const S*>(s.stmt());
}


// Type compute functions
Type fieldType(Expr elementOrSet, std::string fieldName);
Type blockType(Expr tensor);
Type indexExprType(std::vector<IndexVar> lhsIndexVars, Expr expr);


/// Represents a \ref Tensor that is defined as a constant or loaded.  Note
/// that it is only possible to define dense tensor literals.
struct Literal : public ExprNode<Literal> {
  void *data;
  size_t size;

  void cast(Type type);

  static Expr make(Type type) {
    return Literal::make(type, nullptr);
  }

  static Expr make(Type type, void *values) {
    size_t size = 0;
    switch (type.kind()) {
      case Type::Scalar:
        size = type.toScalar()->bytes();
        break;
      case Type::Tensor: {
          const TensorType *ttype = type.toTensor();
          size = ttype->size() * ttype->componentType.toScalar()->bytes();
        break;
      }
      case Type::Element:
      case Type::Set:
      case Type::Tuple:
        assert(false && "Only tensor and scalar literals currently supported");
        break;
    }

    Literal *node = new Literal;
    node->type = type;
    node->size = size;
    node->data = malloc(node->size);
    if (values != nullptr) {
      memcpy(node->data, values, node->size);
    }
    else {
      memset(node->data, 0, node->size);
    } 
    return node;
  }

  static Expr make(Type type, std::vector<double> values) {
    assert(type.isScalar() || type.toTensor()->size() == values.size());
    return Literal::make(type, values.data());
  }

  ~Literal() {free(data);}
};

struct VarExpr : public ExprNode<VarExpr> {
  Var var;

  static Expr make(Var var) {
    VarExpr *node = new VarExpr;
    node->type = var.type;
    node->var = var;
    return node;
  }
};

struct Result : public ExprNode<Result> {
  Stmt producer;
  unsigned location;

  static Expr make(Stmt producer, unsigned location) {
    Result *node = new Result;
//    node->type = TODO
    node->producer = producer;
    node->location = location;
    return node;
  }
};

/// Expression that reads a tensor from an element or set field.
struct FieldRead : public ExprNode<FieldRead> {
  Expr elementOrSet;
  std::string fieldName;

  static Expr make(Expr elementOrSet, std::string fieldName) {
    assert(elementOrSet.type().isElement() || elementOrSet.type().isSet());
    FieldRead *node = new FieldRead;
    node->type = fieldType(elementOrSet, fieldName);
    node->elementOrSet = elementOrSet;
    node->fieldName = fieldName;
    return node;
  }
};

/// Expression that reads a tensor from a tensor location.
struct TensorRead : public ExprNode<TensorRead> {
  Expr tensor;
  std::vector<Expr> indices;

  static Expr make(Expr tensor, std::vector<Expr> indices) {
    assert(tensor.type().isTensor());
    TensorRead *node = new TensorRead;
    node->type = blockType(tensor);
    node->tensor = tensor;
    node->indices = indices;
    return node;
  }
};

struct TupleRead : public ExprNode<TupleRead> {
  Expr tuple, index;

  static Expr make(Expr tuple, Expr index) {
    assert(tuple.type().isTuple());
    TupleRead *node = new TupleRead;
    node->type = tuple.type().toTuple()->elementType;
    node->tuple = tuple;
    node->index = index;
    return node;
  }
};

struct Map : public ExprNode<Map> {
  std::string function;
  Expr target, neighbors;
  ReductionOperator reductionOp;

  static Expr make(std::string function, Expr target, Expr neighbors,
                   ReductionOperator reductionOp) {
    assert(target.type().isSet() && neighbors.type().isSet());
    Map *node = new Map;
    node->function = function;
    node->target = target;
    node->neighbors = neighbors;
    node->reductionOp = reductionOp;
    return node;
  }
};

struct IndexedTensor : public ExprNode<IndexedTensor> {
  Expr tensor;
  std::vector<IndexVar> indexVars;

  static Expr make(Expr tensor, std::vector<IndexVar> indexVars) {
    assert(tensor.type().isTensor() && "Only tensors can be indexed.");
    assert(indexVars.size() == tensor.type().toTensor()->order());
    for (size_t i=0; i < indexVars.size(); ++i) {
      assert(indexVars[i].getDomain() == tensor.type().toTensor()->dimensions[i]
             && "IndexVar domain does not match tensordimension");
    }

    IndexedTensor *node = new IndexedTensor;
    node->type = tensor.type().toTensor()->componentType;
    node->tensor = tensor;
    node->indexVars = indexVars;
    return node;
  }
};

struct IndexExpr : public ExprNode<IndexExpr> {
  std::vector<IndexVar> resultVars;
  Expr value;

  std::vector<IndexVar> domain() const;

  static Expr make(std::vector<IndexVar> resultVars, Expr value) {
    assert(value.type().isScalar());
    for (auto &idxVar : resultVars) {  // No reduction variables on lhs
      assert(idxVar.isFreeVar());
    }
    IndexExpr *node = new IndexExpr;
    node->type = indexExprType(resultVars, value);
    node->resultVars = resultVars;
    node->value = value;
    return node;
  }
};

struct Call : public ExprNode<Call> {
  enum Kind { Internal, Intrinsic };

  std::string function;
  std::vector<Expr> actuals;
  Kind kind;

  static Expr make(std::string function, std::vector<Expr> actuals, Kind kind) {
    Call *node = new Call;
//    node->type = TODO
    node->function = function;
    node->actuals = actuals;
    node->kind = kind;
    return node;
  }
};

struct Neg : public ExprNode<Neg> {
  Expr a;

  static Expr make(Expr a) {
    assert(a.type().isScalar());

    Neg *node = new Neg;
    node->type = a.type();
    node->a = a;
    return node;
  }
};

struct Add : public ExprNode<Add> {
  Expr a, b;

  static Expr make(Expr a, Expr b) {
    assert(a.type().isScalar());
    assert(a.type() == b.type());

    Add *node = new Add;
    node->type = a.type();
    node->a = a;
    node->b = b;
    return node;
  }
};

struct Sub : public ExprNode<Sub> {
  Expr a, b;

  static Expr make(Expr a, Expr b) {
    assert(a.type().isScalar());
    assert(a.type() == b.type());

    Sub *node = new Sub;
    node->type = a.type();
    node->a = a;
    node->b = b;
    return node;
  }
};

struct Mul : public ExprNode<Mul> {
  Expr a, b;

  static Expr make(Expr a, Expr b) {
    assert(a.type().isScalar());
    assert(a.type() == b.type());

    Mul *node = new Mul;
    node->type = a.type();
    node->a = a;
    node->b = b;
    return node;
  }
};

struct Div : public ExprNode<Div> {
  Expr a, b;

  static Expr make(Expr a, Expr b) {
    assert(a.type().isScalar());
    assert(a.type() == b.type());

    Div *node = new Div;
    node->type = a.type();
    node->a = a;
    node->b = b;
    return node;
  }
};


// Statements
struct AssignStmt : public StmtNode<AssignStmt> {
  Var var;
  Expr value;

  static Stmt make(Var var, Expr value) {
    AssignStmt *node = new AssignStmt;
    node->var = var;
    node->value = value;
    return node;
  }
};

struct FieldWrite : public StmtNode<FieldWrite> {
  Expr elementOrSet;
  std::string fieldName;
  Expr value;

  static Stmt make(Expr elementOrSet, std::string fieldName, Expr value) {
    FieldWrite *node = new FieldWrite;
    node->elementOrSet = elementOrSet;
    node->fieldName = fieldName;
    node->value = value;
    return node;
  }
};

struct TensorWrite : public StmtNode<TensorWrite> {
  Expr tensor;
  std::vector<Expr> indices;
  Expr value;

  static Stmt make(Expr tensor, std::vector<Expr> indices, Expr value) {
    TensorWrite *node = new TensorWrite;
    node->tensor = tensor;
    node->indices = indices;
    node->value = value;
    return node;
  }
};

struct E2V {

};

struct V2E {

};

class LoopDomain {
public:
  enum Kind { MinMax, Domain, V2E, E2V };

  Kind getKind() {return kind;}

  std::pair<Expr,Expr> getMinMax() {return minMax;}
  IndexSet getDomain() {return domain;}
  struct E2V getE2V() {return e2v;}
  struct V2E getV2E() {return v2e;}

private:
  Kind kind;
  union {
    std::pair<Expr,Expr> minMax;
    IndexSet domain;
    struct E2V e2v;
    struct V2E v2e;
  };
};

struct For : public StmtNode<For> {
  std::string name;
  IndexSet domain;

  Stmt body;

  static Stmt make(std::string name, IndexSet domain, Stmt body) {
    For *node = new For;
    node->name = name;
    node->domain = domain;
    node->body = body;
    return node;
  }
};

struct IfThenElse : public StmtNode<IfThenElse> {
  Expr condition;
  Stmt thenBody, elseBody;

  static Stmt make(Expr condition, Stmt thenBody, Stmt elseBody) {
    IfThenElse *node = new IfThenElse;
    node->condition = condition;
    node->thenBody = thenBody;
    node->elseBody = elseBody;
    return node;
  }
};

struct Block : public StmtNode<Block> {
  Stmt first, rest;

  static Stmt make(Stmt first, Stmt rest) {
    assert(first.defined() && "Empty block");
    Block *node = new Block;
    node->first = first;
    node->rest = rest;
    return node;
  }

  static Stmt make(std::vector<Stmt> stmts) {
    assert(stmts.size() > 0 && "Empty block");
    Stmt node;
    for (size_t i=stmts.size(); i>0; --i) {
      node = Block::make(stmts[i-1], node);
    }
    return node;
  }
};

/// Empty statement that is convenient during code development.
struct Pass : public StmtNode<Pass> {
  static Stmt make() {
    Pass *node = new Pass;
    return node;
  }
};

/// A Simit function
namespace {
// Content struct to make it cheap to copy the function to pass it around.
struct FuncContent {
  std::string name;
  std::vector<Var> arguments;
  std::vector<Var> results;
  Stmt body;

  mutable long ref = 0;
  friend inline void aquire(FuncContent *c) {++c->ref;}
  friend inline void release(FuncContent *c) {if (--c->ref==0) delete c;}
};
}

class Func : public util::IntrusivePtr<FuncContent> {
public:
  Func() : IntrusivePtr() {}

  Func(const std::string &name, const std::vector<Var> &arguments,
       const std::vector<Var> &results, Stmt body)
      : IntrusivePtr(new FuncContent) {
    ptr->name = name;
    ptr->arguments = arguments;
    ptr->results = results;
    ptr->body = body;
  }

  std::string getName() const {return ptr->name;}
  const std::vector<Var> &getArguments() const {return ptr->arguments;}
  const std::vector<Var> &getResults() const {return ptr->results;}
  Stmt getBody() const {return ptr->body;}

  void accept(IRVisitor *visitor) const { visitor->visit(this); };
};


/// A Simit test case. Simit test cases can be declared in language comments
/// and can subsequently be picked up by a test framework.
class Test {
public:
  Test(const std::string &callee,
       const std::vector<Expr> &actuals,
       const std::vector<Expr> &expected)
      : callee(callee), actuals(actuals), expected(expected) {}

  std::string getCallee() const { return callee; }

  std::vector<Expr> getActuals() const {
    return actuals;
  }

  const std::vector<Expr> &getExpectedResults() const {
    return expected;
  }

private:
  std::string callee;
  std::vector<Expr> actuals;
  std::vector<Expr> expected;
};

bool operator==(const Expr &, const Expr &);
bool operator!=(const Expr &, const Expr &);

bool operator==(const Literal& l, const Literal& r);
bool operator!=(const Literal& l, const Literal& r);

}} // namespace simit::ir

#endif
