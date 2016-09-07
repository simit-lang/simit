#include "ir_codegen.h"

#include <vector>

#include "ir_rewriter.h"
#include "ir_queries.h"
#include "ir_builder.h"
#include "indexvar.h"

using namespace std;

namespace simit {
namespace ir {

Expr getZeroVal(const TensorType *type) {
  switch (type->getComponentType().kind) {
    case ScalarType::Int:
      return Literal::make(0);
    case ScalarType::Float:
      return Literal::make(0.0);
    case ScalarType::Boolean:
      return Literal::make(false);
    case ScalarType::Complex:
      return Literal::make(double_complex(0.0, 0.0));
    default:
      unreachable;
      return Expr();
  }
}

Stmt initializeLhsToZero(Stmt stmt) {
  class ReplaceRhsWithZero : public IRRewriter {
    void visit(const AssignStmt *op) {
      Expr zeroVal = getZeroVal(op->var.getType().toTensor());
      stmt = AssignStmt::make(op->var, zeroVal);
    }

    void visit(const FieldWrite *op) {
      Expr zeroVal = getZeroVal(op->value.type().toTensor());
      stmt = FieldWrite::make(op->elementOrSet, op->fieldName, zeroVal);
    }

    void visit(const TensorWrite *op) {
      Expr zeroVal = getZeroVal(op->tensor.type().toTensor());
      stmt = TensorWrite::make(op->tensor, op->indices, zeroVal);
    }
  };
  return ReplaceRhsWithZero().rewrite(stmt);
}

Stmt initializeTensorToZero(Stmt stmt) {
  class BuildInitLoopNest : public IRRewriter {
    Stmt makeLoopNest(Expr tensor) {
      const TensorType *ttype = tensor.type().toTensor();
      std::vector<Var> indices;
      std::vector<Expr> indicesExpr;
      std::vector<IndexSet> domains;
      for (auto &is : ttype->getOuterDimensions()) {
        indices.push_back(Var("index", Int));
        indicesExpr.push_back(indices.back());
        domains.push_back(is);
      }

      Stmt stmt;
      // Recursively build write statement with any inner loops
      if (!isScalar(ttype->getBlockType())) {
        stmt = makeLoopNest(TensorRead::make(tensor, indicesExpr));
      }
      else {
        stmt = TensorWrite::make(tensor, indicesExpr, getZeroVal(ttype));
      }

      // Wrap in current level loops
      for (int i = domains.size()-1; i >= 0; --i) {
        stmt = For::make(indices[i], domains[i], stmt);
      }
      return stmt;
    }

    void visit(const AssignStmt *op) {
      stmt = makeLoopNest(op->var);
    }

    void visit(const FieldWrite *op) {
      stmt = makeLoopNest(FieldRead::make(op->elementOrSet, op->fieldName));
    }
  };
  return BuildInitLoopNest().rewrite(stmt);
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
