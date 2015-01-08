#include "lower.h"

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
  iassert(dimensions.size() > 0);
  Expr len = createLengthComputation(dimensions[0]);
  for (size_t i=1; i < dimensions.size(); ++i) {
    len = Mul::make(len, createLengthComputation(dimensions[i]));
  }
  return len;
}

static Expr createLoadExpr(Expr tensor, Expr index) {
  // If the tensor is a load then we had a nested tensor read. Since we can't
  // have nested loads we must flatten them.
  if (isa<Load>(tensor)) {
    const Load *load = to<Load>(tensor);
    iassert(load->buffer.type().isTensor());

    Type blockType = load->buffer.type().toTensor()->blockType();
    Expr len  = createLengthComputation(blockType.toTensor()->dimensions);

    index = Add::make(Mul::make(load->index, len), index);
    return Load::make(load->buffer, index);
  }
  else {
    return Load::make(tensor, index);
  }
}

static Stmt createStoreStmt(Expr tensor, Expr index, Expr value,
                            CompoundOperator cop) {
  // If the tensor is a load then we had a nested tensor read. Since we can't
  // have nested loads we must flatten them.
  if (isa<Load>(tensor)) {
    const Load *load = to<Load>(tensor);
    iassert(load->buffer.type().isTensor());

    Type blockType = load->buffer.type().toTensor()->blockType();
    Expr len  = createLengthComputation(blockType.toTensor()->dimensions);

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

  Expr flattenIndices(Expr tensor, std::vector<Expr> indices) {
    // TODO: Generalize to n-order tensors and remove assert (also there's no
    //       need to have specialized code for vectors and matrices).
    iassert(indices.size() <= 2);

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
        if (indices.size() == 1) {
          index = rewrite(indices[0]);
        }
        else if (indices.size() == 2) {
          const TensorType *type = tensor.type().toTensor();
          Expr i = rewrite(indices[0]);
          Expr j = rewrite(indices[1]);

          IndexDomain dim1 = type->dimensions[1];
          Expr d1;
          if (dim1.getIndexSets().size() == 1 &&
              dim1.getIndexSets()[0].getKind() == IndexSet::Range) {
            iassert(dim1.getSize() < (size_t)(-1));
            int dimSize = static_cast<int>(dim1.getSize());
            d1 = Literal::make(i.type(), &dimSize);
          }
          else {
            not_supported_yet;
          }
          iassert(d1.defined());
          index = Add::make(Mul::make(i, d1), j);
        }
        else {
          not_supported_yet;
        }
        break;
      }
      case TensorStorage::SystemReduced: {
        iassert(indices.size() == 1 || indices.size() == 2);

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
