#include "shard_gpu_loops.h"
#include "gpu_ir.h"

#include "ir_rewriter.h"
#include "ir_queries.h"

namespace simit {

static bool isShardable(const ir::For *loop) {
  return (loop->domain.kind == ir::ForDomain::IndexSet &&
          loop->domain.indexSet.getKind() != ir::IndexSet::Range);
}

namespace backend {

void GPUSharding::shardFor(const ir::For *op) {
  iassert(isShardable(op));
  // TODO(jrk) these repeats should never happen
  // iassert (op->domain.indexSet != xDomain &&
  //          op->domain.indexSet != yDomain &&
  //          op->domain.indexSet != zDomain);
  if (!xSharded) {
    xDomain = op->domain.indexSet;
    xVar = op->var;
    xSharded = true;
  }
  else if (!ySharded) {
    yDomain = op->domain.indexSet;
    yVar = op->var;
    ySharded = true;
  }
  else {
    iassert(!zSharded);
    zDomain = op->domain.indexSet;
    zVar = op->var;
    zSharded = true;
  }
}

}  // namespace simit::backend

namespace ir {

class ShardLoops : public IRRewriter {
public:
  ShardLoops() : currentKernelSharding(nullptr) {}

private:
  using IRRewriter::visit;

  Stmt innerFor;
  vector<Var> lowerVars;
  int level = 0; // how many GPU dimensions wrap the current stmt
  int filledLevel = 0; // how many GPU dimensions are allocated

  backend::GPUSharding *currentKernelSharding;

  std::function<bool(const For*)> shardablePred = [&](const For* loop) {
    return isShardable(loop) && (!currentKernelSharding ||
                                 !currentKernelSharding->shardFull());
  };

  void visit(const For *loop) {
    level++;
    // Leave this as a loop if it's not a shardable loop, or if our kernel is
    // already full of dimensions.
    if (!shardablePred(loop) || level <= filledLevel) {
      IRRewriter::visit(loop);
      level--;
      return;
    }

    bool ownsKernel = false;
    backend::GPUSharding _sharding;
    
    // TODO(jrk) we may never have multiple nested kernel loops, at all, in our input - simplify away entirely?
    // for now:
    // iassert(!currentKernelSharding);
    
    // if we're the first loop to be sharded
    if (!currentKernelSharding) {
      ownsKernel = true;
      currentKernelSharding = &_sharding;
    }

    // TODO(jrk) enforce that no logic comes between kernel loops?
    currentKernelSharding->shardFor(loop);
    iassert(filledLevel == level-1);
    filledLevel = level;

    // Split apart body into before and after for shardable loops
    std::vector<Stmt> split = splitOnPredicate(loop->body, shardablePred);
    iassert(split.size() == 3);
    Stmt bodyFirst, bodyRest;
    bodyFirst = split[1];
    bodyRest = split[2];

    std::cout << "bodyFirst: " << bodyFirst << std::endl;
    std::cout << "bodyRest: " << bodyRest << std::endl;

    // Properly rewrite inner body, including sharding inner loops
    rewrite(loop->body);

    // For any loop vars of lower for loops, we should only execute this body
    // once. We choose the convention of all lower loop vars = 0.
    if (bodyFirst.defined()) {
      if (lowerVars.size() > 0) {
        Expr cond = Eq::make(lowerVars[0], 0);
        for (int i = 1; i < lowerVars.size(); ++i) {
          cond = And::make(cond, Eq::make(lowerVars[i], 0));
        }
        innerFor = Block::make(IfThenElse::make(cond, bodyFirst), innerFor);
      }
      else {
        innerFor = Block::make(bodyFirst, innerFor);
      }
    }
    if (bodyRest.defined()) {
      if (lowerVars.size() > 0) {
        Expr cond = Eq::make(lowerVars[0], 0);
        for (int i = 1; i < lowerVars.size(); ++i) {
          cond = And::make(cond, Eq::make(lowerVars[i], 0));
        }
        innerFor = Block::make(innerFor, IfThenElse::make(cond, bodyRest));
      }
      else {
        innerFor = Block::make(innerFor, bodyRest);
      }
    }
    std::cout << "For loop: " << Stmt(loop) << std::endl;
    std::cout << "Built inner for: " << innerFor << std::endl;

    lowerVars.push_back(loop->var);

    // The kernel owner emits the GPUKernel node around the body
    if (ownsKernel) {
      stmt = GPUKernel::make(innerFor, *currentKernelSharding);
      currentKernelSharding = nullptr;
      lowerVars.clear();
      innerFor = Stmt();
    }
    else {
      // Loops that do not own the kernel get snipped and placed in
      // innerFor. This ensures that statements outside loops are correctly
      // guarded to only run on thread 0 for the relevant dimensions.
      stmt = Pass::make();
    }
    level--;
  }
};

Func shardLoops(Func func) {
  return ShardLoops().rewrite(func);
}

}}
