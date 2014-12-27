#ifndef SIMIT_GPU_IR_H
#define SIMIT_GPU_IR_H

#include "error.h"
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

  ShardDimension maybeShardFor(const ir::For *op) {
    ir::ForDomain domain = op->domain;
    if (domain.kind == ir::ForDomain::IndexSet &&
        domain.indexSet.getKind() != ir::IndexSet::Range) {
      if (domain.indexSet == xDomain) {
        return X;
      }
      else if (domain.indexSet == yDomain) {
        return Y;
      }
      else if (domain.indexSet == zDomain) {
        return Z;
      }
      else if (!xSharded) {
        xDomain = domain.indexSet;
        xSharded = true;
        return X;
      }
      else if (!ySharded) {
        yDomain = domain.indexSet;
        ySharded = true;
        return Y;
      }
      else if (!zSharded) {
        zDomain = domain.indexSet;
        zSharded = true;
        return Z;
      }
    }
    return NONE;
  }

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
