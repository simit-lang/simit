#include "lower_transpose.h"

#include "storage.h"
#include "tensor_index.h"
#include "intrinsics.h"

namespace simit {
namespace ir {

Stmt lowerTranspose(Var target, const IndexExpr* iexpr,
                    Environment* env, Storage* storage) {
  iassert(isa<IndexedTensor>(iexpr->value));
  iassert(isa<VarExpr>(to<IndexedTensor>(iexpr->value)->tensor));
  Var source = to<VarExpr>(to<IndexedTensor>(iexpr->value)->tensor)->var;
  auto sourceIndex = storage->getStorage(source).getTensorIndex();
  auto targetIndex = storage->getStorage(target).getTensorIndex();

  auto sourceType = source.getType().toTensor();
  auto iRange   = sourceType->getOuterDimensions()[0];

  Var  i("i",  Int);
  Var ij("ij", Int);
  Var  j("j",  Int);

  Var locVar(INTERNAL_PREFIX("locVar"), Int);
  Stmt locStmt = CallStmt::make({locVar}, intrinsics::loc(),
                                {i, Load::make(sourceIndex.getColidxArray(),ij),
                                 targetIndex.getRowptrArray(),
                                 targetIndex.getColidxArray()});
  Stmt store = Store::make(target, locVar, Load::make(source, ij));
  Stmt body = Block::make(locStmt, store);

  Expr start = Load::make(sourceIndex.getRowptrArray(), i);
  Expr stop  = Load::make(sourceIndex.getRowptrArray(), i+1);
  Stmt innerLoop  = ForRange::make(ij, start, stop, body);
  Stmt transposeLoop = For::make(i, iRange, innerLoop);

  return transposeLoop;
}
    
}}