#ifndef SIMIT_STENCIL_NORM_H
#define SIMIT_STENCIL_NORM_H

#include "ir.h"

namespace simit {
namespace ir {

/// Rewrite lattice relative indexing in assembly functions to normalize
/// the row index to zero for all output tensor writes. This matches the
/// pattern of a gather stencil, and allows easy lowering of the map.
Func normalizeRowIndices(Func func);

}}

#endif
