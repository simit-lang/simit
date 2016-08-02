#ifndef SIMIT_LOWER_STENCIL_ASSEMBLIES_H
#define SIMIT_LOWER_STENCIL_ASSEMBLIES_H

#include "stencil_norm.h"

#include "ir.h"
#include "ir_rewriter.h"

namespace simit {
namespace ir {

Func lowerStencilAssemblies(Func func) {
  func = normalizeRowIndices(func);
  return func;
}

}}

#endif
