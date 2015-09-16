#include "lower_maps.h"

#include "storage.h"
#include "ir_builder.h"
#include "ir_rewriter.h"
#include "ir_transforms.h"
#include "inline.h"

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

  virtual void visit(const TensorWrite *op) {
    // Rewrites the tensor write and assigns the result to stmt
    IRRewriter::visit(op);
    iassert(isa<TensorWrite>(stmt));
    if (isa<VarExpr>(op->tensor) && isResult(to<VarExpr>(op->tensor)->var)) {
      const TensorWrite *tensorWrite = to<TensorWrite>(stmt);
      iassert(tensorWrite->value.type().isTensor());

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
        default: not_supported_yet;
      }
    }
  }
};

class LowerMaps : public IRRewriter {
public:
  LowerMaps(Storage *storage) : storage(storage) {}

private:
  Storage *storage;
  
  using IRRewriter::visit;

  void visit(const Map *op) {
    iassert(hasStorage(op->vars, *storage))
        << "Every assembled tensor should have a storage descriptor";

    LowerMapFunctionRewriter mapFunctionRewriter;
    stmt = inlineMap(op, mapFunctionRewriter);

    // Add comment
    stmt = Comment::make(util::toString(*op), stmt, true);

    // Add storage descriptor for the new tensors in the inlined map
    updateStorage(stmt, storage);
  }
};

Func lowerMaps(Func func) {
  Stmt body = LowerMaps(&func.getStorage()).rewrite(func.getBody());
  func = Func(func, body);
  func = insertVarDecls(func);
  return func;
}

}}
