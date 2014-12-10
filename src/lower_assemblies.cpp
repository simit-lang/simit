#include "lower.h"

#include "ir_rewriter.h"
#include "tensor_storage.h"
#include "inline.h"

using namespace std;

namespace simit {
namespace ir {

inline bool hasStorage(std::vector<Var> vars, const TensorStorages &storages) {
  for (auto &var : vars) {
    if (storages.find(var) == storages.end()) return false;
  }
  return true;
}

inline bool hasSameStorage(std::vector<Var> vars,
                           const TensorStorages &storages) {
  if (vars.size() == 0) return true;
  TensorStorage::Kind storage = storages.at(vars[0]).getKind();
  for (auto &var : vars) {
    if (storages.at(var).getKind() != storage) return false;
  }
  return true;
}


class LowerAssemblies : public IRRewriter {
public:
  LowerAssemblies(const TensorStorages &storages)
      : storages(storages) {}

private:
  TensorStorages storages;

  void visit(const Map *op) {
    stmt = op;
    return;

    iassert(hasStorage(op->vars, storages))
        << "Every assembled tensor should have a storage descriptor";
    tassert(hasSameStorage(op->vars, storages))
        << "All assembled tensors in the same Map must have the same storage.";

    if (op->vars.size() == 0
        || storages.at(op->vars[0]).getKind() != TensorStorage::SystemNone) {
      TensorStorage::Kind storage = storages.at(op->vars[0]).getKind();

      if (storage == TensorStorage::SystemReduced) {
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

Func lowerAssemblies(Func func, const TensorStorages &tensorStorages) {
  Stmt body = LowerAssemblies(tensorStorages).rewrite(func.getBody());
  return Func(func, body);
}

}}
