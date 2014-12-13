#ifndef SIMIT_LOWER_H
#define SIMIT_LOWER_H

#include "ir.h"

namespace simit {
namespace ir {

class Storage;

Func lower(Func func);

/// Lower tensor assemblies to loops that stores the resulting tensors as
/// specified by the 'TensorStorages' descriptors.
Func lowerAssemblies(Func func);

Func lowerIndexExpressions(Func func);

/// Lower tensor reads and writes to loads and stores. Loads are lowered based
/// on the storage scheme of the 
Func lowerTensorAccesses(Func func);

}} // namespace simit::ir

#endif
