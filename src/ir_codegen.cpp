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
      Expr zeroVal;
      switch (op->var.getType().toTensor()->getComponentType().kind) {
        case ScalarType::Int:
          zeroVal = Literal::make(0);
          break;
        case ScalarType::Float:
          zeroVal = Literal::make(0.0);
          break;
        case ScalarType::Boolean:
          zeroVal = Literal::make(false);
          break;
        default:
          unreachable;
          break;
      }
      stmt = AssignStmt::make(op->var, zeroVal);
    }

    void visit(const FieldWrite *op) {
      Expr zeroVal;
      switch (op->value.type().toTensor()->getComponentType().kind) {
        case ScalarType::Int:
          zeroVal = Literal::make(0);
          break;
        case ScalarType::Float:
          zeroVal = Literal::make(0.0);
          break;
        case ScalarType::Boolean:
          zeroVal = Literal::make(false);
          break;
        default:
          unreachable;
          break;
      }
      stmt = FieldWrite::make(op->elementOrSet, op->fieldName, 0.0);
    }

    void visit(const TensorWrite *op) {
      Expr zeroVal;
      switch (op->value.type().toTensor()->getComponentType().kind) {
        case ScalarType::Int:
          zeroVal = Literal::make(0);
          break;
        case ScalarType::Float:
          zeroVal = Literal::make(0.0);
          break;
        case ScalarType::Boolean:
          zeroVal = Literal::make(false);
          break;
        default:
          unreachable;
          break;
      }
      stmt = TensorWrite::make(op->tensor, op->indices, zeroVal);
    }
  };
  return ReplaceRhsWithZero().rewrite(stmt);
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
  iassert(result.defined());
  return result;
}

Stmt increment(const Var &var) {
  return AssignStmt::make(var, var + 1);
}

}}
