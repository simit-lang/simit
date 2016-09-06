#include "kernel_rw_analysis.h"
#include "gpu_ir.h"

#include <algorithm>

#include "ir_rewriter.h"
#include "rw_analysis.h"

namespace simit {
namespace ir {

class FindRootVars : public IRVisitor {
public:
  std::set<Var> getRootVars() {
    return rootVars;
  }

  using IRVisitor::visit;

  void visit(const Func *op) {
    const Environment& env = op->getEnvironment();
    // Consider all environmental variables EXCEPT constants
    for (const Var& tmp : env.getTemporaries()) {
      rootVars.insert(tmp);
    }
    for (auto &ext : env.getExterns()) {
      rootVars.insert(ext.getVar());
    }
    for (Var arg : op->getArguments()) {
      rootVars.insert(arg);
    }
    for (Var res : op->getResults()) {
      rootVars.insert(res);
    }
    IRVisitor::visit(op);
  }

  void visit(const VarDecl *op) {
    if (!curKernel) {
      rootVars.insert(op->var);
    }
    IRVisitor::visit(op);
  }

  void visit(const GPUKernel *op) {
    iassert(!curKernel);
    curKernel = op;
    IRVisitor::visit(op);
    curKernel = nullptr;
  }

private:
  const GPUKernel *curKernel = nullptr;
  std::set<Var> rootVars;
};

std::set<Var> findRootVars(Func func) {
  FindRootVars findRootVarsAnalysis;
  findRootVarsAnalysis.visit(&func);
  return findRootVarsAnalysis.getRootVars();
}

class KernelRWAnalysisRewriter : public IRRewriter {
public:
  KernelRWAnalysisRewriter(std::set<Var> rootVars) {
    this->rootVars = rootVars;
  }

  using IRRewriter::visit;
  
  void visit(const GPUKernel *op) {
    ReadWriteAnalysis readWriteAnalysis(rootVars);
    readWriteAnalysis.visit(op);

    std::set<Var> reads = readWriteAnalysis.getReads();
    std::set<Var> writes = readWriteAnalysis.getWrites();
    stmt = GPUKernel::make(op->body, op->sharding, reads, writes);

    // Safety check: Any variables written must be atomically read only
    std::set<Var> nonAtomicReads = readWriteAnalysis.getNonAtomicReads();
    std::vector<Var> maybeUnsafe;
    std::set_intersection(nonAtomicReads.begin(), nonAtomicReads.end(),
                          writes.begin(), writes.end(),
                          back_inserter(maybeUnsafe));
    std::vector<Var> unsafe;
    // TODO: For now, we assume reads + writes from the same set are okay,
    // as long as it's a kernel dimension, since in this case each thread
    // only has access to its piece of the set.
    std::vector<Var> domainSets;
    iassert(op->sharding.xSharded);
    iassert(isa<VarExpr>(op->sharding.xDomain.getSet()));
    domainSets.push_back(to<VarExpr>(op->sharding.xDomain.getSet())->var);
    if (op->sharding.ySharded) {
      iassert(isa<VarExpr>(op->sharding.yDomain.getSet()));
      domainSets.push_back(to<VarExpr>(op->sharding.yDomain.getSet())->var);
    }
    if (op->sharding.zSharded) {
      iassert(isa<VarExpr>(op->sharding.zDomain.getSet()));
      domainSets.push_back(to<VarExpr>(op->sharding.zDomain.getSet())->var);
    }
    for (auto &v : maybeUnsafe) {
      if (!v.getType().isSet()) {
        unsafe.push_back(v);
      }
      else if (!util::contains(domainSets, v)) {
        unsafe.push_back(v);
      }
    }
    // TODO: This should ideally be a hard assert, but we can't perfectly
    // analyze when a read/write lives safely within the bounds of a single loop
    // iteration
#ifdef SIMIT_ASSERTS
    if (unsafe.size() == 0) {
      uwarning << "Variables both non-atomically read and written in kernel: "
               << stmt << "\n";
    }
#endif
  }

private:
  std::set<Var> rootVars;
};

Func kernelRWAnalysis(Func func) {
  std::set<Var> rootVars = findRootVars(func);
  return KernelRWAnalysisRewriter(rootVars).rewrite(func);
}

}}  // namespace simit::ir
