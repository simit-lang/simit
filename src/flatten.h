#ifndef SIMIT_FLATTEN_H
#define SIMIT_FLATTEN_H

#include "ir.h"

namespace simit {
namespace ir {

Stmt flattenIndexExpressions(Stmt stmt);
Func flattenIndexExpressions(Func func);

}} // namespace simit::ir

#endif
