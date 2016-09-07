#ifndef SIMIT_RW_ANALYSIS
#define SIMIT_RW_ANALYSIS

#include <algorithm>

#include "ir_visitor.h"

namespace simit {
namespace ir {

class ReadWriteAnalysis : public IRVisitor {
public:
  ReadWriteAnalysis(std::set<Var> vars) {
    this->vars = vars;
  }

  std::set<Var> getReads() {
    return allReads;
  }

  std::set<Var> getAtomicReads() {
    return atomicReads;
  }

  std::set<Var> getNonAtomicReads() {
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
    // Check for system dimensions -- we need to read the set length
    if (op->var.getType().isTensor()) {
      const TensorType *ttype = op->var.getType().toTensor();
      for (const IndexDomain& dim : ttype->getDimensions()) {
        for (const IndexSet& is : dim.getIndexSets()) {
          if (is.getKind() == IndexSet::Set) {
            iassert(isa<VarExpr>(is.getSet()))
                << "Cannot understand non-Var set dimensions";
            maybeRead(to<VarExpr>(is.getSet())->var);
          }
        }
      }
    }
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
    if (op->cop != CompoundOperator::None) {
      maybeAtomicRead(op->var);
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
    op->value.accept(this);
    op->index.accept(this);
  }
  void visit(const Load *op) {
    if (isa<VarExpr>(op->buffer)) {
      maybeRead(to<VarExpr>(op->buffer)->var);
    }
    else if (isa<FieldRead>(op->buffer)) {
      const FieldRead* fieldRead = to<FieldRead>(op->buffer);
      iassert(isa<VarExpr>(fieldRead->elementOrSet));
      maybeRead(to<VarExpr>(fieldRead->elementOrSet)->var);
    }
    op->index.accept(this);
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
      allReads.insert(var);
    }
  }
  void maybeAtomicRead(Var var) {
    if (vars.find(var) != vars.end()) {
      atomicReads.insert(var);
      allReads.insert(var);
    }
  }
  void maybeWrite(Var var) {
    if (vars.find(var) != vars.end()) {
      writes.insert(var);
    }
  }

  std::set<Var> vars;
  std::set<Var> reads;
  std::set<Var> atomicReads;
  std::set<Var> allReads;
  std::set<Var> writes;
};

}}  // namespace simit::ir

#endif
