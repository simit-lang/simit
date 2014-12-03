#ifndef SIMIT_LOWER_H
#define SIMIT_LOWER_H

#include "ir.h"


namespace simit {
namespace ir {

class TensorStorage;
typedef std::map<Var,TensorStorage> TensorStorages;

Func lower(Func func);

Func lowerAssemblies(Func func);
Func lowerIndexExpressions(Func func, const TensorStorages &tensorStorages);
Func lowerTensorAccesses(Func func);

}} // namespace simit::ir

#endif
