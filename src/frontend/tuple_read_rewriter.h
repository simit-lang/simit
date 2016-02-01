#ifndef SIMIT_TUPLE_READ_REWRITER_H
#define SIMIT_TUPLE_READ_REWRITER_H

#include "hir.h"
#include "hir_rewriter.h"

namespace simit {
namespace hir {

// Analysis for identifying all tuple reads and rewriting higher-level IR 
// appropriately. Needed since Simit grammar cannot distinguish between tuple 
// reads and tensor reads. Relies on result of type analysis.
class TupleReadRewriter : public HIRRewriter {
private:
  virtual void visit(TensorReadExpr::Ptr);
};

}
}

#endif

