#ifndef SIMIT_IR_CODEGEN_H
#define SIMIT_IR_CODEGEN_H

#include <memory>

#include "ir.h"
#include "location.hh"
#include "errors.h"

namespace simit {
namespace ir {

// TODO: Change error handling to use LocationDiagnostics, which is retrieved
//       from Diagnostics using Diagnostics(location), e.g.
//       auto negExpr = unaryElwiseExpr(IndexExpr::NEG, expr, diagnostics(@2));

/// A factory for creating index variables with unique names.
class IndexVarFactory {
public:
  IndexVarFactory() : nameID(0) {}

  IndexVar makeIndexVar(const IndexDomain &domain);
  IndexVar makeIndexVar(const IndexDomain &domain, ReductionOperator rop);

private:
  int nameID;
  std::string makeName();
};

enum UnaryOperator { None, Neg };
enum BinaryOperator { Add, Sub, Mul, Div };

Expr unaryElwiseExpr(UnaryOperator op, Expr e);

/// Apply the operator element-wise to the l and r.  Generally this requires
/// the types of each operand to be identical, but as a special case iff there
/// are two operands an one of them is a scalar, then the operator is applied
/// to the combinatio of that scalar and each element in the other operand.
Expr binaryElwiseExpr(Expr l, BinaryOperator op, Expr r);

Expr innerProduct(Expr l, Expr r);

Expr outerProduct(Expr l, Expr r);

Expr gemv(Expr l, Expr r);

Expr gevm(Expr l, Expr r);

Expr gemm(Expr l, Expr r);

Expr transposeMatrix(Expr mat);

}} // namespace simit::internal
#endif
