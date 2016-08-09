#ifndef SIMIT_LOWER_H
#define SIMIT_LOWER_H

#include "ir.h"

namespace simit {
namespace ir {

/// Optimize and lower `func` into the low level part of the Simit IR, that is
/// is supported by backends. If `print` is true, then the IR will be printed
/// to stdout between each lowering step.
Func lower(Func func, std::ostream* os=nullptr, bool time=false);

}}
#endif
