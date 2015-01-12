#ifndef SIMIT_IR_CODEGEN_H
#define SIMIT_IR_CODEGEN_H

#include "ir.h"

namespace simit {
namespace ir {

Stmt initializeLhsToZero(Stmt stmt);

/// Utility function that inserts VarDecl statements in front of every
/// assignment to a variable that has not been declared. The Simit system
/// expects every variable to have been declared, and this function makes it
/// easier for frontends, since the declaration invariant can be imposed after
/// IR construction.
Func insertVarDecls(Func func);

}}

#endif
