/// @file Contains functions that transform Simit code in various useful ways.

#ifndef SIMIT_IR_TRANSFORMS_H
#define SIMIT_IR_TRANSFORMS_H

#include "ir.h"
#include <vector>

namespace simit {
namespace ir {

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

/// Makes all the system tensors declared in the func body global variables.
/// A system tensor is a tensor whose dimensions include at least one Simit set.
/// The global variables are added to the resulting Funcs environment.
Func makeSystemTensorsGlobal(Func func);

/// Goes through the use-def graph of func, and insert a statement (can be a
/// loop) to zero initialize each numeric type variable if it is used without
/// being defined first. The initialization statement is inserted after VarDecl.
Func insertInitializations(Func func);

}}
#endif
