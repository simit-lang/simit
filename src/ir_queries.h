#ifndef SIMIT_EXPR_QUERIES_H
#define SIMIT_EXPR_QUERIES_H

#include "ir.h"
#include "indexvar.h"

namespace simit {
namespace ir {

std::vector<IndexVar> getFreeVars(Expr expr);
std::vector<IndexVar> getReductionVars(Expr expr);

}}

#endif
