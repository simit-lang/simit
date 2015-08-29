#ifndef SIMIT_LOWER_SCATTER_WORKSPACE_H
#define SIMIT_LOWER_SCATTER_WORKSPACE_H

#include <vector>
#include "ir.h"
#include "loops.h"

namespace simit {
namespace ir {

/// Lower the index expression to a loop nest that iterates over operand indices
/// and scatters computed values into a dense workspace. In the course of
/// lowering this functon may add arrays and indices to the environment.
Stmt lowerScatterWorkspace(Var target, const IndexExpr* indexExpression,
                           Environment* env);

}}
#endif
