#ifndef SIMIT_LOWER_TRANPOSE_H
#define SIMIT_LOWER_TRANPOSE_H

#include "ir.h"

namespace simit {
namespace ir {

Stmt lowerTranspose(Var target, const IndexExpr* indexExpression,
                    Environment* env, Storage* storage);
    
}}
#endif
