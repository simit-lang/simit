#ifndef SIMIT_LOWER_MAPS_H
#define SIMIT_LOWER_MAPS_H

#include "ir.h"

namespace simit {
namespace ir {

/// Lower map statements to loops. Map assemblies are lowered to loops that
/// store the resulting tensors as specified by Func's Storage descriptor.
Func lowerMaps(Func func);

}}
#endif
