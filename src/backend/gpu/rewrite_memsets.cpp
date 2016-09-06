#include "rewrite_memsets.h"

#include "ir_codegen.h"
#include "ir_builder.h"
#include "ir_rewriter.h"
#include "storage.h"

namespace simit {
namespace ir {

class MemsetRewriter : public IRRewriter {
public:
  MemsetRewriter(Storage storage) : storage(storage) {}
private:
  Storage storage;

  bool isMemset(Type targetType, Expr value) {
    if (!targetType.isTensor()) return false;
    iassert(value.type().isTensor());

    const TensorType *ttype = targetType.toTensor();
    const TensorType *vtype = value.type().toTensor();
    return (ttype->order() > 0 && vtype->order() == 0 &&
            isa<Literal>(value) && to<Literal>(value)->isAllZeros());
  }

  bool isStorageSparse(Var tensor) {
    if (!storage.hasStorage(tensor)) {
      return false;
    }
    const TensorStorage& ts = storage.getStorage(tensor);
    tassert(ts.getKind() != TensorStorage::Stencil)
        << "Don't know how to deal with Stencil storage yet";
    return (ts.getKind() == TensorStorage::Indexed);
  }

  Expr makeIndexExpr(Type targetType, Expr value) {
    IndexVarFactory factory;
    std::vector<IndexVar> indices;
    for (auto &d : targetType.toTensor()->getDimensions()) {
      indices.push_back(factory.createIndexVar(d));
    }
    return IndexExpr::make(indices, value);
  }

  void visit(const FieldWrite *op) {
    Type fieldType = getFieldType(op->elementOrSet, op->fieldName);
    if (isMemset(fieldType, op->value)) {
      stmt = initializeTensorToZero(op);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const AssignStmt *op) {
    if (isMemset(op->var.getType(), op->value) && !isStorageSparse(op->var)) {
      // Expr value = makeIndexExpr(op->var.getType(), op->value);
      // stmt = AssignStmt::make(op->var, value);
      stmt = initializeTensorToZero(op);
    }
    else {
      IRRewriter::visit(op);
    }
  }
};

Func rewriteMemsets(Func func) {
  return MemsetRewriter(func.getStorage()).rewrite(func);
}

}} // simit::ir
