#ifndef SIMIT_SIR_H
#define SIMIT_SIR_H

#include <cassert>
#include <string>
#include <list>

#include "interfaces.h"
#include "types.h"
#include "sir_visitor.h"

namespace simit {
namespace ir {

// \todo
// Domain one of:
// - A full set
// - Edge endpoints
// - Element edges

struct SetIRNode : public interfaces::Uncopyable {
  virtual void accept(SetIRVisitor *visitor) = 0;
  virtual void accept(SetIRConstVisitor *visitor) const = 0;
};

struct ExprNode : public SetIRNode {
};

struct StmtNode : public SetIRNode {
};

struct Expr {
  Expr() : expr(nullptr) {}
  Expr(ExprNode *expr) : expr(expr) {}
  std::shared_ptr<ExprNode> expr;
  bool defined() const { return expr != nullptr; }
  void accept(SetIRVisitor *v) { expr->accept(v); }
  void accept(SetIRConstVisitor *v) const { expr->accept(v); }
};

struct Stmt {
  Stmt() : stmt(nullptr) {}
  Stmt(StmtNode *stmt) : stmt(stmt) {}
  std::shared_ptr<StmtNode> stmt;
  bool defined() const { return stmt != nullptr; }
  void accept(SetIRVisitor *v) { stmt->accept(v); }
  void accept(SetIRConstVisitor *v) const { stmt->accept(v); }
};

struct IntLiteral : public ExprNode {
  int val;

  static Expr make(int val) {
    IntLiteral *node = new IntLiteral;
    node->val = val;
    return node;
  }
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Variable : public ExprNode {
  std::string name;

  static Expr make(const std::string &name) {
    Variable *node = new Variable;
    node->name = name;
    return node;
  }
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Load : public ExprNode {
  Expr target, index;

  static Expr make(Expr target, Expr index) {
    Load *node = new Load;
    node->target = target;
    node->index = index;
    return node;
  }
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Neg : public ExprNode {
  Expr a;

  static Expr make(Expr a) {
    Neg *node = new Neg;
    node->a = a;
    return node;
  }
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Add : public ExprNode {
  Expr a, b;

  static Expr make(Expr a, Expr b) {
    Add *node = new Add;
    node->a = a;
    node->b = b;
    return node;
  }
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Sub : public ExprNode {
  Expr a, b;

  static Expr make(Expr a, Expr b) {
    Sub *node = new Sub;
    node->a = a;
    node->b = b;
    return node;
  }
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Mul : public ExprNode {
  Expr a, b;

  static Expr make(Expr a, Expr b) {
    Mul *node = new Mul;
    node->a = a;
    node->b = b;
    return node;
  }
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Div : public ExprNode {
  Expr a, b;

  static Expr make(Expr a, Expr b) {
    Div *node = new Div;
    node->a = a;
    node->b = b;
    return node;
  }
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Block : public StmtNode {
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
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Foreach : public StmtNode {
  std::string name;
  IndexSet domain;
  Stmt body;

  static Stmt make(const std::string &name, const IndexSet &domain,
                   const Stmt &body) {
    Foreach *node = new Foreach;
    node->name = name;
    node->domain = domain;
    node->body = body;
    return node;
  }
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

/// Stores a value to the buffer with the given name at index.
struct Store : public StmtNode {
  Expr target, index, value;

  static Stmt make(Expr target, Expr index, Expr value) {
    Store *node = new Store;
    node->target = target;
    node->index = index;
    node->value = value;
    return node;
  }
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct StoreMatrix : public StmtNode {
  Expr target, row, col, value;

  static Stmt make(Expr target, Expr row, Expr col, Expr value) {
    StoreMatrix *node = new StoreMatrix;
    node->target = target;
    node->row = row;
    node->col = col;
    node->value = value;
    return node;
  }
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

/// Empty statement that is convenient during code development.
struct Pass : public StmtNode {
  static Stmt make() {
    Pass *node = new Pass;
    return node;
  }
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};


}} // namespace simit::ir

#endif
