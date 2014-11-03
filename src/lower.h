#ifndef SIMIT_LOWER_H
#define SIMIT_LOWER_H

#include "ir.h"

namespace simit {
namespace ir {

class UseDef;

Func lower(Func func);

Func insertTemporaries(Func func);
Func flattenIndexExpressions(Func func);
Func lowerIndexExpressions(Func func);
Func lowerMaps(Func func);
Func lowerTensorAccesses(Func func);

Expr flattenIndexExpressions(Expr expr);
Stmt flattenIndexExpressions(Stmt stmt);
Stmt lowerIndexExpressions(Stmt stmt, const UseDef &ud);
Stmt lowerMaps(Stmt stmt);
Stmt lowerTensorAccesses(Stmt stmt);

}} // namespace simit::ir

#endif
