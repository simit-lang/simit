#ifndef SIMIT_GPU_IR_H
#define SIMIT_GPU_IR_H

#include "error.h"
#include "gpu_ir.h"
#include "ir.h"

namespace simit {
namespace internal {

class GPUSharding {
public:
  enum ShardDimension {NONE, X, Y, Z};

  GPUSharding() {}
  ~GPUSharding() {}

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

  // Update the tracked sharded variables to include the sharded GPUFor loop
  void addShardDomain(const ir::GPUFor *op);
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
};

}  // namespace simit::internal


namespace ir {

struct GPUFor : public StmtNode<GPUFor> {
  Var var;
  ForDomain domain;
  Stmt body;
  internal::GPUSharding::ShardDimension dimension;

  static Stmt make(Var var, ForDomain domain, Stmt body,
                   internal::GPUSharding::ShardDimension dimension) {
    GPUFor *node = new GPUFor;
    node->var = var;
    node->domain = domain;
    node->body = body;
    node->dimension = dimension;
    return node;
  }
};

Func shardLoops(Func func);

}}  // namespace simit::ir

#endif
