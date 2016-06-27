#ifndef SIMIT_TENSOR_INDEX_H
#define SIMIT_TENSOR_INDEX_H

#include <string>
#include <ostream>
#include <vector>
#include <memory>

namespace simit {
namespace pe {
class PathExpression;
}

namespace ir {
class Var;

/// A tensor index is a CSR index that describes the sparsity of a matrix.
/// Tensor indices may have path expressions that describe their sparsity as a
/// function of a Simit graph.  Such tensor indices are added to the
/// environemnt, pre-assembled on function initialization and shared between
/// tensors with the same sparsity.  Tensor indices without path expressions
/// cannot be pre-assembled and must be assembled by the user if they come
/// from an extern function, or by Simit as they are computed.
/// Note: only sparse matrix CSR indices are supported for now.
class TensorIndex {
public:
  TensorIndex() {}
  TensorIndex(std::string name, pe::PathExpression pexpr);

  /// Get tensor index name
  const std::string getName() const;

  /// Get the tensor index's path expression.  Tensor indices with defined path
  /// expressions are stored in the environment, pre-assembled and shared
  /// between tensors with the same sparsity.  Tensors with undefined path
  /// expressions are assembled by the user if they are returned from an extern
  /// function, or by Simit as they are computed.
  const pe::PathExpression& getPathExpression() const;

  /// Return the tensor index's rowptr array.  A rowptr array contains the
  /// beginning and end of the column indices in the colidx array for each row
  /// of the tensor index.
  /// Note: only sparse matrix CSR indices are supported for now.
  const Var& getRowptrArray() const;

  /// Return the tensor index's colidx array.  A colidx array contains the
  /// column index of every non-zero tensor value.
  /// Note: only sparse matrix CSR indices are supported for now.
  const Var& getColidxArray() const;

  /// Defined if the tensor exists, false otherwise.
  bool defined() const {return content != nullptr;}

private:
  struct Content;
  std::shared_ptr<Content> content;
};

std::ostream& operator<<(std::ostream&, const TensorIndex&);

}}

#endif
