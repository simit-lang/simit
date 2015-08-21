#ifndef SIMIT_LOWER_ACCESSES_H
#define SIMIT_LOWER_ACCESSES_H

#include "ir.h"

namespace simit {
namespace ir {

/// Lower tensor reads and writes to loads and stores. Loads are lowered based
/// on the storage scheme of the 
Func lowerTensorAccesses(Func func);

}}
#endif
