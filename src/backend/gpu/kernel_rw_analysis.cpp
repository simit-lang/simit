#include "kernel_rw_analysis.h"
#include "gpu_ir.h"

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
  }

private:
  std::set<Var> rootVars;
};

Func kernelRWAnalysis(Func func) {
  std::set<Var> rootVars = findRootVars(func);
  return KernelRWAnalysisRewriter(rootVars).rewrite(func);
}

}}  // namespace simit::ir
