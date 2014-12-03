#ifndef SIMIT_LOWER_H
#define SIMIT_LOWER_H

#include "ir.h"

namespace simit {
namespace ir {

Func lower(Func func);

Func lowerAssemblies(Func func);
Func lowerIndexExpressions(Func func);
Func lowerTensorAccesses(Func func);

}} // namespace simit::ir

#endif
