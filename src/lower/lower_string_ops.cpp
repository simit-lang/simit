#include "lower_string_ops.h"

#include "intrinsics.h"
#include "ir_rewriter.h"
#include "ir_transforms.h"
#include "types.h"

#include <vector>

namespace simit {
namespace ir {

class LowerStringOps : public IRRewriter {
public:
  using IRRewriter::rewrite;

  Stmt rewrite(Stmt s) {
    if (s.defined()) {
      s.accept(this);
      stmts.push_back(stmt);
      s = (stmts.size() > 0) ? Block::make(stmts) : stmt;
      stmts.clear();
    }
    else {
      s = Stmt();
    }
    expr = Expr();
    stmt = Stmt();
    return s;
  }

private:
  std::vector<Stmt> stmts;
  
  using IRRewriter::visit;

  enum class CompareOp {EQ, NE, LE, LT, GE, GT};

  void lowerStringCompare(CompareOp op, Expr a, Expr b) {
    iassert(isString(a.type()));
    iassert(isString(b.type()));

    a = rewrite(a);
    b = rewrite(b);
    
    Var tmp("tmp", Int);
    Stmt cmpStmt = CallStmt::make({tmp}, intrinsics::strcmp(), {a, b});
    stmts.push_back(cmpStmt);

    Expr tmpExpr = VarExpr::make(tmp);
    Expr zero = Literal::make(0);

    switch (op) {
      case CompareOp::EQ:
        expr = Eq::make(tmpExpr, zero);
        break;
      case CompareOp::NE:
        expr = Ne::make(tmpExpr, zero);
        break;
      case CompareOp::LE:
        expr = Le::make(tmpExpr, zero);
        break;
      case CompareOp::LT:
        expr = Lt::make(tmpExpr, zero);
        break;
      case CompareOp::GE:
        expr = Ge::make(tmpExpr, zero);
        break;
      case CompareOp::GT:
        expr = Gt::make(tmpExpr, zero);
        break;
      default:
        unreachable;
    }
  }

  void visit(const Eq* op) {
    if (isString(op->a.type())) {
      lowerStringCompare(CompareOp::EQ, op->a, op->b);
    } else {
      IRRewriter::visit(op);
    }
  }

  void visit(const Ne* op) {
    if (isString(op->a.type())) {
      lowerStringCompare(CompareOp::NE, op->a, op->b);
    } else {
      IRRewriter::visit(op);
    }
  }

  void visit(const Le* op) {
    if (isString(op->a.type())) {
      lowerStringCompare(CompareOp::LE, op->a, op->b);
    } else {
      IRRewriter::visit(op);
    }
  }

  void visit(const Lt* op) {
    if (isString(op->a.type())) {
      lowerStringCompare(CompareOp::LT, op->a, op->b);
    } else {
      IRRewriter::visit(op);
    }
  }

  void visit(const Ge* op) {
    if (isString(op->a.type())) {
      lowerStringCompare(CompareOp::GE, op->a, op->b);
    } else {
      IRRewriter::visit(op);
    }
  }

  void visit(const Gt* op) {
    if (isString(op->a.type())) {
    lowerStringCompare(CompareOp::GT, op->a, op->b);
    } else {
      IRRewriter::visit(op);
    }
  }
};

Func lowerStringOps(Func func) {
  func = LowerStringOps().rewrite(func);
  func = insertVarDecls(func);
  return func;
}

}}

