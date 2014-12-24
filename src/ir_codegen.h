#ifndef SIMIT_IR_CODEGEN_H
#define SIMIT_IR_CODEGEN_H

#include "ir.h"

namespace simit {
namespace ir {

struct CompoundOperator {
  enum Kind { Add };
  Kind kind;
  CompoundOperator(Kind kind) : kind(kind) {}
};
Stmt makeCompound(Stmt stmt, CompoundOperator cop);

Stmt initializeLhsToZero(Stmt stmt);

}}

#endif
