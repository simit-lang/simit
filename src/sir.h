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
  Expr(ExprNode *expr) : expr(expr) {}
  std::shared_ptr<ExprNode> expr;
  void accept(SetIRVisitor *v) { expr->accept(v); }
  void accept(SetIRConstVisitor *v) const { expr->accept(v); }
};

struct Stmt {
  Stmt(StmtNode *stmt) : stmt(stmt) {}
  std::shared_ptr<StmtNode> stmt;
  void accept(SetIRVisitor *v) { stmt->accept(v); }
  void accept(SetIRConstVisitor *v) const { stmt->accept(v); }
};

struct Variable : public ExprNode {
  std::string name;

  Variable(const std::string &name) : name(name) {}
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Load : public ExprNode {
  std::string name;
  Expr index;

  Load(const std::string &name, const Expr &index) : name(name), index(index) {}
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Neg : public ExprNode {
  Expr a;

  Neg(Expr a) : a(a) {}
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Add : public ExprNode {
  Expr a, b;

  Add(const Expr &a, const Expr &b) : a(a), b(b) {}
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Sub : public ExprNode {
  Expr a, b;

  Sub(const Expr &a, const Expr &b) : a(a), b(b) {}
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Mul : public ExprNode {
  Expr a, b;

  Mul(const Expr &a, const Expr &b) : a(a), b(b) {}
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Div : public ExprNode {
  Expr a, b;

  Div(const Expr &a, const Expr &b) : a(a), b(b) {}
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Block : public StmtNode {
  std::vector<Stmt> stmts;

  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

struct Foreach : public StmtNode {
  std::string name;
  IndexSet domain;
  Stmt body;

  Foreach(const std::string &name, const IndexSet &domain, const Stmt &body)
      : name(name), domain(domain), body(body) {}
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};

/// Stores the value to the buffer with the given name at index.
struct Store : public StmtNode {
  std::string name;
  Expr index, value;

  Store(const std::string &name, Expr &index, const Expr &value)
      : name(name), index(index), value(value) {}
  void accept(SetIRVisitor *v) { v->visit(this); };
  void accept(SetIRConstVisitor *v) const { v->visit(this); }
};


}} // namespace simit::ir

#endif
