#ifndef SIMIT_LOWER_H
#define SIMIT_LOWER_H

#include "ir.h"

namespace simit {
namespace ir {

/// Optimize and lower `func` into the low level part of the Simit IR, that is
/// is supported by backends. If `print` is true, then the IR will be printed
/// to stdout between each lowering step.
Func lower(Func func, bool print=false);


/// Lower map statements to loops. Map assemblies are lowered to loops that
/// store the resulting tensors as specified by Func's Storage descriptor.
Func lowerMaps(Func func);

Func lowerIndexExpressions(Func func);

/// Lower tensor reads and writes to loads and stores. Loads are lowered based
/// on the storage scheme of the 
Func lowerTensorAccesses(Func func);

}}
#endif
