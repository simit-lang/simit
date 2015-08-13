#ifndef SIMIT_IR_CODEGEN_H
#define SIMIT_IR_CODEGEN_H

#include "ir.h"
#include <vector>

namespace simit {
namespace ir {

Stmt initializeLhsToZero(Stmt stmt);

/// Inserts VarDecl statements in front of every assignment to a variable that
/// has not been declared. The Simit system expects every variable to have been
/// declared, and this function makes it easier for frontends, since the
/// declaration invariant can be imposed after IR construction.
Func insertVarDecls(Func func);

/// Removes the VarDecl statements from `stmt` and returns them together with
/// the rewritten statement.
std::pair<Stmt,std::vector<Stmt>> removeVarDecls(Stmt stmt);

/// Moves VarDecl statements from within `stmt` to in front of it.
Stmt moveVarDeclsToFront(Stmt stmt);

/// Compute the smallest value of the given Exprs and assign the result to var.
Stmt min(const Var &result, const std::vector<Expr> &exprs);

/// Compute the largest value of the given Exprs and assign the result to var.
Stmt max(const Var &result, const std::vector<Expr> &exprs);

/// Increment var by 1. Assumes var is an integer.
Stmt increment(const Var &var);

}}

#endif
