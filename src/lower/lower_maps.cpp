#include "lower_maps.h"

#include "storage.h"
#include "ir_builder.h"
#include "ir_rewriter.h"
#include "ir_transforms.h"
#include "inline.h"
#include "path_expressions.h"
#include "tensor_index.h"
#include "util/collections.h"

using namespace std;

namespace simit {
namespace ir {

inline bool hasStorage(std::vector<Var> vars, const Storage &storage) {
  for (auto &var : vars) {
    if (!storage.hasStorage(var)) return false;
  }
  return true;
}

inline bool hasSameStorage(std::vector<Var> vars, const Storage &storage) {
  if (vars.size() == 0) return true;
  TensorStorage::Kind firstStorage = storage.getStorage(vars[0]).getKind();
  for (auto &var : vars) {
    if (storage.getStorage(var).getKind() != firstStorage) {
      return false;
    }
  }
  return true;
}

class LowerMapFunctionRewriter : public MapFunctionRewriter {

  /// Change assignments to result to compound  assignments, using the map
  /// reduction operator.
  Stmt makeCompoundTensorWrite(Expr tensor, vector<Expr> indices, Expr value) {
    switch (reduction.getKind()) {
      case ReductionOperator::Sum: {
        return TensorWrite::make(tensor, indices, value, CompoundOperator::Add);
      }
      case ReductionOperator::Undefined: {
        return TensorWrite::make(tensor, indices, value);
      }
    }
    simit_unreachable;
    return Stmt();
  }

  using MapFunctionRewriter::visit;

  void visit(const TensorWrite *op) {
    // Rewrites the tensor write and assigns the result to stmt
    IRRewriter::visit(op);
    simit_iassert(isa<TensorWrite>(stmt));
    const TensorWrite *tensorWrite = to<TensorWrite>(stmt);
    simit_iassert(tensorWrite->value.type().isTensor());

    Var targetVar;
    match(op->tensor,
      function<void(const TensorRead*,Matcher*)>([&](const TensorRead* op,
                                                     Matcher* ctx){
        ctx->match(op->tensor);
      }),
      std::function<void(const VarExpr*)>([&targetVar](const VarExpr* v) {
        targetVar = v->var;
      })
    );
    simit_iassert(targetVar.defined());

    if (isResult(targetVar)) {
      Var mapVar = getMapVar(targetVar);
      auto tensorStorage = storage->getStorage(mapVar);
      if (clocs.size() > 0) {
        simit_iassert(op->indices.size() == 2);
        // Row index must be normalized to zero
        simit_iassert((isa<VarExpr>(op->indices[0]) &&
                 to<VarExpr>(op->indices[0])->var == target) ||
                (isa<SetRead>(op->indices[0]) && util::isAllZeros(
                    getOffsets(to<SetRead>(op->indices[0])->indices))));
        // Get col index
        simit_iassert(throughSet.type().isGridSet());
        int dims = throughSet.type().toGridSet()->dimensions;
        Expr index;
        if (isa<VarExpr>(op->indices[1])) {
          vector<int> offsets(dims);
          index = clocs[offsets];
        }
        else if (isa<SetRead>(op->indices[1])) {
          vector<int> offsets = getOffsets(to<SetRead>(op->indices[1])->indices);
          index = clocs[offsets];
        }
        else {
          simit_unreachable;
        }
        stmt = makeCompoundTensorWrite(rewrite(op->tensor), {index},
                                       rewrite(op->value));
      }
      else if (tensorStorage.getKind() == TensorStorage::Indexed) {
        auto index = tensorStorage.getTensorIndex();
        simit_iassert(util::contains(locs, index));

        simit_iassert(op->indices.size() == 2);
        simit_iassert(endpoints.defined());
        vector<Expr> indices;
        for (auto& index : op->indices) {
          if (isa<UnnamedTupleRead>(index)) {
            indices.push_back(to<UnnamedTupleRead>(index)->index);
          } else if (isa<NamedTupleRead>(index)) {
            const auto tupleRead = to<NamedTupleRead>(index);
            const auto tupleType = tupleRead->tuple.type().toNamedTuple();
            const auto index = tupleType->elementIndex(tupleRead->elementName);
            indices.push_back((int)index);
          }
        }
        Expr indexExpr = TensorRead::make(locs[index], indices);
        stmt = makeCompoundTensorWrite(rewrite(op->tensor), {indexExpr},
                                       rewrite(op->value));
      }
      else {
        stmt = makeCompoundTensorWrite(tensorWrite->tensor,tensorWrite->indices,
                                       tensorWrite->value);
        simit_iassert(stmt.defined());
      }
    }
  }
};

class LowerMaps : public IRRewriter {
public:
  LowerMaps(Storage *storage, Environment *env)
      : storage(storage), env(env) {}

private:
  Storage *storage;
  Environment *env;
  
  using IRRewriter::visit;

  void visit(const Map *op) {
    simit_iassert(hasStorage(op->vars, *storage))
        << "Every assembled tensor should have a storage descriptor (" << util::join(op->vars) << ")";

    LowerMapFunctionRewriter mapFunctionRewriter;
    stmt = inlineMap(op, mapFunctionRewriter, storage);

    // Add comment
    stmt = Comment::make(util::toString(*op), stmt, true);

    // Add storage descriptor for the new tensors in the inlined map
    updateStorage(stmt, storage, env);

    // Add result variable indices to the environment
    for (auto result : op->vars) {
      auto tensorStorage = storage->getStorage(result);
      if (tensorStorage.getKind() == TensorStorage::Indexed) {
        auto& pexpr = tensorStorage.getTensorIndex().getPathExpression();
        env->addTensorIndex(pexpr, result);
      }
    }

    // Add storage from mapped Func's environment
    Func noBody(op->function, Pass::make());
    updateStorage(noBody, storage, env);

    // Add constants from inlined map function into environment
    for (auto &c : op->function.getEnvironment().getConstants()) {
      env->addConstant(c.first, c.second);
    }
  }
};

Func lowerMaps(Func func) {
  LowerMaps rewriter(&func.getStorage(),&func.getEnvironment());
  Stmt body = rewriter.rewrite(func.getBody());
  func = Func(func, body);
  func = insertVarDecls(func);
  return func;
}

}}
