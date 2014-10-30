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

  bool defined() {return type.defined();}

  Var() {}
  Var(std::string name, Type type) : name(name), type(type) {}
};

inline std::ostream &operator<<(std::ostream &os, const Var &v) {
  return os << v.name;
}
inline bool operator==(const Var &l, const Var &r) {return l.name == r.name;}
inline bool operator!=(const Var &l, const Var &r) {return l.name != r.name;}
inline bool operator <(const Var &l, const Var &r) {return l.name < r.name;}
inline bool operator >(const Var &l, const Var &r) {return l.name > r.name;}


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

  Expr(int val);
  Expr(double val);

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


/// A Simit function
namespace {
// Content struct to make it cheap to copy the function to pass it around.
struct FuncContent {
  int kind;

  std::string name;
  std::vector<Var> arguments;
  std::vector<Var> results;
  Stmt body;

  std::vector<Var> temporaries;

  mutable long ref = 0;
  friend inline void aquire(FuncContent *c) {++c->ref;}
  friend inline void release(FuncContent *c) {if (--c->ref==0) delete c;}
};
}

class Func : public util::IntrusivePtr<FuncContent> {
public:
  enum Kind { Internal=0, Intrinsic=1 };

  Func() : IntrusivePtr() {}

  Func(const std::string &name, const std::vector<Var> &arguments,
       const std::vector<Var> &results, Stmt body, Kind kind,
       const std::vector<Var> &temporaries) : IntrusivePtr(new FuncContent) {
    ptr->kind = kind;
    ptr->name = name;
    ptr->arguments = arguments;
    ptr->results = results;
    ptr->body = body;
    ptr->temporaries = temporaries;
  }

  Func(const std::string &name, const std::vector<Var> &arguments,
       const std::vector<Var> &results, Stmt body, Kind kind=Internal)
      : Func(name, arguments, results, body, kind, std::vector<Var>()) {}

  Func(const std::string &name, const std::vector<Var> &arguments,
       const std::vector<Var> &results, Kind kind)
      : Func(name, arguments, results, Stmt(), kind) {
    assert(kind != Internal);
  }

  Func::Kind getKind() const {return static_cast<Kind>(ptr->kind);}
  std::string getName() const {return ptr->name;}
  const std::vector<Var> &getArguments() const {return ptr->arguments;}
  const std::vector<Var> &getResults() const {return ptr->results;}
  Stmt getBody() const {return ptr->body;}
  const std::vector<Var> &getTemporaries() const {return ptr->temporaries;}

  void accept(IRVisitor *visitor) const { visitor->visit(this); };
};


// Intrinsics:
class Intrinsics {
public:
  static Func sin;
  static Func cos;
  static Func atan2;
  static Func sqrt;
  static Func log;
  static Func exp;
  static Func solve;
  static std::map<std::string, Func> byName;
};


