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

  Do we ever allow shadowing decls? Or just decl on first assign,
  only ever allowing one version of the same var?
  
  Allow F32 mode in simit-dump, for testing.
 */

class GPUSharding {
public:
  GPUSharding() {}

  bool isSharded() {
    iassert((xSharded || !ySharded)
         && (xSharded || !zSharded)
         && (ySharded || !zSharded));
    return xSharded;
  }

  bool shardFull() {
    return xSharded && ySharded && zSharded;
  }

  // Choose an available dimension and shard the For loop if possible
  void shardFor(const ir::For *op);

  bool xSharded = false;
  bool ySharded = false;
  bool zSharded = false;
  ir::IndexSet xDomain;
  ir::IndexSet yDomain;
  ir::IndexSet zDomain;
  ir::Var xVar;
  ir::Var yVar;
  ir::Var zVar;
};

std::ostream &operator<<(std::ostream &os, const GPUSharding &var);

bool operator==(const GPUSharding& sharding1, const GPUSharding& sharding2);

}  // namespace simit::internal


namespace ir {

struct GPUKernel : public StmtNode<GPUKernel> {
  Stmt body;
  internal::GPUSharding sharding;
  std::set<Var> reads;
  std::set<Var> writes;

  static Stmt make(Stmt body, internal::GPUSharding sharding,
                   std::set<Var> reads = std::set<Var>(),
                   std::set<Var> writes = std::set<Var>()) {
    GPUKernel *node = new GPUKernel;
    node->body = body;
    node->sharding = sharding;
    node->reads = reads;
    node->writes = writes;
    return node;
  }
};

}}  // namespace simit::ir

#endif
