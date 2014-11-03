#ifndef SIMIT_GATHER_BUFFERS_H
#define SIMIT_GATHER_BUFFERS_H

#include "ir.h"

namespace simit {
namespace internal {

std::vector<ir::Var> gatherLocalBuffers(ir::Func func);

}}

#endif
