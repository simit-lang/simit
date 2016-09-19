#ifndef SIMIT_VAR_REPLACE_REWRITER_H
#define SIMIT_VAR_REPLACE_REWRITER_H

#include "ir.h"

namespace simit {
namespace ir {

Stmt replaceVar(Stmt stmt, Var init, Var final);
Func replaceVar(Func func, Var init, Var final);

Stmt replaceVarByExpr(Stmt stmt, Var init, Expr final);

}}  // namespace simit::ir

#endif
