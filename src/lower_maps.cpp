#include "lower.h"

#include "ir_rewriter.h"
#include "storage.h"
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
  TensorStorage::Kind firstStorage = storage.get(vars[0]).getKind();
  for (auto &var : vars) {
    if (storage.get(var).getKind() != firstStorage) return false;
  }
  return true;
}


class LowerMaps : public IRRewriter {
public:
  LowerMaps(const Storage &storage) : storage(storage) {}

private:
  Storage storage;

  void visit(const Map *op) {
    iassert(hasStorage(op->vars, storage))
        << "Every assembled tensor should have a storage descriptor";
    tassert(hasSameStorage(op->vars, storage))
        << "All assembled tensors in the same Map must have the same storage.";

    TensorStorage::Kind tensorStorage = storage.get(op->vars[0]).getKind();
    if (tensorStorage != TensorStorage::SystemNone || op->vars.size() == 0) {
      if (tensorStorage == TensorStorage::SystemReduced ||
          tensorStorage == TensorStorage::DenseRowMajor) {
        stmt = inlineMap(op);
      }
      else {
        ierror << "Unsupported tensor storage lowering";
      }
    }
    else {
      stmt = op;
    }
  }
};

Func lowerMaps(Func func) {
  Stmt body = LowerMaps(func.getStorage()).rewrite(func.getBody());
  return Func(func, body);
}

}}
