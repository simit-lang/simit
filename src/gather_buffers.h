#ifndef SIMIT_GATHER_BUFFERS_H
#define SIMIT_GATHER_BUFFERS_H

#include "ir.h"

namespace simit {
namespace internal {

/// Gather all the local buffers used in the function. Local buffers are not
/// allocated inside a time-step so this function gathers them so that they can
/// be pre-allocated.
std::vector<ir::Var> gatherLocalBuffers(ir::Func func);

}}

#endif
