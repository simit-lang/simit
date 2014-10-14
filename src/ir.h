#ifndef SIMIT_IR_H
#define SIMIT_IR_H

#include <cassert>
#include <string>
#include <list>

#include <iostream>
#include "ir_printer.h"

#include "intrusive_ptr.h"
#include "interfaces.h"
#include "types.h"
#include "indexvar.h"

namespace simit {
namespace ir {

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

  Type type() const {return ptr->type;}

  const ExprNodeBase *expr() const {return ptr;}

  void accept(IRVisitor *v) const {ptr->accept(v);}
};

class Stmt : public util::IntrusivePtr<const StmtNodeBase> {
public:
  Stmt() : IntrusivePtr() {}
  Stmt(const StmtNodeBase *stmt) : IntrusivePtr(stmt) {}

  const StmtNodeBase *stmt() const {return ptr;}

  void accept(IRVisitor *v) const {ptr->accept(v);}
};


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

struct Variable : public ExprNode<Variable> {
  std::string name;

  static Expr make(std::string name, Type type) {
    Variable *node = new Variable;
    node->type = type;
    node->name = name;
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
    assert(tuple.type().isTensor());
    TupleRead *node = new TupleRead;
    // TODO: Compute type
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
  std::vector<IndexVar> lhsIndexVars;
  Expr rhs;

  std::vector<IndexVar> domain();

  static Expr make(std::vector<IndexVar> lhsIndexVars, Expr rhs) {
    assert(rhs.type().isScalar());
    for (auto &idxVar : lhsIndexVars) {  // No reduction variables on lhs
      assert(idxVar.isFreeVar());
    }

    IndexExpr *node = new IndexExpr;
    node->type = indexExprType(lhsIndexVars, rhs);
    node->lhsIndexVars = lhsIndexVars;
    node->rhs = rhs;
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
  std::vector<std::string> lhs;
  Expr rhs;

  static Stmt make(std::vector<std::string> lhs, Expr rhs) {
    AssignStmt *node = new AssignStmt;
    node->lhs = lhs;
    node->rhs = rhs;
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

struct For : public StmtNode<For> {
  std::string name;
  IndexDomain domain;
  Stmt body;

  static Stmt make(std::string name, IndexDomain domain, Stmt body) {
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
  std::vector<Expr> arguments;
  std::vector<Expr> results;
  Stmt body;

  mutable long ref = 0;
  friend inline void aquire(FuncContent *c) {++c->ref;}
  friend inline void release(FuncContent *c) {if (--c->ref == 0) delete c;}
};
}

class Func : public util::IntrusivePtr<FuncContent> {
public:
  Func() : IntrusivePtr() {}

  Func(const std::string &name, const std::vector<Expr> &arguments,
       const std::vector<Expr> &results, Stmt body)
      : IntrusivePtr(new FuncContent) {
    ptr->name = name;
    ptr->arguments = arguments;
    ptr->results = results;
    ptr->body = body;
  }

  void setBody(Stmt body) {ptr->body = body;}

  std::string getName() const {return ptr->name;}
  const std::vector<Expr> &getArguments() const {return ptr->arguments;}
  const std::vector<Expr> &getResults() const {return ptr->results;}
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

//
inline const Literal *toLiteral(Expr e) {
  return static_cast<const Literal*>(e.expr());
}

inline const Variable *toVariable(Expr e) {
  return static_cast<const Variable*>(e.expr());
}

}} // namespace simit::internal

#endif
