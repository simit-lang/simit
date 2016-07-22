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
  using MapFunctionRewriter::visit;

  void visit(const TensorWrite *op) {
    // Rewrites the tensor write and assigns the result to stmt
    IRRewriter::visit(op);
    iassert(isa<TensorWrite>(stmt));
    const TensorWrite *tensorWrite = to<TensorWrite>(stmt);
    iassert(tensorWrite->value.type().isTensor());

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
    iassert(targetVar.defined());

    if (isResult(targetVar)) {
      Var mapVar = getMapVar(targetVar);
      if (clocs.size() > 0) {
        iassert(op->indices.size() == 2);
        // Row index must be normalized to zero
        iassert((isa<VarExpr>(op->indices[0]) &&
                 to<VarExpr>(op->indices[0])->var == target) ||
                (isa<SetRead>(op->indices[0]) && util::isAllZeros(
                    getOffsets(to<SetRead>(op->indices[0])->indices))));
        // Get col index
        iassert(throughSet.type().toSet()->kind == SetType::LatticeLink);
        int dims = throughSet.type().toSet()->dimensions;
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
          unreachable;
        }

        // Change assignments to result to compound  assignments, using the map
        // reduction operator.
        switch (reduction.getKind()) {
          case ReductionOperator::Sum: {
            stmt = TensorWrite::make(rewrite(op->tensor), {index},
                                     rewrite(op->value), CompoundOperator::Add);
            break;
          }
          case ReductionOperator::Undefined: {
            stmt = TensorWrite::make(rewrite(op->tensor), {index},
                                     rewrite(op->value));
            break;
          }
        }
      }
      else if (storage->getStorage(mapVar).getKind() ==
               TensorStorage::Kind::Indexed) {
        iassert(locs.defined());
        iassert(op->indices.size() == 2);
        iassert(endpoints.defined());
        vector<Expr> indices;
        for (auto& index : op->indices) {
          iassert(isa<TupleRead>(index)) << index;
          indices.push_back(to<TupleRead>(index)->index);
        }
        Expr index = TensorRead::make(locs, indices);

        // Change assignments to result to compound  assignments, using the map
        // reduction operator.
        switch (reduction.getKind()) {
          case ReductionOperator::Sum: {
            stmt = TensorWrite::make(rewrite(op->tensor), {index},
                                     rewrite(op->value), CompoundOperator::Add);
            break;
          }
          case ReductionOperator::Undefined: {
            stmt = TensorWrite::make(rewrite(op->tensor), {index},
                                     rewrite(op->value));
            break;
          }
        }
      }
      else {
        // Change assignments to result to compound  assignments, using the map
        // reduction operator.
        switch (reduction.getKind()) {
          case ReductionOperator::Sum: {
            stmt = TensorWrite::make(tensorWrite->tensor, tensorWrite->indices,
                                     tensorWrite->value, CompoundOperator::Add);
            break;
          }
          case ReductionOperator::Undefined: {
            stmt = TensorWrite::make(tensorWrite->tensor, tensorWrite->indices,
                                     tensorWrite->value);
            break;
          }
        }
        iassert(stmt.defined());
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
    iassert(hasStorage(op->vars, *storage))
        << "Every assembled tensor should have a storage descriptor";

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
  Stmt body = LowerMaps(&func.getStorage(),&func.getEnvironment())
      .rewrite(func.getBody());
  func = Func(func, body);
  func = insertVarDecls(func);
  return func;
}

}}
