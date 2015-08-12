#ifndef SIMIT_LOWER_SCATTER_WORKSPACE_H
#define SIMIT_LOWER_SCATTER_WORKSPACE_H

#include "ir.h"
#include "tensor_index.h"

namespace simit {
namespace ir {

Stmt lower_scatter_workspace(Expr target, const IndexExpr *indexExpression);

}}
#endif
