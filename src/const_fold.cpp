#include <memory>

#include "const_fold.h"
#include "hir.h"
#include "ir.h"
#include "hir_rewriter.h"

namespace simit {
namespace hir {

void ConstantFolding::visit(NegExpr::Ptr expr) {
  expr->operand = rewrite<Expr>(expr->operand);
  if (isa<IntLiteral>(expr->operand)) {
    const auto negOperand = to<IntLiteral>(expr->operand);
    negOperand->val *= -1;
    node = negOperand;
    return;
  } else if (isa<FloatLiteral>(expr->operand)) {
    const auto negOperand = to<FloatLiteral>(expr->operand);
    negOperand->val *= -1.0;
    node = negOperand;
    return;
  } else if (isa<DenseTensorLiteral>(expr->operand)) {
    class NegateTensor : public HIRRewriter {
      public:
        virtual void visit(DenseIntVector::Ptr vec) {
          for (unsigned i = 0; i < vec->vals.size(); ++i) {
            vec->vals[i] *= -1;
          }
          node = vec;
        }
        virtual void visit(DenseFloatVector::Ptr vec) {
          for (unsigned i = 0; i < vec->vals.size(); ++i) {
            vec->vals[i] *= -1.0;
          }
          node = vec;
        }
    };
    const auto negOperand = to<DenseTensorLiteral>(expr->operand);
    negOperand->tensor = 
        NegateTensor().rewrite<DenseTensorElement>(negOperand->tensor);
    node = negOperand;
    return;
  }
  node = expr;
}

void ConstantFolding::visit(TransposeExpr::Ptr expr) {
  expr->operand = rewrite<Expr>(expr->operand);
  if (isa<IntLiteral>(expr->operand) || isa<FloatLiteral>(expr->operand)) {
    node = expr->operand;
    return;
  } else if (isa<DenseTensorLiteral>(expr->operand)) {
    const auto transposedOperand = to<DenseTensorLiteral>(expr->operand);
    if (isa<DenseIntVector>(transposedOperand->tensor) || 
        isa<DenseFloatVector>(transposedOperand->tensor)) {
      transposedOperand->transposed = !transposedOperand->transposed;
      node = transposedOperand;
      return;
    }
  }
  node = expr;
}

}
}

