#ifndef SIMIT_RW_ANALYSIS
#define SIMIT_RW_ANALYSIS

#include "ir_visitor.h"

namespace simit {
namespace ir {

class ReadWriteAnalysis : public IRVisitor {
public:
  ReadWriteAnalysis(std::set<Var> vars) {
    this->vars = vars;
  }

  std::set<Var> getReads() {
    return reads;
  }

  std::set<Var> getWrites() {
    return writes;
  }

  using IRVisitor::visit;

  void visit(const VarDecl *op) {
    // VarDecls should not be swapped around with anything that refers
    // to op->var.
    maybeRead(op->var);
    maybeWrite(op->var);
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
    if (vars.find(var) != vars.end()) {
      reads.insert(var);
    }
  }
  void maybeWrite(Var var) {
    if (vars.find(var) != vars.end()) {
      writes.insert(var);
    }
  }

  std::set<Var> vars;
  std::set<Var> reads;
  std::set<Var> writes;
};

}}  // namespace simit::ir

#endif
