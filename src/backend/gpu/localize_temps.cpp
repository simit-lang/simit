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
      env(env), storage(storage) {}

  using IRRewriter::visit;

  void visit(const GPUKernel *op) {
    iassert(!inKernel) << "Cannot handle nested GPU kernels";
    inKernel = true;
    sharding = op->sharding;
    renamedTemps.clear();

    Stmt newBody = IRRewriter::rewrite(op->body);
    for (auto& kv : renamedTemps) {
      Var newVar = kv.second;
      // if (isSystemTensorType(newVar.getType())) {
        // Emit system tensors into the environment as new temps
      //   env.addTemporary(newVar);
      // }
      // else {
        // Emit element tensors as VarDecls at the top of the body
        newBody = Block::make(VarDecl::make(newVar), newBody);
      // }
    }
    renamedTemps.clear();
    inKernel = false;

    stmt = GPUKernel::make(newBody, op->sharding, op->reads, op->writes);
  }

  void visit(const VarExpr *op) {
    if (inKernel && env.hasTemporary(op->var)) {
      Var newVar = renameVar(op->var);
      // if (!isSystemTensorType(newVar.getType())) {
        expr = VarExpr::make(newVar);
        storage.add(newVar, TensorStorage::Kind::Dense);
      // }
      // else {
      //   iassert(newVar.getType().isTensor());
      //   const TensorType *ttype = newVar.getType().toTensor();
      //   tassert(ttype->order() == 2)
      //       << "Cannot handle non-matrix expansion of kernel temps";
      //   tassert(sharding.xSharded &&
      //           !sharding.ySharded &&
      //           !sharding.zSharded)
      //       << "Cannot handle multiply shard dimensions";
      //   Var xVar = sharding.xVar;

      //   // The second dimension is the kernel dimension
      //   std::vector<IndexDomain> dims = ttype->getDimensions();
      //   IndexVarFactory factory;
      //   IndexVar freeVar = factory.createIndexVar(dims[0]);
      //   IndexVar fixedVar(xVar.getName(), sharding.xDomain,
      //                     new Expr(VarExpr::make(xVar)));
      //   std::vector<IndexVar> tensorIndicies = {freeVar, fixedVar};
      //   expr = IndexExpr::make(
      //       {freeVar},
      //       IndexedTensor::make(VarExpr::make(newVar), tensorIndicies));
      // }
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
      Var newVar;
      Type newType = var.getType();
      // if (!isSystemTensorType(newType)) {
        // Element tensors can be replaced with VarDecls
        // at the head of the kernel, so no need to transform them
        newVar = Var(var.getName() + "_kernel", newType);
      // }
      // else {
        // System tensors must be allocated globally, because we
        // don't know their size at compile time.
        // Expand the dimension of Var by the sharding dimension
      //   iassert(newType.isTensor())
      //       << "Cannot have non-tensor temporary";
      //   const TensorType *ttype = newType.toTensor();
      //   tassert(ttype->order() == 1)
      //       << "Cannot handle matrix temporaries duplicated across kernels";
      //   std::vector<IndexDomain> dimensions = ttype->getDimensions();
      //   tassert(sharding.xSharded &&
      //           !sharding.ySharded &&
      //           !sharding.zSharded)
      //       << "Cannot handle multi-dimensional kernels yet";
      //   dimensions.emplace_back(sharding.xDomain);

      //   newType = TensorType::make(ttype->getComponentType(), dimensions);
      //   newVar = Var(var.getName() + "_kernel", newType);
      // }
      renamedTemps[var] = newVar;
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
