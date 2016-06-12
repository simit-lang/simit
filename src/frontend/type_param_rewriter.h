#ifndef SIMIT_TYPE_PARAM_REWRITER_H
#define SIMIT_TYPE_PARAM_REWRITER_H

#include "hir.h"
#include "hir_rewriter.h"

namespace simit {
namespace hir {

class TypeParamRewriter : public HIRRewriter {
private:
  virtual void visit(FuncDecl::Ptr);
};

}
}

#endif

