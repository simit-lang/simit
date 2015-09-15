#ifndef SIMIT_LOWER_MATRIX_MULTIPLY_H
#define SIMIT_LOWER_MATRIX_MULTIPLY_H

#include "ir.h"

namespace simit {
namespace ir {

Stmt lowerMatrixMultiply(Var target, const IndexExpr* indexExpression,
                         Environment* env, Storage* storage);

}}
#endif