// Type compute functions
Type getFieldType(Expr elementOrSet, std::string fieldName);
Type getBlockType(Expr tensor);
Type getIndexExprType(std::vector<IndexVar> lhsIndexVars, Expr expr);


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
    assert(type.isTensor() && "only tensor literals are supported for now");

    size_t size = 0;
    switch (type.kind()) {
      case Type::Tensor: {
          const TensorType *ttype = type.toTensor();
          size = ttype->size() * ttype->componentType.bytes();
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
    assert(isScalarTensor(type) || type.toTensor()->size() == values.size());
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
    node->type = getFieldType(elementOrSet, fieldName);
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
    for (auto &index : indices) {
      assert(isScalarTensor(index.type()));
    }

    TensorRead *node = new TensorRead;
    node->type = getBlockType(tensor);
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

/// An IndexRead retrieves an index from an edge set.  An example of an index
/// is the endpoints of the edges in the set.
struct IndexRead : public ExprNode<IndexRead> {
  Expr edgeSet;
  std::string indexName;

  static Expr make(Expr edgeSet, std::string indexName) {
    assert(edgeSet.type().isSet());
    IndexRead *node = new IndexRead;
    node->type = TensorType::make(ScalarType(ScalarType::Int),
                                  {IndexDomain(IndexSet(edgeSet))});
    node->edgeSet = edgeSet;
    node->indexName = indexName;
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
    node->type = TensorType::make(tensor.type().toTensor()->componentType);
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
    assert(isScalarTensor(value.type()));
    for (auto &idxVar : resultVars) {  // No reduction variables on lhs
      assert(idxVar.isFreeVar());
    }
    IndexExpr *node = new IndexExpr;
    node->type = getIndexExprType(resultVars, value);
    node->resultVars = resultVars;
    node->value = value;
    return node;
  }
};

struct Call : public ExprNode<Call> {
  Func function;
  std::vector<Expr> actuals;

  static Expr make(Func function, std::vector<Expr> actuals) {
    assert(function.getResults().size() == 1 &&
           "only calls of function with one results is currently supported.");

    Call *node = new Call;
    node->type = function.getResults()[0].type;
    node->function = function;
    node->actuals = actuals;
    return node;
  }
};

struct Neg : public ExprNode<Neg> {
  Expr a;

  static Expr make(Expr a) {
    assert(isScalarTensor(a.type()));

    Neg *node = new Neg;
    node->type = a.type();
    node->a = a;
    return node;
  }
};

struct Add : public ExprNode<Add> {
  Expr a, b;

  static Expr make(Expr a, Expr b) {
    assert(isScalarTensor(a.type()));
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
    assert(isScalarTensor(a.type()));
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
    assert(isScalarTensor(a.type()));
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
    assert(isScalarTensor(a.type()));
    assert(a.type() == b.type());

    Div *node = new Div;
    node->type = a.type();
    node->a = a;
    node->b = b;
    return node;
  }
};

struct Load : public ExprNode<Load> {
  Expr buffer;
  Expr index;

  static Expr make(Expr buffer, Expr index) {
    assert(isScalarTensor(index.type()));

    // TODO: Create a buffer/array type and assert that buffer is that type.
    //       Then get the component from the buffer.
    ScalarType ctype = buffer.type().toTensor()->componentType;

    Load  *node = new Load;
    node->type = TensorType::make(ctype);
    node->buffer = buffer;
    node->index = index;
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

struct Map : public StmtNode<Map> {
  std::vector<Var> vars;
  Func function;
  Expr target, neighbors;
  ReductionOperator reduction;

  static Stmt make(std::vector<Var> vars, Func function,
                   Expr target, Expr neighbors=Expr(),
                   ReductionOperator reduction=ReductionOperator()) {
    assert(target.type().isSet());
    assert(!neighbors.defined() || neighbors.type().isSet());
    assert(vars.size() == function.getResults().size());
    Map *node = new Map;
    node->vars = vars;
    node->function = function;
    node->target = target;
    node->neighbors = neighbors;
    node->reduction = reduction;
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

struct Store : public StmtNode<Store> {
  Expr buffer;
  Expr index;
  Expr value;

  static Stmt make(Expr buffer, Expr index, Expr value) {
    Store *node = new Store;
    node->buffer = buffer;
    node->index = index;
    node->value = value;
    return node;
  }
};

struct ForDomain {
  enum Kind { IndexSet, Endpoints, Edges };
  Kind kind;

  /// An index set
  class IndexSet indexSet;

  /// A lookup in the index structures of an edge set
  Expr set;
  Var var;

  ForDomain() {}
  ForDomain(class IndexSet indexSet) : kind(IndexSet), indexSet(indexSet) {}
  ForDomain(Expr set, Var var, Kind kind) : kind(kind), set(set), var(var) {
    assert(kind==Edges || kind==Endpoints);
  }
};

struct For : public StmtNode<For> {
  Var var;
  ForDomain domain;
  Stmt body;

  static Stmt make(Var var, ForDomain domain, Stmt body) {
    For *node = new For;
    node->var = var;
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
