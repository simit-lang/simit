#ifndef SIMIT_LOWER_H
#define SIMIT_LOWER_H

#include "ir.h"

namespace simit {
namespace ir {

Func lower(Func func);

Func insertTemporaries(Func func);
Func lowerIndexExpressions(Func func);
Func lowerMaps(Func func);
Func lowerTensorAccesses(Func func);

//Function lowerIntrinsics(Func func);

}} // namespace simit::ir

#endif
