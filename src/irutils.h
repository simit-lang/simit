#ifndef SIMIT_LA_H
#define SIMIT_LA_H


#include <memory>
#include "ir.h"

namespace simit {
namespace internal {

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

IndexExpr *binaryElwiseExpr(const std::shared_ptr<TensorNode> &l,
                            IndexExpr::Operator op,
                            const std::shared_ptr<TensorNode> &r);

IndexExpr *transposeMatrix(const std::shared_ptr<TensorNode> &mat);

}} // namespace simit::internal
#endif
