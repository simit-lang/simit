#include "node_replacer.h"

using namespace simit::ir;
using namespace simit::ir::program_analysis;

Expr NodeReplacer::rewrite(Expr e) {
  if (e.defined()) {
    e.accept(this);
    if (!disableLevel) {
      if (exprReplacement.defined()) {
        e = exprReplacement;
      } else if (expr.defined()) {
        e = expr;
      }
    }
  }
  exprReplacement = Expr();
  stmtReplacement = Stmt();
  funcReplacement = Func();
  expr = Expr();
  stmt = Stmt();
  func = Func();
  return e;
}

Stmt NodeReplacer::rewrite(Stmt s) {
  if (s.defined()) {
    s.accept(this);
    if (!disableLevel) {
      if (stmtReplacement.defined()) {
        s = stmtReplacement;
      } else if (stmt.defined()) {
        s = stmt;
      }
    }
  }
  exprReplacement = Expr();
  stmtReplacement = Stmt();
  funcReplacement = Func();
  expr = Expr();
  stmt = Stmt();
  func = Func();
  return s;
}

Func NodeReplacer::rewrite(Func f) {
  if (f.defined()) {
    f.accept(this);
    if (!disableLevel) {
      if (funcReplacement.defined()) {
        f = funcReplacement;
      } else if (func.defined()) {
        f = func;
      }
    }
  }
  exprReplacement = Expr();
  stmtReplacement = Stmt();
  funcReplacement = Func();
  expr = Expr();
  stmt = Stmt();
  func = Func();
  return f;
}

void NodeReplacer::enable() {
  iassert(disableLevel);
  disableLevel--;
}

void NodeReplacer::disable() {
  disableLevel++;
}
