#include "localize_temps.h"

#include "gpu_ir.h"
#include "ir_rewriter.h"
#include "ir_builder.h"

#include <set>

namespace simit {
namespace ir {

class LocalizeTempsRewriter : public IRRewriter {
public:
  LocalizeTempsRewriter(Environment& env, Storage& storage) :
      env(env), storage(storage), inKernel(false) {}

  using IRRewriter::visit;

  void visit(const GPUKernel *op) {
    iassert(!inKernel) << "Cannot handle nested GPU kernels";
    inKernel = true;
    sharding = op->sharding;
    renamedTemps.clear();

    Stmt newBody = IRRewriter::rewrite(op->body);
    for (auto& kv : renamedTemps) {
      newBody = Block::make(VarDecl::make(kv.second), newBody);
    }
    renamedTemps.clear();
    inKernel = false;

    stmt = GPUKernel::make(newBody, op->sharding, op->reads, op->writes);
  }

  void visit(const VarExpr *op) {
    if (inKernel && env.hasTemporary(op->var)) {
      Var newVar = renameVar(op->var);
      expr = VarExpr::make(newVar);
      storage.add(newVar, TensorStorage::Kind::Dense);
    }
    else {
      expr = op;
    }
  }
  void visit(const AssignStmt *op) {
    Expr value = IRRewriter::rewrite(op->value);
    Var newVar = op->var;
    if (inKernel && env.hasTemporary(op->var)) {
       Var newVar = renameVar(op->var);
    }
    stmt = AssignStmt::make(newVar, value, op->cop);
  }

private:
  Var renameVar(const Var& var) {
    if (!renamedTemps.count(var)) {
      renamedTemps[var] = Var(var.getName() + "_kernel", var.getType());
    }
    return renamedTemps[var];
  }

  Environment& env;
  Storage& storage;
  bool inKernel;
  backend::GPUSharding sharding;
  std::map<Var, Var> renamedTemps;
};

Func localizeTemps(Func func) {
  return LocalizeTempsRewriter(func.getEnvironment(),
                               func.getStorage()).rewrite(func);
}

}}  // namespace simit::ir
