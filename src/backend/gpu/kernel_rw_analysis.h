#ifndef SIMIT_KERNEL_RW_ANALYSIS_H
#define SIMIT_KERNEL_RW_ANALYSIS_H

#include "ir.h"

namespace simit {
namespace ir {

std::set<Var> findRootVars(Func func);
Func kernelRWAnalysis(Func func);

}}  // namespace simit::ir

#endif
