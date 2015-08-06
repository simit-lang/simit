#ifndef SIMIT_LOWER_INDEX_EXPRESSIONS_H
#define SIMIT_LOWER_INDEX_EXPRESSIONS_H

#include "ir.h"

namespace simit {
namespace ir {


Stmt lower(Expr target, const IndexExpr *indexExpression);

}}
#endif
