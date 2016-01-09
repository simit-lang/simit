#ifndef SIMIT_CONST_FOLD_H
#define SIMIT_CONST_FOLD_H

#include "hir.h"
#include "hir_rewriter.h"

namespace simit {
namespace hir {

// Constant folding pass for taking care of negation and transpose of dense 
// tensor literals. Needed since tests can only have literals as operands.
class ConstantFolding : public HIRRewriter {
private:
  virtual void visit(NegExpr::Ptr);
  virtual void visit(TransposeExpr::Ptr);
};

}
}

#endif

