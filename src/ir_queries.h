#ifndef SIMIT_EXPR_QUERIES_H
#define SIMIT_EXPR_QUERIES_H

#include "ir.h"
#include "indexvar.h"

namespace simit {
namespace ir {

std::vector<IndexVar> getFreeVars(Expr expr);
std::vector<IndexVar> getReductionVars(Expr expr);

bool containsFreeVar(Expr expr);
bool containsFreeVar(Stmt stmt);

bool containsReductionVar(Expr expr);
bool containsReductionVar(Stmt stmt);

bool containsIndexedTensor(Expr expr);

size_t countIndexVars(Expr expr);

/// Returns true if the statement has been flattened (only contains one index
/// expression), and false otherwise.
bool isFlattened(Stmt stmt);

/// Returns true if it is an assignment, tensor write or field write, whose
/// rhs is a blocked tensor
bool isBlocked(Stmt stmt);

/// Returns the call tree of `func`. The call tree constains all functions
/// (transitively) called from `func`.
std::vector<Func> getCallTree(Func func);

}}

#endif
