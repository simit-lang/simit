#ifndef SIMIT_LOWER_SCATTER_WORKSPACE_H
#define SIMIT_LOWER_SCATTER_WORKSPACE_H

#include <vector>
#include "ir.h"
#include "loops.h"

namespace simit {
namespace ir {

Stmt lowerScatterWorkspace(Var target, const IndexExpr *indexExpression);

}}
#endif
