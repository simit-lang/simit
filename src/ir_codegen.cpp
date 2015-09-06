#include "ir_codegen.h"

#include <vector>

#include "ir_rewriter.h"
#include "ir_queries.h"
#include "ir_builder.h"
#include "indexvar.h"

using namespace std;

namespace simit {
namespace ir {

Stmt initializeLhsToZero(Stmt stmt) {
  class ReplaceRhsWithZero : public IRRewriter {
    using IRRewriter::visit;
    
    void visit(const AssignStmt *op) {
      stmt = AssignStmt::make(op->var, 0.0);
    }

    void visit(const FieldWrite *op) {
      stmt = FieldWrite::make(op->elementOrSet, op->fieldName, 0.0);
    }

    void visit(const TensorWrite *op) {
      stmt = TensorWrite::make(op->tensor, op->indices, 0.0);
    }
  };
  return ReplaceRhsWithZero().rewrite(stmt);
}

Func insertVarDecls(Func func) {
  class InsertVarDeclsRewriter : public IRRewriter {
    /// The set of variables that have already been declared. We do not need a
    /// scoped map here because Vars are compared by identity and cannot shadow.
    set<Var> declared;

    void visit(const Func *f) {
      declared.clear();

      for (auto &argument : f->getArguments()) {
        declared.insert(argument);
      }

      for (auto &result : f->getResults()) {
        declared.insert(result);
      }

      for (auto &constant : f->getEnvironment().getConstants()) {
        declared.insert(constant.first);
      }

      IRRewriter::visit(f);
    }

    void visit(const VarDecl *op) {
      declared.insert(op->var);
      stmt = op;
    }

    void visit(const AssignStmt *op) {
      if (declared.find(op->var) == declared.end()) {
        stmt = Block::make(VarDecl::make(op->var), op);
        declared.insert(op->var);
      }
      else {
        stmt = op;
      }
    }

    void visit(const Map *op) {
      stmt = op;
      for (auto &var : op->vars) {
        if (declared.find(var) == declared.end()) {
          stmt = Block::make(VarDecl::make(var), stmt);
          declared.insert(var);
        }
      }
    }
  };
  return InsertVarDeclsRewriter().rewrite(func);
}

std::pair<Stmt,std::vector<Stmt>> removeVarDecls(Stmt stmt) {
  class RemoveVarDeclsRewriter : public IRRewriter {
  public:
    std::vector<Stmt> varDecls;

    void visit(const VarDecl *op) {
      varDecls.push_back(op);
      stmt = Stmt();
    }
  };
  RemoveVarDeclsRewriter rewriter;

  Stmt result = rewriter.rewrite(stmt);
  return std::pair<Stmt,vector<Stmt>>(result, rewriter.varDecls);
}

Stmt moveVarDeclsToFront(Stmt stmt) {
  std::pair<Stmt,vector<Stmt>> varDecls = removeVarDecls(stmt);
  return (varDecls.second.size() > 0)
      ? Block::make(Block::make(varDecls.second), varDecls.first)
      : varDecls.first;
}

Stmt find(const Var &result, const std::vector<Expr> &exprs, string name,
          function<Expr(Expr,Expr)> compare) {
  iassert(exprs.size() > 0);

  Stmt resultStmt;
  if (exprs.size() == 1) {
    resultStmt = AssignStmt::make(result, exprs[0]);
  }
  else if (exprs.size() == 2) {
    resultStmt = IfThenElse::make(compare(exprs[0], exprs[1]),
                               AssignStmt::make(result, exprs[0]),
                               AssignStmt::make(result, exprs[1]));
  }
  else {
    resultStmt = AssignStmt::make(result, exprs[0]);
    vector<Stmt> tests;
    for (size_t i=1; i < exprs.size(); ++i) {
      Stmt test = IfThenElse::make(compare(exprs[i], result),
                                   AssignStmt::make(result, exprs[i]));
      tests.push_back(test);
    }
    resultStmt = Block::make(resultStmt, Block::make(tests));
  }

  string commentString = result.getName() + " = " + name
                       + "(" + util::join(exprs) + ")";
  return Comment::make(commentString, resultStmt);
}

Stmt min(const Var &result, const std::vector<Expr> &exprs) {
  return find(result, exprs, "min", Lt::make);
}

Stmt max(const Var &result, const std::vector<Expr> &exprs) {
  return find(result, exprs, "max", Gt::make);
}

Expr compare(const vector<Expr> &expressions) {
  iassert(expressions.size() > 0);

  Expr result;
  if (expressions.size() == 1) {
    result = expressions[0];
  }
  else {
    result = Eq::make(expressions[0], expressions[1]);
    for (size_t i=2; i < expressions.size(); ++i) {
      result = And::make(result, Eq::make(expressions[i-1], expressions[i]));
    }
  }
  iassert(result);
  return result;
}

Stmt increment(const Var &var) {
  return AssignStmt::make(var, var + 1);
}

}}
