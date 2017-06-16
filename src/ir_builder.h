#ifndef SIMIT_IR_BUILDER_H
#define SIMIT_IR_BUILDER_H

#include <memory>

#include "ir.h"
#include "error.h"
#include "macros.h"
#include "util/name_generator.h"

namespace simit {
namespace ir {

// TODO: Change error handling to use LocationDiagnostics, which is retrieved
//       from Diagnostics using Diagnostics(location), e.g.
//       auto negExpr = unaryElwiseExpr(IndexExpr::NEG, expr, diagnostics(@2));

/// A factory for creating index variables with unique names.
class IndexVarFactory {
public:
  IndexVarFactory() : nameID(0) {}

  IndexVar createIndexVar(const IndexDomain &domain);
  IndexVar createIndexVar(const IndexDomain &domain, ReductionOperator rop);

private:
  int nameID;
  std::string makeName();
};

/// Builds Simit IR nodes with unique names.
class IRBuilder {
public:
  enum UnaryOperator { Copy, Neg };
  enum BinaryOperator { Add, Sub, Mul, Div };

  void setInsertionPoint(std::vector<ir::Stmt> *stmts) {this->stmts = stmts;}

  Expr unaryElwiseExpr(UnaryOperator op, Expr e);

  /// Apply the operator element-wise to the l and r.  Generally this requires
  /// the types of each operand to be identical, but as a special case iff there
  /// are two operands an one of them is a scalar, then the operator is applied
  /// to the combination of that scalar and each element in the other operand.
  Expr binaryElwiseExpr(Expr l, BinaryOperator op, Expr r);

  Expr innerProduct(Expr l, Expr r);

  Expr outerProduct(Expr l, Expr r);

  Expr gemv(Expr l, Expr r);

  Expr gevm(Expr l, Expr r);

  Expr gemm(Expr l, Expr r);
  
  Expr transposedMatrix(Expr mat);

  /// Create a temporary variable
  Var temporary(Type type, std::string name=INTERNAL_PREFIX("tmp"));

  Expr unaryTensorElwiseExpr(UnaryOperator op, Expr e);

  Expr binaryTensorElwiseExpr(Expr l, BinaryOperator op, Expr r);

private:
  util::NameGenerator names;
  std::vector<ir::Stmt> *stmts = nullptr;
  IndexVarFactory factory;

  void addStmt(Stmt stmt)
    {simit_iassert(stmts != nullptr); stmts->push_back(stmt);}
};

}} // namespace simit::internal
#endif
