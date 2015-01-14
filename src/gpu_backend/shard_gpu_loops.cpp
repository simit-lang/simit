#include "shard_gpu_loops.h"
#include "gpu_ir.h"

#include "ir_rewriter.h"
#include "scopedmap.h"

namespace simit {

static bool isShardable(const ir::For *loop) {
  return (loop->domain.kind == ir::ForDomain::IndexSet &&
          loop->domain.indexSet.getKind() != ir::IndexSet::Range);
}

namespace internal {

void GPUSharding::shardFor(const ir::For *op) {
  iassert(isShardable(op));
  // TODO(jrk) these repeats should never happen
  iassert (op->domain.indexSet != xDomain && op->domain.indexSet != yDomain && op->domain.indexSet != zDomain);
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

}  // namespace simit::internal

namespace ir {

class ShardLoops : public IRRewriter {
public:
  ShardLoops() : currentKernelSharding(nullptr) {}

private:
  using IRRewriter::visit;

  void visit(const For *loop) {
    // Leave this as a loop if it's not a shardable loop, or if our kernel is
    // already full of dimensions.
    if (!isShardable(loop) ||
        (currentKernelSharding && currentKernelSharding->shardFull())) {
      IRRewriter::visit(loop);
      return;
    }
    
    bool ownsKernel = false;
    internal::GPUSharding _sharding;
    
    // TODO(jrk) we may never have multiple nested kernel loops, at all, in our input - simplify away entirely?
    // for now:
    iassert(!currentKernelSharding);
    
    // if we're the first loop to en
    if (!currentKernelSharding) {
      ownsKernel = true;
      currentKernelSharding = &_sharding;
    }

    // TODO(jrk) enforce that no logic comes between kernel loops?
    currentKernelSharding->shardFor(loop);
    stmt = rewrite(loop->body);

    if (ownsKernel) {
      stmt = GPUKernel::make(stmt, *currentKernelSharding);
      currentKernelSharding = nullptr;
    }
  }

  internal::GPUSharding *currentKernelSharding;
};

Func shardLoops(Func func) {
  return ShardLoops().rewrite(func);
}

}}  // namespace simit::ir
