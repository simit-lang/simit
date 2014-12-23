#ifndef SIMIT_EXPR_QUERIES_H
#define SIMIT_EXPR_QUERIES_H

#include "ir.h"
#include "indexvar.h"

namespace simit {
namespace ir {

std::vector<IndexVar> getFreeVars(Expr expr);
std::vector<IndexVar> getReductionVars(Expr expr);

bool containsReduction(Expr expr);
bool containsReduction(Stmt stmt);

/// Returns true if the statement has been flattened (only contains one index
/// expression), and false otherwise.
bool isFlattened(Stmt stmt);

}}

#endif
