#ifndef SIMIT_LOWER_H
#define SIMIT_LOWER_H

#include "ir.h"

namespace simit {
namespace ir {

Func lowerIndexExpressions(Func func);
//Function lowerMaps(Func func);
Func lowerTensorAccesses(Func func);

//Function lowerIntrinsics(Func func);

}} // namespace simit::ir

#endif
