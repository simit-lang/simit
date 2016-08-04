#ifndef SIMIT_FIR_INTRINSICS_H
#define SIMIT_FIR_INTRINSICS_H

#include <vector>
#include "fir.h"

namespace simit {
namespace fir {

std::vector<fir::FuncDecl::Ptr> createIntrinsics();

}}
#endif
