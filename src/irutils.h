#ifndef SIMIT_LA_H
#define SIMIT_LA_H

#include <memory>

#include "ir.h"
#include "location.hh"
#include "errors.h"

namespace simit {
namespace internal {

// TODO: Change error handling to use LocationDiagnostics, which is retrieved
//       from Diagnostics using Diagnostics(location), e.g.
//       auto negExpr = unaryElwiseExpr(IndexExpr::NEG, expr, diagnostics(@2));

/// A factory for creating index variables with unique names.
class IndexVarFactory {
public:
  IndexVarFactory() : nameID(0) {}

  std::shared_ptr<IndexVar> makeFreeVar(const IndexSetProduct &indexSet);
  std::shared_ptr<IndexVar> makeReductionVar(const IndexSetProduct &indexSet,
                                             IndexVar::Operator op);
private:
  int nameID;
  std::string makeName();
};

IndexExpr *unaryElwiseExpr(IndexExpr::Operator op,
                           const std::shared_ptr<Expression> &expr);

/// Apply the operator element-wise to the l and r.  Generally this requires
/// the types of each operand to be identical, but as a special case iff there
/// are two operands an one of them is a scalar, then the operator is applied
/// to the combinatio of that scalar and each element in the other operand.
IndexExpr *binaryElwiseExpr(const std::shared_ptr<Expression> &l,
                            IndexExpr::Operator op,
                            const std::shared_ptr<Expression> &r);

IndexExpr *elwiseExpr(IndexExpr::Operator op,
                      std::vector<std::shared_ptr<Expression>> &operands);

IndexExpr *innerProduct(const std::shared_ptr<Expression> &l,
                        const std::shared_ptr<Expression> &r);

IndexExpr *outerProduct(const std::shared_ptr<Expression> &l,
                        const std::shared_ptr<Expression> &r);

IndexExpr *gemv(const std::shared_ptr<Expression> &l,
                const std::shared_ptr<Expression> &r);

IndexExpr *gevm(const std::shared_ptr<Expression> &l,
                const std::shared_ptr<Expression> &r);

IndexExpr *gemm(const std::shared_ptr<Expression> &l,
                const std::shared_ptr<Expression> &r);

IndexExpr *transposeMatrix(const std::shared_ptr<Expression> &mat);

}} // namespace simit::internal
#endif
