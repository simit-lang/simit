#include "temps.h"

#include <string>

#include "ir.h"
#include "ir_builder.h"
#include "ir_rewriter.h"
#include "ir_transforms.h"
#include "util/name_generator.h"

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
  util::NameGenerator names;

  using IRRewriter::visit;

#if 0
  Expr multiplyByOne(Expr tensor) {
    std::vector<IndexVar> indexVars;
    std::vector<IndexVar> scalarIndexVars;
    IndexVarFactory factory;
    
    const TensorType *tensorType = tensor.type().toTensor();
    std::vector<IndexDomain> dimensions = tensorType->getDimensions();
    for (unsigned int i=0; i < tensorType->order(); ++i) {
      IndexDomain domain = dimensions[i];
      indexVars.push_back(factory.createIndexVar(domain));
    }
  
    
    Expr l = IndexedTensor::make(Literal::make(1.0), scalarIndexVars);
    Expr r = IndexedTensor::make(tensor, indexVars);
    Expr val = Mul::make(l, r);
    
    return IndexExpr::make(indexVars, val);
  }
#endif

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

    Var tmp(names.getName(), fieldType);

    Stmt tmpAssignment = AssignStmt::make(tmp, op->value);
    Stmt writeTmpToField = FieldWrite::make(elemOrSet, fieldName, tmp);
    stmt = Block::make(tmpAssignment, writeTmpToField);
  }

  void visit(const Print *op) {
    if (!op->expr.defined()) {
      stmt = op;
      return;
    }

    std::stringstream oss;
    std::string exprLabel;

    oss << op->expr;
    exprLabel = oss.str() + " = \n";

    if (isa<VarExpr>(op->expr)) {
      stmt = Block::make(Print::make(exprLabel), Print::make(op->expr));
      return;
    }

    Var tmp(names.getName(), op->expr.type());
    Expr val = IRBuilder().unaryElwiseExpr(IRBuilder::None, op->expr);

    std::vector<Stmt> stmts;
    stmts.push_back(AssignStmt::make(tmp, val));
    stmts.push_back(Print::make(exprLabel));
    stmts.push_back(Print::make(tmp));

    stmt = Block::make(stmts);
  }
};

Func insertTemporaries(Func func) {
  func = InsertTemporaries().rewrite(func);
  func = insertVarDecls(func);
  return func;
}

}}
