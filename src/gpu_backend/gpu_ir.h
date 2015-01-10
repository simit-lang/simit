#ifndef SIMIT_GPU_IR_H
#define SIMIT_GPU_IR_H

#include "error.h"
#include "gpu_ir.h"
#include "ir.h"

namespace simit {
namespace internal {

/**
  TODO(jrk)
  Passes:
    - variable lifetime, role - store var annotations
      - lives past kernel
      - written within kernel
        - only reduced into
        - write access is 1:1 with thread
    - move var decls
  
  Tease apart memory management from sharding (insertion of gpukernel blocks)
  
  Do we ever allow shadowing decls? Or just decl on first assign,
  only ever allowing one version of the same var?
  
  Tests:
    - simple IR transform before/after structure validation
  
  Allow F32 mode in simit-dump, for testing.
 */

class GPUSharding {
public:
  enum ShardDimension {NONE, X, Y, Z};

  GPUSharding() {}
  ~GPUSharding() {}

  // TODO(gkanwar): Remove scoping
  void scope(ShardDimension dim) {
    switch (dim) {
      case X: {
        iassert(!inXShard);
        inXShard = true;
        break;
      }
      case Y: {
        iassert(!inYShard);
        inYShard = true;
        break;
      }
      case Z: {
        iassert(!inZShard);
        inZShard = true;
        break;
      }
      default: {
        break;
      }
    }
  }

  void unscope(ShardDimension dim) {
    switch (dim) {
      case X: {
        inXShard = false;
        break;
      }
      case Y: {
        inYShard = false;
        break;
      }
      case Z: {
        inZShard = false;
        break;
      }
      default: {
        break;
      }
    }
  }

  bool inShard() {
    return inXShard || inYShard || inZShard;
  }

  bool isSharded() {
    return xSharded || ySharded || zSharded;
  }

  int getDepth() {
    int depth = 0;
    if (inXShard) depth++;
    if (inYShard) depth++;
    if (inZShard) depth++;
    return depth;
  }

  // Choose an available dimension and shard the For loop if possible
  ShardDimension maybeShardFor(const ir::For *op);

  bool xSharded = false;
  bool ySharded = false;
  bool zSharded = false;
  bool inXShard = false;
  bool inYShard = false;
  bool inZShard = false;
  ir::IndexSet xDomain;
  ir::IndexSet yDomain;
  ir::IndexSet zDomain;
  ir::Var xVar;
  ir::Var yVar;
  ir::Var zVar;
};

std::ostream &operator<<(std::ostream &os, const GPUSharding &var);

bool operator==(const GPUSharding& sharding1, const GPUSharding& sharding2);

struct GPUProtoKernel {
  std::vector<ir::Stmt> stmts;
  GPUSharding sharding;
};

}  // namespace simit::internal


namespace ir {

struct GPUKernel : public StmtNode<GPUKernel> {
  Stmt body;
  internal::GPUSharding sharding;

  static Stmt make(Stmt body, internal::GPUSharding sharding) {
    GPUKernel *node = new GPUKernel;
    node->body = body;
    node->sharding = sharding;
    return node;
  }
};

Func shardLoops(Func func);

}}  // namespace simit::ir

#endif
