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


class LowerAssemblies : public IRRewriter {
public:
  LowerAssemblies(const Storage &storage) : storage(storage) {}

private:
  Storage storage;

  void visit(const Map *op) {
    stmt = op;
    return;

    iassert(hasStorage(op->vars, storage))
        << "Every assembled tensor should have a storage descriptor";
    tassert(hasSameStorage(op->vars, storage))
        << "All assembled tensors in the same Map must have the same storage.";

    if (op->vars.size() == 0
        || storage.get(op->vars[0]).getKind() != TensorStorage::SystemNone) {
      TensorStorage::Kind tensorStorage = storage.get(op->vars[0]).getKind();

      if (tensorStorage == TensorStorage::SystemReduced) {
        Func kernel = op->function;
        Stmt body = kernel.getBody();

        Var targetVar = kernel.getArguments()[0];
        Var neighborsVar = kernel.getArguments()[1];

        Var loopVar(targetVar.getName(), Int);
        ForDomain domain(op->target);

        body = InlineMappedFunction(op,loopVar).rewrite(body);
        Stmt loweredMap = For::make(loopVar, domain, body);
        stmt = Block::make(op, loweredMap);
      }
      else {
        terror << "Unsupported tensor storage lowering";
      }
    }
    else {
      stmt = op;
    }
  }
};

Func lowerAssemblies(Func func) {
  Stmt body = LowerAssemblies(func.getStorage()).rewrite(func.getBody());
  return Func(func, body);
}

}}
