#include "rewrite_system_assign.h"

#include "ir_builder.h"
#include "ir_rewriter.h"

namespace simit {
namespace ir {

class SystemAssignRewriter : public IRRewriter {
private:
  IRBuilder builder;
public:
  using IRRewriter::visit;

  void visit(const FieldWrite *op) {
    if (!isa<IndexExpr>(op->value)) {
      // TODO: Abstract away this logic
      Type fieldType = getFieldType(op->elementOrSet, op->fieldName);
      Type valueType = op->value.type();
      const TensorType *ftype = fieldType.toTensor();
      const TensorType *vtype = valueType.toTensor();
      if (ftype->order() == vtype->order() &&
          ftype->hasSystemDimensions()) {
        auto indexed = builder.unaryElwiseExpr(IRBuilder::Copy, op->value);
        stmt = FieldWrite::make(op->elementOrSet, op->fieldName,
                                indexed, op->cop);
        return;
      }
    }
    IRRewriter::visit(op);
  }

  void visit(const AssignStmt *op) {
    if (!isa<IndexExpr>(op->value)) {
      Type valueType = op->value.type();
      if (valueType.toTensor()->hasSystemDimensions()) {
        auto indexed = builder.unaryElwiseExpr(IRBuilder::Copy, op->value);
        stmt = AssignStmt::make(op->var, indexed, op->cop);
        return;
      }
    }
    IRRewriter::visit(op);
  }
};

Func rewriteSystemAssigns(Func func) {
  return SystemAssignRewriter().rewrite(func);
}

}}  // namespace simit::ir
