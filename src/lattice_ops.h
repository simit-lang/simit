#ifndef SIMIT_LATTICE_OPS
#define SIMIT_LATTICE_OPS

#include "ir.h"
#include "types.h"

namespace simit {
namespace ir {

/// Compute the index in the linearized lattice set of the given set of indices
/// `indices' should run from innermost (fastest running) to outermost
/// (slowest running).
inline Expr getLatticeCoord(vector<Expr> indices, Expr latticeSet) {
  iassert(latticeSet.type().isLatticeLinkSet());

  const LatticeLinkSetType *setType = latticeSet.type().toLatticeLinkSet();
  size_t ndims = setType->dimensions;

  iassert(indices.size() == ndims);

  // index = (... ((indices[d-1]) * sizes[d-2] + indices[d-2]) * sizes[d-3] ...)
  Expr totalInd = indices.back();
  for (int i = ndims-2; i >= 0; --i) {
    Expr dimSize = IndexRead::make(latticeSet, IndexRead::LatticeDim, i);
    totalInd = totalInd * dimSize + indices[i];
  }
  return totalInd;
}

/// Compute the index in the linearized lattice set of the given set of indices
/// `indices' should run from innermost (fastest running) to outermost
/// (slowest running), and include a directional index (mu) innermost.
inline Expr getLatticeLinkCoord(vector<Expr> indices, Expr latticeSet) {
  iassert(latticeSet.type().isLatticeLinkSet());

  const LatticeLinkSetType *setType = latticeSet.type().toLatticeLinkSet();
  int ndims = setType->dimensions;

  // ndims + 1 indices define a lattice link
  iassert(static_cast<int>(indices.size()) == ndims + 1);

  // index = (... ((indices[d]) * sizes[d-1] + indices[d-1]) * sizes[d-2] ...)
  Expr totalInd = indices.back();
  for (int i = ndims-1; i >= 0; --i) {
    Expr dimSize;
    if (i-1 >= 0) {
      dimSize = IndexRead::make(latticeSet, IndexRead::LatticeDim, i-1);
    }
    else {
      dimSize = Expr(ndims);
    }
    totalInd = totalInd * dimSize + indices[i];
  }
  return totalInd;
}

/// Compute the set of lattice indices from a linearized grid coordinate
inline vector<Expr> getLatticeIndices(Expr coord, Expr latticeSet) {
  iassert(latticeSet.type().isLatticeLinkSet());

  const LatticeLinkSetType *setType = latticeSet.type().toLatticeLinkSet();
  int ndims = setType->dimensions;

  vector<Expr> indices;
  for (int i = 0; i < ndims; ++i) {
    Expr dimSize = IndexRead::make(latticeSet, IndexRead::LatticeDim, i);
    indices.push_back(coord % dimSize);
    coord = coord / dimSize;
  }
  // Lattice point defined by ndims indices
  iassert(static_cast<int>(indices.size()) == ndims);
  return indices;
}

/// Compute the set of lattice indices from a linearized link coordinate
inline vector<Expr> getLatticeLinkIndices(Expr coord, Expr latticeSet) {
  iassert(latticeSet.type().isLatticeLinkSet());

  const LatticeLinkSetType *setType = latticeSet.type().toLatticeLinkSet();
  int ndims = setType->dimensions;

  vector<Expr> indices;
  // Add directional index
  indices.push_back(coord % ndims);
  coord = coord / ndims;
  for (int i = 0; i < ndims; ++i) {
    Expr dimSize = IndexRead::make(latticeSet, IndexRead::LatticeDim, i);
    indices.push_back(coord % dimSize);
    coord = coord / dimSize;
  }
  // Lattice point defined by ndims+1 indices
  iassert(static_cast<int>(indices.size()) == ndims+1);
  return indices;
}

/// Apply lattice index offsets with appopriate modulus
inline vector<Expr> getLatticeOffsetIndices(
    vector<Expr> base, vector<Expr> offset, Expr latticeSet) {
  iassert(base.size() == offset.size());
  iassert(latticeSet.type().isLatticeLinkSet());

  const LatticeLinkSetType *setType = latticeSet.type().toLatticeLinkSet();
  int ndims = setType->dimensions;
  iassert(static_cast<int>(base.size()) == ndims);

  vector<Expr> indices;
  for (int i = 0; i < ndims; ++i) {
    Expr dimSize = IndexRead::make(latticeSet, IndexRead::LatticeDim, i);
    // TODO: Double modulus required by truncating style of mod semantics
    Expr ind = ((base[i] + offset[i]) % dimSize
                + dimSize) % dimSize;
    indices.push_back(ind);
  }

  // Lattice points defined by ndims indices
  iassert(static_cast<int>(indices.size()) == ndims);
  return indices;
}

/// Apply lattice index offsets with appopriate modulus for links
inline vector<Expr> getLatticeLinkOffsetIndices(
    vector<Expr> base, vector<Expr> offset, Expr latticeSet) {
  iassert(base.size() == offset.size());
  iassert(latticeSet.type().isLatticeLinkSet());

  const LatticeLinkSetType *setType = latticeSet.type().toLatticeLinkSet();
  int ndims = setType->dimensions;
  iassert(static_cast<int>(base.size()) == ndims+1);

  vector<Expr> indices;
  // Add directional index first
  Expr ind = (((base[0] + offset[0]) % ndims + ndims) % ndims);
  indices.push_back(ind);
  for (int i = 0; i < ndims; ++i) {
    Expr dimSize = IndexRead::make(latticeSet, IndexRead::LatticeDim, i);
    // TODO: Double modulus required by truncating style of mod semantics
    Expr ind = ((base[i+1] + offset[i+1]) % dimSize
                + dimSize) % dimSize;
    indices.push_back(ind);
  }

  // Lattice links defined by ndims+1 indices
  iassert(static_cast<int>(indices.size()) == ndims+1);
  return indices;
}

}} // namespace simit::ir

#endif // SIMIT_LATTICE_OPS
