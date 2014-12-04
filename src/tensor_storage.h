#ifndef SIMIT_TENSOR_STORAGE_H
#define SIMIT_TENSOR_STORAGE_H

#include "ir.h"

namespace simit {
namespace ir {

/// Describes the storage arrangement of a tensor.
class TensorStorage {
public:
  enum Kind {
    Undefined,

    /// The tensor is stored in dense row major order.
    DenseRowMajor,

    /// The tensor is stored in dense z (morton) order.
    // DenseZOrder

    /// The matrix is stored sparsely using the compressed sparse row layout.
    // SparseMatrixCSR,

    /// A system tensor that is never stored. Any index expressions that use
    /// this tensor must be fused with the tensor assembly.
    SystemNone,

    /// A system tensor that is split in one dimension with one slice stored on
    /// each element of the set of that dimension.
    /// For now we will assume it was split along the first dimension.
    SystemReduced,

    /// A system tensor whose contributions are stored on the target set that it
    /// was assembled from. That is, the tensor is stored prior to the map
    /// reduction, and any expression that uses the tensor must reduce it.
    SystemUnreduced
  };

  TensorStorage() : kind(Undefined) {}
  TensorStorage(Kind kind) : kind(kind) {}

  /// Retrieve the tensor storage kind.
  Kind getKind() const { return kind; }

  /// True if the tensor is stored on a system, false otherwise.
  bool isSystem() const {
    return kind==SystemNone || kind==SystemReduced || kind==SystemUnreduced;
  }

  void setSystemStorageSet(Expr systemStorageSet) {
    this->systemStorageSet = systemStorageSet;
  }

  Expr getSystemTargetSet() const {
    iassert(!isSystem() || systemStorageSet.defined())
        << "System storages require the target set be provided";
    return systemStorageSet;
  }


private:
  Kind kind;

  /// The target set that was used to assemble the system if the tensor is
  /// stored on a system, false otherwise.
  Expr systemStorageSet;
};
std::ostream &operator<<(std::ostream &os, const TensorStorage &);

typedef std::map<Var,TensorStorage> TensorStorages;

/// Retrieve a storage descriptor for each tensor used in 'func'.
TensorStorages getTensorStorages(Func func);

/// Retrieve a storage descriptor for each tensor used in 'stmt'
TensorStorages getTensorStorages(Stmt stmt);

}}

#endif
