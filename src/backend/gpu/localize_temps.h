#ifndef SIMIT_LOCALIZE_TEMPS_H
#define SIMIT_LOCALIZE_TEMPS_H

#include "ir.h"

namespace simit {
namespace ir {

// Any temporary variables used within kernels need to be
// reduced to local allocations
Func localizeTemps(Func func);

}}  // namespace simit::ir

#endif
