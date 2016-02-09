#include "lower_string_ops.h"

#include "intrinsics.h"
#include "ir_rewriter.h"
#include "ir_transforms.h"
#include "types.h"

#include <vector>
#include <stack>
#include <set>

namespace simit {
namespace ir {

class LowerStringOps : public IRRewriter {
private:
  std::stack<std::set<Var>> stringVars;
  
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

  void visit(const Eq *op) {
    if (isString(op->a.type())) {
      lowerStringCompare(CompareOp::EQ, op->a, op->b);
    } else {
      IRRewriter::visit(op);
    }
  }

  void visit(const Ne *op) {
    if (isString(op->a.type())) {
      lowerStringCompare(CompareOp::NE, op->a, op->b);
    } else {
      IRRewriter::visit(op);
    }
  }

  void visit(const Le *op) {
    if (isString(op->a.type())) {
      lowerStringCompare(CompareOp::LE, op->a, op->b);
    } else {
      IRRewriter::visit(op);
    }
  }

  void visit(const Lt *op) {
    if (isString(op->a.type())) {
      lowerStringCompare(CompareOp::LT, op->a, op->b);
    } else {
      IRRewriter::visit(op);
    }
  }

  void visit(const Ge *op) {
    if (isString(op->a.type())) {
      lowerStringCompare(CompareOp::GE, op->a, op->b);
    } else {
      IRRewriter::visit(op);
    }
  }

  void visit(const Gt *op) {
    if (isString(op->a.type())) {
      lowerStringCompare(CompareOp::GT, op->a, op->b);
    } else {
      IRRewriter::visit(op);
    }
  }

  void visit(const Add *op) {
    if (isString(op->a.type())) {
      iassert(isString(op->b.type()));
    } else {
      IRRewriter::visit(op);
    }
  }

  void visit(const AssignStmt *op) {
    if (isString(op->var.getType())) {
      iassert(isString(op->value.type()));

      Stmt freeStmt = CallStmt::make({}, intrinsics::free(), 
                                     {VarExpr::make(op->var)});
      stmts.push_back(freeStmt);

      Var len("len", Int);
      Stmt lenStmt = CallStmt::make({len}, intrinsics::strlen(), {op->value});
      stmts.push_back(lenStmt);
      
      Expr lenExpr = Add::make(VarExpr::make(len), Literal::make(1));
      Stmt mallocStmt = CallStmt::make({op->var}, intrinsics::malloc(), 
                                       {lenExpr});
      stmts.push_back(mallocStmt);

      stmt = CallStmt::make({op->var}, intrinsics::strcpy(), 
                            {VarExpr::make(op->var), op->value});
    } else {
      IRRewriter::visit(op);
    }
  }
  
  void visit(const VarDecl *op) {
    IRRewriter::visit(op);
    if (isString(op->var.getType())) {
      stringVars.top().insert(op->var);
    }
  }

  void visit(const Scope *op) {
    stringVars.emplace();

    Stmt scopedStmt = rewrite(op->scopedStmt);
    for (const auto var : stringVars.top()) {
      const Stmt freeStmt = CallStmt::make({}, intrinsics::free(), 
                                           {VarExpr::make(var)});
      scopedStmt = Block::make(scopedStmt, freeStmt);
    }

    stringVars.pop();
    stmt = Scope::make(scopedStmt);
  }
};

class InsertStringFrees : public IRRewriter {
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
  std::stack<std::set<Var>> stringVars;
  
  using IRRewriter::visit;
 
};

Func lowerStringOps(Func func) {
  func = LowerStringOps().rewrite(func);
  func = insertVarDecls(func);
  return func;
}

}}

