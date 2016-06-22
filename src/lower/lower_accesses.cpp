#include "lower_accesses.h"

#include <algorithm>
#include <map>

#include "ir_rewriter.h"
#include "intrinsics.h"
#include "path_expressions.h"
#include "tensor_index.h"
#include "util/util.h"
#include "var_replace_rewriter.h"

using namespace std;
using simit::util::quote;

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

    index = Add::make(load->index, index);
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

    index = Add::make(load->index, index);
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
  Environment environment;
  
  using IRRewriter::visit;

  void visit(const Func* f) {
    environment = f->getEnvironment();
    Stmt body = rewrite(f->getBody());
    func = Func(f->getName(), f->getArguments(), f->getResults(), body,
                environment);
    func.setStorage(storage);
  }

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
      tensorStorage = storage.getStorage(to<VarExpr>(tensor)->var);
    }
    else {
      // Fields are always dense row major
      tensorStorage = TensorStorage::Kind::Dense;
    }

    Expr index;
    switch (tensorStorage.getKind()) {
      case TensorStorage::Kind::Dense: {
        iassert(indices.size() > 0);
        const TensorType *type = tensor.type().toTensor();
        vector<IndexSet> outerDims = type->getOuterDimensions();
        Type blockType = type->getBlockType();
        Expr blockSize = Literal::make(1);
        if (blockType.toTensor()->getDimensions().size() > 0) {
          blockSize = createLengthComputation(
              blockType.toTensor()->getDimensions());
        }
        vector<Expr> outerSizes;
        for (const IndexSet& is : outerDims) {
          outerSizes.push_back(createLengthComputation(is));
        }

        // It simplifies the logic to generate the inner index first
        reverse(indices.begin(), indices.end());
        reverse(outerSizes.begin(), outerSizes.end());
        iassert(indices.size() == outerSizes.size())
            << "Must index completely into tensor block to generate "
            << "a flattened access: " << tensor;

        index = rewrite(indices[0]);
        Expr subSize = outerSizes[0];
        for (size_t i=1; i < indices.size(); ++i) {
          Expr idx = Mul::make(rewrite(indices[i]), subSize);
          index = Add::make(idx, index);
          subSize = Mul::make(outerSizes[i], subSize);
        }
        break;
      }
      case TensorStorage::Kind::Indexed: {
        iassert(tensor.type().isTensor());
        size_t order = tensor.type().toTensor()->order();
        tassert(order == 2)
            << "Only currently supports matrices in reduced form. "
            << quote(tensor) << " has " << indices.size() << " dimensions";
        iassert(indices.size() == 1 || indices.size() == order)
            << "Must either supply one index per dimension or a single "
            << "index (flattened)";

        iassert(isa<VarExpr>(tensor));
        const Var& var = to<VarExpr>(tensor)->var;

        if (indices.size() == 1) {
          index = rewrite(indices[0]);
        }
        else {
          Expr i = rewrite(indices[0]);
          Expr j = rewrite(indices[1]);

          const pe::PathExpression pexpr = tensorStorage.getPathExpression();
          if (!environment.hasTensorIndex(pexpr)) {
            environment.addTensorIndex(pexpr, var);
          }
          TensorIndex tensorIndex = environment.getTensorIndex(pexpr);

          Expr coords = tensorIndex.getCoordArray();
          Expr sinks  = tensorIndex.getSinkArray();

          Var locVar(INTERNAL_PREFIX("locVar"), Int);
          spill(VarDecl::make(locVar));
          spill(CallStmt::make({locVar}, intrinsics::loc(),{i,j,coords,sinks}));
          index = locVar;
        }
        break;
      }
      case TensorStorage::Kind::Diagonal:
        index = rewrite(indices[0]);
        break;
      case TensorStorage::Kind::Undefined:
        ierror;
        break;
    }

    // Multiply in inner block size
    Type blockType = tensor.type().toTensor()->getBlockType();
    Expr blockSize = Literal::make(1);
    if (blockType.toTensor()->getDimensions().size() > 0) {
      blockSize = createLengthComputation(
          blockType.toTensor()->getDimensions());
    }
    index = Mul::make(index, blockSize);
        
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
  return LowerTensorAccesses(func.getStorage()).rewrite(func);
}

Func lowerFieldAccesses(Func func) {
  class FindElementVars : public IRVisitor {
    public:
      FindElementVars(std::map<Var, Expr> &elemVars) : elemVars(elemVars) {} 
  
    private:
      using IRVisitor::visit;

      void visit(const For *op) {
        IRVisitor::visit(op); 
        if (op->domain.kind == ForDomain::IndexSet) {
          elemVars[op->var] = op->domain.indexSet.getSet();
        }
      }

      std::map<Var, Expr> &elemVars;
  };

  class LowerFieldAccesses : public IRRewriter {
    public:
      LowerFieldAccesses(const std::map<Var, Expr> &elemVars) : 
        elemVars(elemVars) {}

    private:
      using IRRewriter::visit;

      void visit(const FieldRead *op) {
        IRRewriter::visit(op);
        for (const auto &kv : elemVars) {
          if (isa<VarExpr>(op->elementOrSet) &&
              to<VarExpr>(op->elementOrSet)->var == kv.first) {
            Expr setFieldRead = FieldRead::make(kv.second, op->fieldName);
            expr = TensorRead::make(setFieldRead, {kv.first});
          }
        }
      }

      const std::map<Var, Expr> &elemVars;
  };

  std::map<Var, Expr> elemVars;
  FindElementVars elemVarsFinder(elemVars);
  func.accept(&elemVarsFinder);

  func = LowerFieldAccesses(elemVars).rewrite(func);
  for (const auto &kv : elemVars) {
    func = replaceVar(func, kv.first, Var(kv.first.getName(), Int));
  }
  return func;
}

}}
