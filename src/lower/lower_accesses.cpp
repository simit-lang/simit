#include "lower_accesses.h"

#include <algorithm>

#include "ir_rewriter.h"

using namespace std;

namespace simit {
namespace ir {

// lowerTensorAccesses
static Expr createLengthComputation(const IndexSet &indexSet) {
  return Length::make(indexSet);
}

static Expr createLengthComputation(const IndexDomain &dimensions) {
  iassert(dimensions.getIndexSets().size() > 0);
  const vector<IndexSet> &indexSets = dimensions.getIndexSets();
  Expr len = createLengthComputation(indexSets[0]);
  for (size_t i=1; i < indexSets.size(); ++i) {
    len = Mul::make(len, createLengthComputation(indexSets[i]));
  }
  return len;
}

static Expr createLengthComputation(const vector<IndexDomain> &dimensions) {
  iassert(dimensions.size()>0)
      << "attempting to compute the length of a scalar";
  Expr len = createLengthComputation(dimensions[0]);
  for (size_t i=1; i < dimensions.size(); ++i) {
    len = Mul::make(len, createLengthComputation(dimensions[i]));
  }
  return len;
}

static Expr createLoadExpr(Expr tensor, Expr index) {
  iassert(tensor.type().isTensor())
      << "attempting to load from a non-tensor:" << tensor;

  // If the tensor is a load then we had a nested tensor read. Since we can't
  // have nested loads we must flatten them.
  if (isa<Load>(tensor)) {
    const Load *load = to<Load>(tensor);
    iassert(load->buffer.type().isTensor());

    Type blockType = load->buffer.type().toTensor()->getBlockType();
    Expr len  = createLengthComputation(blockType.toTensor()->getDimensions());

    index = Add::make(Mul::make(load->index, len), index);
    return Load::make(load->buffer, index);
  }
  else {
    return Load::make(tensor, index);
  }
}

static Stmt createStoreStmt(Expr tensor, Expr index, Expr value,
                            CompoundOperator cop) {
  iassert(tensor.type().isTensor())
      << "attempting to store to a non-tensor:" << tensor;

  // If the tensor is a load then we had a nested tensor read. Since we can't
  // have nested loads we must flatten them.
  if (isa<Load>(tensor)) {
    const Load *load = to<Load>(tensor);
    iassert(load->buffer.type().isTensor());

    Type blockType = load->buffer.type().toTensor()->getBlockType();
    Expr len  = createLengthComputation(blockType.toTensor()->getDimensions());

    index = Add::make(Mul::make(load->index, len), index);
    return Store::make(load->buffer, index, value, cop);
  }
  else {
    return Store::make(tensor, index, value, cop);
  }
}

class LowerTensorAccesses : public IRRewriter {
public:
  LowerTensorAccesses(const Storage &storage) : storage(storage) {}

private:
  Storage storage;
  
  using IRRewriter::visit;

  static bool canComputeSize(const IndexDomain &dom) {
    for (auto &is : dom.getIndexSets()) {
      if (is.getKind() != IndexSet::Range) {
        return false;
      }
    }
    return true;
  }

  static int getDimSize(int i, const vector<IndexDomain> &dimensions) {
    tassert(canComputeSize(dimensions[i]))
        << "only currently support dense tensors with static size";
    int dimsize = dimensions[i].getSize();
    return dimsize;
  }

  Expr flattenIndices(Expr tensor, std::vector<Expr> indices) {
    iassert(tensor.type().isTensor());

    TensorStorage tensorStorage;
    if (isa<VarExpr>(tensor)) {
      tensorStorage = storage.get(to<VarExpr>(tensor)->var);
    }
    else {
      // Fields are always dense row major
      tensorStorage = TensorStorage::DenseRowMajor;
    }

    Expr index;
    switch (tensorStorage.getKind()) {
      case TensorStorage::DenseRowMajor: {
        iassert(indices.size() > 0);
        const TensorType *type = tensor.type().toTensor();
        vector<IndexDomain> dimensions = type->getDimensions();

        // It simplifies the logic to generate the inner index first
        reverse(indices.begin(), indices.end());

        index = rewrite(indices[0]);

        for (size_t i=1; i < indices.size(); ++i) {
          Expr stride = getDimSize(indices.size()-i, dimensions);
          for (size_t j=i-1; j > 0; --j) {
            stride = Mul::make(getDimSize(indices.size()-j,dimensions), stride);
          }
          Expr idx = Mul::make(rewrite(indices[i]), stride);
          index = Add::make(idx, index);
        }
        break;
      }
      case TensorStorage::SystemReduced: {
        iassert(tensor.type().isTensor());
        size_t order = tensor.type().toTensor()->order();
        tassert(order == 2)
            << "only currently supports matrices in reduced form."
            << tensor << "has" << indices.size() << "dimensions";
        iassert(indices.size() == 1 || indices.size() == order)
            << "must either supply one index per dimension"
            << "or a single index (flattened)";

        if (indices.size() == 1) {
          index = rewrite(indices[0]);
        }
        else {
          Expr i = rewrite(indices[0]);
          Expr j = rewrite(indices[1]);

          Expr edgeSet = tensorStorage.getSystemTargetSet();
          Expr nbrs_start = IndexRead::make(edgeSet, IndexRead::NeighborsStart);
          Expr nbrs = IndexRead::make(edgeSet, IndexRead::Neighbors);

          index = Call::make(Intrinsics::loc, {i, j, nbrs_start, nbrs});
        }
        break;
      }
      case TensorStorage::SystemDiagonal:
        index = rewrite(indices[0]);
        break;
      case TensorStorage::SystemNone:
        ierror << "Can't store to a tensor without storage.";
        break;
      case TensorStorage::Undefined:
        ierror;
        break;
    }
    iassert(index.defined());
    return index;
  }

  void visit(const TensorRead *op) {
    iassert(op->type.isTensor() && op->tensor.type().toTensor());
    Expr tensor = rewrite(op->tensor);
    Expr index = flattenIndices(op->tensor, op->indices);
    expr = createLoadExpr(tensor, index);
  }

  void visit(const TensorWrite *op) {
    iassert(op->tensor.type().isTensor());
    Expr tensor = rewrite(op->tensor);
    Expr value = rewrite(op->value);
    Expr index = flattenIndices(op->tensor, op->indices);
    stmt = createStoreStmt(tensor, index, value, op->cop);
  }
};

Func lowerTensorAccesses(Func func) {
  Stmt body = LowerTensorAccesses(func.getStorage()).rewrite(func.getBody());
  return Func(func, body);
}

}}
