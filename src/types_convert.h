/// This file converts between types in the simit namespace and types in the
/// simit::ir namespace.

#ifndef SIMIT_CONVERT_TYPES_H
#define SIMIT_CONVERT_TYPES_H

#include "tensor_type.h"
#include "types.h"

namespace simit {
class TensorType;

namespace ir {
class Type;

ir::Type convert(const simit::TensorType &tensorType);
simit::TensorType convert(const ir::Type &tensorType);

}}
#endif
