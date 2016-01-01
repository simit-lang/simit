#ifndef SIMIT_TUPLE_READ_REWRITER_H
#define SIMIT_TUPLE_READ_REWRITER_H

#include "hir.h"
#include "hir_rewriter.h"

namespace simit {
namespace hir {

class TupleReadRewriter : public HIRRewriter {
private:
  virtual void visit(TensorReadExpr::Ptr);
};

}
}

#endif

