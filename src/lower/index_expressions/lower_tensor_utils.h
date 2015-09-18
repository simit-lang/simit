#ifndef SIMIT_LOWER_TENSOR_UTILS_H
#define SIMIT_LOWER_TENSOR_UTILS_H

#include <vector>

#include "ir.h"

using namespace std;

namespace simit {
namespace ir {

/// Rewrite a statement looped over a block of size `blockSize`, substituting
/// induction variables appropriately.
Stmt rewriteToBlocked(Stmt stmt, vector<Var> inductionVars, Expr blockSize);

}}

#endif
