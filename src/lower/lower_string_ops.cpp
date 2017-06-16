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
  std::stack<std::set<Var>> stringVars;
  
  using IRRewriter::visit;

  enum class CompareOp {EQ, NE, LE, LT, GE, GT};

  void lowerStringCompare(CompareOp op, Expr a, Expr b) {
    simit_iassert(isString(a.type()));
    simit_iassert(isString(b.type()));

    a = rewrite(a);
    b = rewrite(b);
    
    Var tmp("tmp", Int);
    Stmt cmpStmt = CallStmt::make({tmp}, intrinsics::strcmp(), {a, b});
    IRRewriter::spill(cmpStmt);

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
        simit_unreachable;
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

  void lowerStringConcat(Var target, Expr op) {
    class GetOperands : public IRVisitor {
      public:
        GetOperands(std::vector<Expr> &operands) : operands(operands) {}

      private:
        std::vector<Expr> &operands;

        void visit(const Literal *op) {
          operands.push_back(op);
        }

        void visit(const VarExpr *op) {
          operands.push_back(op);
        }
    };

    std::vector<Expr> operands;
    GetOperands getOps(operands);
    op.accept(&getOps);

    Expr lenExpr = Literal::make(1);
    for (const auto opnd : operands) {
      Var len("len", Int);
      Stmt lenStmt = CallStmt::make({len}, intrinsics::strlen(), {opnd});
      IRRewriter::spill(lenStmt);
      
      lenExpr = Add::make(lenExpr, VarExpr::make(len));
    }
    
    Expr targetExpr = VarExpr::make(target);
    Stmt freeStmt = CallStmt::make({}, intrinsics::free(), {targetExpr});
    IRRewriter::spill(freeStmt);

    Stmt mallocStmt = CallStmt::make({target}, intrinsics::malloc(), {lenExpr});
    IRRewriter::spill(mallocStmt);

    Stmt cpyStmt = CallStmt::make({target}, intrinsics::strcpy(), 
                                  {targetExpr, operands[0]});
    IRRewriter::spill(cpyStmt);

    for (unsigned i = 1; i < operands.size(); ++i) {
      Stmt catStmt = CallStmt::make({target}, intrinsics::strcat(),
                                    {targetExpr, operands[i]});
      IRRewriter::spill(catStmt);
    }
  }

  void visit(const Add *op) {
    if (isString(op->a.type())) {
      simit_iassert(isString(op->b.type()));

      Var tmp("tmp", String);
      stringVars.top().insert(tmp);

      lowerStringConcat(tmp, op);
      expr = VarExpr::make(tmp);
    } else {
      IRRewriter::visit(op);
    }
  }

  void visit(const AssignStmt *op) {
    if (isString(op->var.getType())) {
      simit_iassert(isString(op->value.type()));
      lowerStringConcat(op->var, op->value);
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

Func lowerStringOps(Func func) {
  func = LowerStringOps().rewrite(func);
  func = insertVarDecls(func);
  return func;
}

}}

