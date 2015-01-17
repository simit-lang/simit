#include "kernel_rw_analysis.h"
#include "gpu_ir.h"

#include "ir_rewriter.h"

namespace simit {
namespace ir {

class KernelRWAnalysisRewriter : public IRRewriter {
public:
  using IRRewriter::visit;

  void visit(const GPUKernel *op) {
    iassert(!curKernel);
    curKernel = op;
    reads.clear();
    writes.clear();
    IRRewriter::visit(op);
    curKernel = nullptr;

    stmt = GPUKernel::make(op->body, op->sharding, reads, writes);
  }

  void visit(const VarDecl *op) {
    if (!curKernel) {
      globalBufs.insert(op->var);
    }
    IRRewriter::visit(op);
  }

  void visit(const VarExpr *op) {
    maybeRead(op->var);
    IRRewriter::visit(op);
  }
  void visit(const CallStmt *op) {
    for (Var result : op->results) {
      maybeWrite(result);
    }
    IRRewriter::visit(op);
  }
  void visit(const ForRange *op) {
    maybeRead(op->var);
    IRRewriter::visit(op);
  }
  void visit(const For *op) {
    maybeRead(op->domain.var);
    IRRewriter::visit(op);
  }

  // Writes
  void visit(const AssignStmt *op) {
    if (op->cop.kind != CompoundOperator::None) {
      maybeRead(op->var);
    }
    maybeWrite(op->var);
    IRRewriter::visit(op);
  }
  void visit(const Store *op) {
    // TODO(gkanwar): Fix this nasty casework
    if (isa<VarExpr>(op->buffer)) {
      maybeWrite(to<VarExpr>(op->buffer)->var);
    }
    else if (isa<FieldRead>(op->buffer)) {
      const FieldRead* fieldRead = to<FieldRead>(op->buffer);
      iassert(isa<VarExpr>(fieldRead->elementOrSet));
      maybeWrite(to<VarExpr>(fieldRead->elementOrSet)->var);
    }
    else {
      not_supported_yet;
    }
    IRRewriter::visit(op);
  }
  void visit(const FieldWrite *op) {
    iassert(isa<VarExpr>(op->elementOrSet));
    maybeWrite(to<VarExpr>(op->elementOrSet)->var);
    IRRewriter::visit(op);
  }

private:
  void maybeRead(Var var) {
    if (curKernel) {
      if (globalBufs.find(var) != globalBufs.end()) {
        reads.insert(var);
      }
    }
  }
  void maybeWrite(Var var) {
    if (curKernel) {
      if (globalBufs.find(var) != globalBufs.end()) {
        writes.insert(var);
      }
    }
  }

  const GPUKernel *curKernel = nullptr;
  std::set<Var> globalBufs;
  std::set<Var> reads;
  std::set<Var> writes;
};

Func kernelRWAnalysis(Func func) {
  return KernelRWAnalysisRewriter().rewrite(func);
}

}}  // namespace simit::ir
