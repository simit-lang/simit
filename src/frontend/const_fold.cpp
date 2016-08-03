#include <memory>

#include "const_fold.h"
#include "fir.h"
#include "ir.h"
#include "fir_rewriter.h"

namespace simit {
namespace fir {

void ConstantFolding::visit(NegExpr::Ptr expr) {
  expr->operand = rewrite<Expr>(expr->operand);
  
  if (isa<IntLiteral>(expr->operand)) {
    const auto operand = to<IntLiteral>(expr->operand);
    if (expr->negate) {
      operand->val *= -1;
    }
    node = operand;
    return;
  } else if (isa<FloatLiteral>(expr->operand)) {
    const auto operand = to<FloatLiteral>(expr->operand);
    if (expr->negate) {
      operand->val *= -1.0;
    }
    node = operand;
    return;
  } else if (isa<ComplexLiteral>(expr->operand)) {
    const auto operand = to<ComplexLiteral>(expr->operand);
    if (expr->negate) {
      operand->val.real *= -1.0;
      operand->val.imag *= -1.0;
    }
    node = operand;
  } else if (isa<DenseTensorLiteral>(expr->operand)) {
    // Helper visitor for negating all elements in tensor literal.
    class NegateTensorLiteral : public FIRVisitor {
      public:
        void negate(DenseTensorLiteral::Ptr tensor) {
          tensor->accept(this);
        }

      private:
        virtual void visit(IntVectorLiteral::Ptr vec) {
          for (unsigned i = 0; i < vec->vals.size(); ++i) {
            vec->vals[i] *= -1;
          }
        }
        virtual void visit(FloatVectorLiteral::Ptr vec) {
          for (unsigned i = 0; i < vec->vals.size(); ++i) {
            vec->vals[i] *= -1.0;
          }
        }
        virtual void visit(ComplexVectorLiteral::Ptr vec) {
          for (unsigned i = 0; i < vec->vals.size(); ++i) {
            vec->vals[i].real *= -1.0;
            vec->vals[i].imag *= -1.0;
          }
        }
    };
    const auto operand = to<DenseTensorLiteral>(expr->operand);
    if (expr->negate) {
      NegateTensorLiteral().negate(operand);
    }
    node = operand;
    return;
  }

  node = expr;
}

void ConstantFolding::visit(TransposeExpr::Ptr expr) {
  expr->operand = rewrite<Expr>(expr->operand);
  
  if (isa<IntLiteral>(expr->operand) || isa<FloatLiteral>(expr->operand) ||
      isa<ComplexLiteral>(expr->operand)) {
    node = expr->operand;
    return;
  } else if (isa<DenseTensorLiteral>(expr->operand)) {
    const auto operand = to<DenseTensorLiteral>(expr->operand);
    if (isa<IntVectorLiteral>(operand) || isa<FloatVectorLiteral>(operand) ||
        isa<ComplexVectorLiteral>(operand)) {
      operand->transposed = !operand->transposed;
      node = operand;
      return;
    }
  }

  node = expr;
}

}
}

