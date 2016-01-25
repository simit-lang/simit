#include <memory>

#include "tuple_read_rewriter.h"
#include "hir.h"
#include "types.h"

namespace simit {
namespace hir {

void TupleReadRewriter::visit(TensorReadExpr::Ptr expr) {
  HIRRewriter::visit(expr);

  if (expr->tensor->type.size() != 1 || !expr->tensor->type[0].isTuple() || 
      expr->indices.size() != 1 || expr->indices[0]->isSlice()){
    return;
  }

  auto tupleRead = std::make_shared<TupleReadExpr>();
  tupleRead->setLoc(expr);
  tupleRead->tuple = expr->tensor;
  tupleRead->index = to<ExprParam>(expr->indices[0])->expr;
  
  node = tupleRead;
}

}
}

