#include "temps.h"

#include <string>

#include "ir.h"
#include "ir_rewriter.h"
#include "ir_transforms.h"

namespace simit {
namespace ir {

class GetFieldRead : public IRVisitor {
public:
  GetFieldRead(Expr elementOrSet, std::string fieldName)
      : elementOrSet(elementOrSet), fieldName(fieldName) {}

  Expr check(Expr expr) {
    expr.accept(this);
    return fieldRead;
  }

private:
  Expr elementOrSet;
  std::string fieldName;

  Expr fieldRead;
  
  using IRVisitor::visit;

  void visit(const FieldRead *op) {
    if (op->elementOrSet == elementOrSet && op->fieldName == fieldName) {
      fieldRead = op;
    }
    else {
      IRVisitor::visit(op);
    }
  }
};


class IsFieldReduced : public IRVisitor {
public:
  IsFieldReduced(Expr fieldRead) : fieldRead(fieldRead) {}

  bool check (Expr expr) {
    expr.accept(this);
    return fieldReductionFound;
  }

private:
  Expr fieldRead;
  bool fieldReductionFound = false;
  
  using IRVisitor::visit;

  void visit(const IndexedTensor *op) {
    if (op->tensor == fieldRead) {
      for (auto &iv : op->indexVars) {
        if (iv.isReductionVar()) {
          fieldReductionFound = true;
          return;
        }
      }
    }
  }
};

class InsertTemporaries : public IRRewriter {
  int id=0;
  
  using IRRewriter::visit;

  void visit(const FieldWrite *op) {
    Expr elemOrSet = op->elementOrSet;
    std::string fieldName = op->fieldName;

    // If the same field is read and written in the same statement and the
    // values are combined/reduced (e.g. multiplied) then we must introduce a
    // temporary to avoid read/write interference.
    Expr fieldRead = GetFieldRead(elemOrSet, fieldName).check(op->value);
    if (!fieldRead.defined()) {
      stmt = op;
      return;
    }

    bool valsCombined = IsFieldReduced(fieldRead).check(op->value);
    if (!valsCombined) {
      stmt = op;
      return;
    }

    Type fieldType = getFieldType(elemOrSet, fieldName);

    Var tmp("tmp" + std::to_string(id++), fieldType);

    Stmt tmpAssignment = AssignStmt::make(tmp, op->value);
    Stmt writeTmpToField = FieldWrite::make(elemOrSet, fieldName, tmp);
    stmt = Block::make(tmpAssignment, writeTmpToField);
  }
};

Func insertTemporaries(Func func) {
  func = InsertTemporaries().rewrite(func);
  func = insertVarDecls(func);
  return func;
}

}}
