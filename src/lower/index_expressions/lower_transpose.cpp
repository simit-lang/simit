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
                                {Load::make(sourceIndex.getColidxArray(),ij), i,
                                 targetIndex.getRowptrArray(),
                                 targetIndex.getColidxArray()});
  Stmt body;
  auto blockType = *sourceType->getBlockType().toTensor();
  if (blockType.order() == 0) {  // Not blocked
    body = Store::make(target, locVar, Load::make(source, ij));
  }
  else {  // Blocked
    iassert(blockType.order() == 2);
    Var ii("ii", Int);
    Var jj("jj", Int);
    auto d1 = blockType.getOuterDimensions()[0];
    auto d2 = blockType.getOuterDimensions()[1];
    Expr l1 = Length::make(d1);
    Expr l2 = Length::make(d2);

    body = Store::make(target, locVar*l1*l2 + ii*l2 + jj,
                       Load::make(source, ij*l1*l2 + ii*l2 + jj));

    body = For::make(jj, ForDomain(d2), body);
    body = For::make(ii, ForDomain(d1), body);
  }
  iassert(body.defined());

  Expr start = Load::make(sourceIndex.getRowptrArray(), i);
  Expr stop  = Load::make(sourceIndex.getRowptrArray(), i+1);
  Stmt innerLoop  = ForRange::make(ij, start, stop, Block::make(locStmt, body));
  return For::make(i, iRange, innerLoop);
}
    
}}
