#ifndef SIMIT_REWRITE_MEMSETS_H
#define SIMIT_REWRITE_MEMSETS_H

#include "ir.h"

namespace simit {
namespace ir {

/// Rewrite all memsets as system loops. This allows the later loops
/// to perform loop fusion and kernel emission.
Func rewriteMemsets(Func func);

}} // simit::ir

#endif
