#ifndef SIMIT_LOWER_INDEXEXPRS_H
#define SIMIT_LOWER_INDEXEXPRS_H

#include "ir.h"

namespace simit {
namespace ir {

Stmt lowerIndexStatement(Stmt stmt, Environment* environment, Storage storage);

}}
#endif
