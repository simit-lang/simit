#include "kernel_rw_analysis.h"
#include "gpu_ir.h"

#include "ir_rewriter.h"

namespace simit {
namespace ir {

class FindRootVars : public IRVisitor {
public:
  std::set<Var> getRootVars() {
    return rootVars;
  }

  using IRVisitor::visit;

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

class ReadWriteAnalysis : public IRVisitor {
public:
  ReadWriteAnalysis(std::set<Var> rootVars) {
    this->rootVars = rootVars;
  }

  std::set<Var> getReads() {
    return reads;
  }

  std::set<Var> getWrites() {
    return writes;
  }

  using IRVisitor::visit;

  void visit(const VarDecl *op) {
    IRVisitor::visit(op);
  }

  void visit(const VarExpr *op) {
    maybeRead(op->var);
    IRVisitor::visit(op);
  }
  void visit(const CallStmt *op) {
    for (Var result : op->results) {
      maybeWrite(result);
    }
    IRVisitor::visit(op);
  }
  void visit(const ForRange *op) {
    maybeRead(op->var);
    IRVisitor::visit(op);
  }
  void visit(const For *op) {
    maybeRead(op->domain.var);
    IRVisitor::visit(op);
  }

  // Writes
  void visit(const AssignStmt *op) {
    if (op->cop.kind != CompoundOperator::None) {
      maybeRead(op->var);
    }
    maybeWrite(op->var);
    IRVisitor::visit(op);
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
    IRVisitor::visit(op);
  }
  void visit(const FieldWrite *op) {
    iassert(isa<VarExpr>(op->elementOrSet));
    maybeWrite(to<VarExpr>(op->elementOrSet)->var);
    IRVisitor::visit(op);
  }

private:
  void maybeRead(Var var) {
    if (rootVars.find(var) != rootVars.end()) {
      reads.insert(var);
    }
  }
  void maybeWrite(Var var) {
    if (rootVars.find(var) != rootVars.end()) {
      writes.insert(var);
    }
  }

  std::set<Var> rootVars;
  std::set<Var> reads;
  std::set<Var> writes;
};

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
  FindRootVars findRootVars;
  findRootVars.visit(&func);
  std::set<Var> rootVars = findRootVars.getRootVars();
  return KernelRWAnalysisRewriter(rootVars).rewrite(func);
}

}}  // namespace simit::ir
