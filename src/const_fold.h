#ifndef SIMIT_CONST_FOLD_H
#define SIMIT_CONST_FOLD_H

#include "hir.h"
#include "hir_rewriter.h"

namespace simit {
namespace hir {

class ConstantFolding : public HIRRewriter {
public:
  virtual void visit(NegExpr::Ptr);
  virtual void visit(TransposeExpr::Ptr);
};

}
}

#endif

