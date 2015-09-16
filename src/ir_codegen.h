/// @file Contains functions that create Simit code.

#ifndef SIMIT_IR_CODEGEN_H
#define SIMIT_IR_CODEGEN_H

#include "ir.h"

namespace simit {
namespace ir {

Stmt initializeLhsToZero(Stmt stmt);

/// Compute the smallest value of the given Exprs and assign the result to var.
Stmt min(const Var &result, const std::vector<Expr> &exprs);

/// Compute the largest value of the given Exprs and assign the result to var.
Stmt max(const Var &result, const std::vector<Expr> &exprs);

/// Check whether all the expressions are equal.
Expr compare(const std::vector<Expr> &expressions);

/// Increment var by 1. Assumes var is an integer.
Stmt increment(const Var &var);

}}

#endif
