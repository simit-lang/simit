#include "lower_tensor_utils.h"

#include "substitute.h"

namespace simit {
namespace ir {

Stmt rewriteToBlocked(Stmt stmt, vector<Var> inductionVars, Expr blockSize) {
  Var ii("ii", inductionVars[0].getType());
  map<Expr,Expr> substitutions;
  for (const Var& inductionVar : inductionVars) {
    substitutions.insert({inductionVar, inductionVar*blockSize + ii});
  }
  return ForRange::make(ii, 0, blockSize, substitute(substitutions, stmt));
}

}}
