#include "rewrite_system_assign.h"

#include "ir_builder.h"
#include "ir_rewriter.h"

namespace simit {
namespace ir {

class SystemAssignRewriter : public IRRewriter {
public:
  using IRRewriter::visit;

  void visit(const FieldWrite *op) {
    // TODO: Abstract away this logic
    Type fieldType = getFieldType(op->elementOrSet, op->fieldName);
    Type valueType = op->value.type();
    if (fieldType.toTensor()->order() == valueType.toTensor()->order() &&
        fieldType.toTensor()->isSparse()) {
      IRBuilder builder;
      auto indexed = builder.unaryElwiseExpr(IRBuilder::None, op->value);
      stmt = FieldWrite::make(op->elementOrSet, op->fieldName,
                              indexed, op->cop);
      return;
    }
    IRRewriter::visit(op);
  }
};

Func rewriteSystemAssigns(Func func) {
  return SystemAssignRewriter().rewrite(func);
}

}}  // namespace simit::ir
