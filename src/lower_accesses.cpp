#include "lower.h"

#include "ir_rewriter.h"

using namespace std;

namespace simit {
namespace ir {

// lowerTensorAccesses
Expr createLengthComputation(const IndexSet &indexSet) {
  return Length::make(indexSet);
}

Expr createLengthComputation(const IndexDomain &dimensions) {
  iassert(dimensions.getIndexSets().size() > 0);
  const vector<IndexSet> &indexSets = dimensions.getIndexSets();
  Expr len = createLengthComputation(indexSets[0]);
  for (size_t i=1; i < indexSets.size(); ++i) {
    len = Mul::make(len, createLengthComputation(indexSets[i]));
  }
  return len;
}

Expr createLengthComputation(const vector<IndexDomain> &dimensions) {
  iassert(dimensions.size() > 0);
  Expr len = createLengthComputation(dimensions[0]);
  for (size_t i=1; i < dimensions.size(); ++i) {
    len = Mul::make(len, createLengthComputation(dimensions[i]));
  }
  return len;
}

Expr createLoadExpr(Expr tensor, Expr index) {
  // If the tensor is a load then we hada  nested tensor read. Since we can't
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

Stmt createStoreStmt(Expr tensor, Expr index, Expr value) {
  // If the tensor is a load then we hada  nested tensor read. Since we can't
  // have nested loads we must flatten them.
  if (isa<Load>(tensor)) {
    const Load *load = to<Load>(tensor);
    iassert(load->buffer.type().isTensor());

    Type blockType = load->buffer.type().toTensor()->blockType();
    Expr len  = createLengthComputation(blockType.toTensor()->dimensions);

    index = Add::make(Mul::make(load->index, len), index);
    return Store::make(load->buffer, index, value);
  }
  else {
    return Store::make(tensor, index, value);
  }
}

class LowerTensorAccesses : public IRRewriter {

  void visit(const TensorRead *op) {
    iassert(op->type.isTensor() && op->tensor.type().toTensor());

    const TensorType *type = op->tensor.type().toTensor();

    // TODO: Generalize to n-order tensors and remove assert (also there's no
    //       need to have specialized code for vectors and matrices).
    iassert(op->indices.size() <= 2);

    Expr tensor = rewrite(op->tensor);
    Expr index;
    if (op->indices.size() == 1) {
      index = rewrite(op->indices[0]);
    }
    else if (op->indices.size() == 2) {
      // TODO: Clearly we need something more sophisticated here (for sparse
      // tensors or nested dense tensors).  For example, a tensor type could
      // carry a 'TensorStorage' object and we could ask this TensorStorage to
      // give us an Expr that computes an i,j location, or an Expr that gives us
      // a row/column.
      Expr i = rewrite(op->indices[0]);
      Expr j = rewrite(op->indices[1]);

      IndexDomain dim1 = type->dimensions[1];
      Expr d1;
      if (dim1.getIndexSets().size() == 1 &&
          dim1.getIndexSets()[0].getKind() == IndexSet::Range) {
        // TODO: Add support for unsigned ScalarTypes
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
    iassert(index.defined());

    expr = createLoadExpr(tensor, index);
  }

  void visit(const TensorWrite *op) {
    iassert(op->tensor.type().isTensor());

    const TensorType *type = op->tensor.type().toTensor();

    // TODO: Generalize to n-order tensors and remove assert (also there's no
    //       need to have specialized code for vectors and matrices).
    iassert(op->indices.size() <= 2);

    Expr tensor = rewrite(op->tensor);
    Expr value = rewrite(op->value);

    Expr index;
    if (op->indices.size() == 1) {
      index = rewrite(op->indices[0]);
    }
    else if (op->indices.size() == 2) {
      // TODO: Clearly we need something more sophisticated here (for sparse
      // tensors or nested dense tensors).  For example, a tensor type could
      // carry a 'TensorStorage' object and we could ask this TensorStorage to
      // give us an Expr that computes an i,j location, or an Expr that gives us
      // a row/column.
      Expr i = rewrite(op->indices[0]);
      Expr j = rewrite(op->indices[1]);

      IndexDomain dim1 = type->dimensions[1];
      Expr d1;
      if (dim1.getIndexSets().size() == 1 &&
          dim1.getIndexSets()[0].getKind() == IndexSet::Range) {
        // TODO: Add support for unsigned ScalarTypes
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
    iassert(index.defined());

    stmt = createStoreStmt(tensor, index, value);
  }
};

Stmt lowerTensorAccesses(Stmt stmt) {
  return LowerTensorAccesses().rewrite(stmt);
}

Func lowerTensorAccesses(Func func) {
  Stmt body = lowerTensorAccesses(func.getBody());
  return Func(func, body);
}

}}