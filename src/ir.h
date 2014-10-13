#ifndef SIMIT_IR_H
#define SIMIT_IR_H

#include <cassert>
#include <string>
#include <list>

#include <iostream>
#include "ir_printer.h"

#include "interfaces.h"
#include "types.h"
#include "indexvar.h"

namespace simit {
namespace ir {

/// The base class of all nodes in the Simit Intermediate Representation
/// (Simit IR)
struct IRNode : private simit::interfaces::Uncopyable {
  virtual void accept(IRVisitor *visitor) = 0;
  virtual void accept(IRConstVisitor *visitor) const = 0;
};

struct ExprNodeBase : public IRNode {
public:
  Type type;
};

struct StmtNodeBase : public IRNode {
};

template <typename T>
struct ExprNode : public ExprNodeBase {
  void accept(IRVisitor *v) { v->visit((T *)this); }
  void accept(IRConstVisitor *v) const { v->visit((const T *)this); }
};

template <typename T>
struct StmtNode : public StmtNodeBase {
  void accept(IRVisitor *v) { v->visit((T *)this); }
  void accept(IRConstVisitor *v) const { v->visit((const T *)this); }
};

class Expr {
public:
  explicit Expr() : exprPtr(nullptr) {}
  explicit Expr(ExprNodeBase *expr) : exprPtr(expr) {assert(expr != nullptr);}

  bool defined() const {return exprPtr != nullptr;}
  Type type() const {return exprPtr->type;}

  ExprNodeBase *expr() {return exprPtr.get();}
  const ExprNodeBase *expr() const {return exprPtr.get();}

  void accept(IRVisitor *v) {exprPtr->accept(v);}
  void accept(IRConstVisitor *v) const {exprPtr->accept(v);}

private:
  std::shared_ptr<ExprNodeBase> exprPtr;
};

class Stmt {
public:
  explicit Stmt() : stmtPtr(nullptr) {}
  explicit Stmt(StmtNodeBase *stmt) : stmtPtr(stmt) {assert(stmt != nullptr);}

  bool defined() const { return stmtPtr != nullptr; }

  StmtNodeBase *stmt() {return stmtPtr.get();}
  const StmtNodeBase *stmt() const {return stmtPtr.get();}

  void accept(IRVisitor *v) { stmtPtr->accept(v); }
  void accept(IRConstVisitor *v) const { stmtPtr->accept(v); }

private:
  std::shared_ptr<StmtNodeBase> stmtPtr;
};


// Type compute functions
Type fieldType(Expr setOrElement, std::string fieldName);
Type blockType(Expr tensor);
Type indexExprType(std::vector<IndexVar> lhsIndexVars, Expr expr);


/// Represents a \ref Tensor that is defined as a constant or loaded.  Note
/// that it is only possible to define dense tensor literals.
struct Literal : public ExprNode<Literal> {
  void *data;
  size_t size;

  void cast(Type type);

  static Expr make(Type type) {
    assert(type.isTensor() && "Only tensor literals currently supported");
    Literal *node = new Literal;
    node->type = type;
    const TensorType *ttype = type.toTensor();
    node->size = ttype->size() * ttype->componentType.toScalar()->bytes();
    node->data = malloc(node->size);
    return Expr(node);
  }

  static Expr make(Type type, void *values) {
    assert(type.isTensor() && "Only tensor literals currently supported");
    Literal *node = new Literal;
    node->type = type;
    const TensorType *ttype = type.toTensor();
    node->size = ttype->size() * ttype->componentType.toScalar()->bytes();
    node->data = malloc(node->size);
    memcpy(node->data, values, node->size);
    return Expr(node);
  }

  ~Literal() {free(data);}
};

struct Variable : public ExprNode<Variable> {
  std::string name;

  static Expr make(std::string name, Type type) {
    Variable *node = new Variable;
    node->type = type;
    node->name = name;
    return Expr(node);
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
    return Expr(node);
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
    return Expr(node);
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
    return Expr(node);
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
    return Expr(node);
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
    return Expr(node);
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
    return Expr(node);
  }
};

struct IndexExpr : public ExprNode<IndexExpr> {
  std::vector<IndexVar> lhsIndexVars;
  Expr expr;

  std::vector<IndexVar> domain();

  static Expr make(std::vector<IndexVar> lhsIndexVars, Expr expr) {
    assert(expr.type().isScalar());
    for (auto &idxVar : lhsIndexVars) {  // No reduction variables on lhs
      assert(idxVar.isFreeVar());
    }

    IndexExpr *node = new IndexExpr;
    node->type = indexExprType(lhsIndexVars, expr);
    node->lhsIndexVars = lhsIndexVars;
    node->expr = expr;
    return Expr(node);
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
    return Expr(node);
  }
};

struct Neg : public ExprNode<Neg> {
  Expr a;

  static Expr make(Expr a) {
    assert(a.type().isScalar());

    Neg *node = new Neg;
    node->type = a.type();
    node->a = a;
    return Expr(node);
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
    return Expr(node);
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
    return Expr(node);
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
    return Expr(node);
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
    return Expr(node);
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
    return Stmt(node);
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
    return Stmt(node);
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
    return Stmt(node);
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
    return Stmt(node);
  }
};

struct IfThenElse : public StmtNode<IfThenElse> {
  Expr condition;
  Stmt thenBody, elseBody;
};

struct Block : public StmtNode<Block> {
  Stmt first, rest;

  static Stmt make(Stmt first, Stmt rest) {
    assert(first.defined() && "Empty block");
    Block *node = new Block;
    node->first = first;
    node->rest = rest;
    return Stmt(node);
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
    return Stmt(node);
  }
};


/// A Simit function.
class Function {
public:
  Function(const std::string &name,
           const std::vector<Expr> &arguments,
           const std::vector<Expr> &results)
      : name(name), arguments(arguments), results(results) {}

  void setBody(Stmt body) {this->body = body;}

  std::string getName() const {return name;}
  const std::vector<Expr> &getArguments() const {return arguments;}
  const std::vector<Expr> &getResults() const {return results;}
  Stmt getBody() const {return body;}

  void accept(IRVisitor *visitor) { visitor->visit(this); };
  void accept(IRConstVisitor *visitor) const { visitor->visit(this); };

private:
  std::string name;
  std::vector<Expr> arguments;
  std::vector<Expr> results;
  Stmt body;
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

inline Literal *toLiteral(Expr e) { return static_cast<Literal*>(e.expr()); }
inline Variable *toVariable(Expr e) { return static_cast<Variable*>(e.expr()); }

}} // namespace simit::internal

#endif
