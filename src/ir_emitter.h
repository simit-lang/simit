#ifndef SIMIT_IR_EMITTER_H
#define SIMIT_IR_EMITTER_H

#include <vector>

#include "hir.h"
#include "hir_visitor.h"
#include "types.h"
#include "domain.h"
#include "program_context.h"
#include "error.h"
#include "ir.h"

namespace simit {
namespace hir {

class IREmitter : public HIRVisitor {
public:
  IREmitter(internal::ProgramContext *ctx) : 
    retField(ir::Field("", ir::Type())), ctx(ctx) {}

  inline void emitIR(Program::Ptr program) {
    program->accept(this);
  }

private:
  virtual void visit(StmtBlock::Ptr);
  virtual void visit(RangeIndexSet::Ptr);
  virtual void visit(SetIndexSet::Ptr);
  virtual void visit(DynamicIndexSet::Ptr);
  virtual void visit(ElementType::Ptr);
  virtual void visit(Endpoint::Ptr);
  virtual void visit(SetType::Ptr);
  virtual void visit(TupleType::Ptr);
  virtual void visit(ScalarType::Ptr);
  virtual void visit(NDTensorType::Ptr);
  virtual void visit(IdentDecl::Ptr);
  virtual void visit(ElementTypeDecl::Ptr);
  virtual void visit(ExternDecl::Ptr);
  virtual void visit(FuncDecl::Ptr);
  virtual void visit(ProcDecl::Ptr);
  virtual void visit(VarDecl::Ptr);
  virtual void visit(ConstDecl::Ptr);
  virtual void visit(WhileStmt::Ptr);
  virtual void visit(DoWhileStmt::Ptr);
  virtual void visit(IfStmt::Ptr);
  virtual void visit(IndexSetDomain::Ptr);
  virtual void visit(RangeDomain::Ptr);
  virtual void visit(ForStmt::Ptr);
  virtual void visit(PrintStmt::Ptr);
  virtual void visit(ExprStmt::Ptr);
  virtual void visit(AssignStmt::Ptr);
  virtual void visit(ExprParam::Ptr);
  virtual void visit(MapExpr::Ptr);
  virtual void visit(OrExpr::Ptr);
  virtual void visit(AndExpr::Ptr);
  virtual void visit(XorExpr::Ptr);
  virtual void visit(EqExpr::Ptr);
  virtual void visit(NotExpr::Ptr);
  virtual void visit(AddExpr::Ptr);
  virtual void visit(SubExpr::Ptr);
  virtual void visit(MulExpr::Ptr);
  virtual void visit(DivExpr::Ptr);
  virtual void visit(ElwiseMulExpr::Ptr);
  virtual void visit(ElwiseDivExpr::Ptr);
  virtual void visit(NegExpr::Ptr);
  virtual void visit(ExpExpr::Ptr);
  virtual void visit(TransposeExpr::Ptr);
  virtual void visit(CallExpr::Ptr);
  virtual void visit(TensorReadExpr::Ptr);
  virtual void visit(TupleReadExpr::Ptr);
  virtual void visit(FieldReadExpr::Ptr);
  virtual void visit(VarExpr::Ptr);
  virtual void visit(IntLiteral::Ptr);
  virtual void visit(FloatLiteral::Ptr);
  virtual void visit(BoolLiteral::Ptr);
  virtual void visit(DenseIntVector::Ptr);
  virtual void visit(DenseFloatVector::Ptr);
  virtual void visit(DenseNDTensor::Ptr);
  virtual void visit(DenseTensorLiteral::Ptr);
  virtual void visit(Test::Ptr);

private:
  struct Domain {
    enum class Type {SET, RANGE};

    static Domain make(ir::IndexSet set) {
      Domain newRange;

      newRange.type = Type::SET;
      newRange.set = set;
      
      return newRange;
    }
    static Domain make(ir::Expr lower, ir::Expr upper) {
      Domain newRange;

      newRange.type = Type::RANGE;
      newRange.lower = lower;
      newRange.upper = upper;

      return newRange;
    }

    Type type;
    ir::IndexSet set;
    ir::Expr lower;
    ir::Expr upper;
  };
  
  struct TensorValues {
    enum class Type {UNKNOWN, INT, FLOAT};

    TensorValues() : dimSizes(1), type(Type::UNKNOWN) {};

    inline void addDimension() { dimSizes.push_back(1); }
    void addIntValues(const std::vector<int> &vals) {
      iassert(type != Type::FLOAT);
      type = Type::INT;
      intVals.insert(intVals.end(), vals.begin(), vals.end());
      dimSizes[dimSizes.size() - 1] += vals.size();
    }
    void addFloatValues(const std::vector<double> &vals) {
      iassert(type != Type::INT);
      type = Type::FLOAT;
      floatVals.insert(floatVals.end(), vals.begin(), vals.end());
      dimSizes[dimSizes.size() - 1] += vals.size();
    }
    void merge(const TensorValues &other) {
      iassert(type == other.type);
      iassert(dimSizes.size() - 1 == other.dimSizes.size());
      switch (type) {
        case Type::INT:
          intVals.insert(intVals.end(), other.intVals.begin(), 
                         other.intVals.end());
          break;
        case Type::FLOAT:
          floatVals.insert(floatVals.end(), other.floatVals.begin(), 
                           other.floatVals.end());
          break;
        default:
          unreachable;
          break;
      }
      dimSizes[dimSizes.size() - 1]++;
    }

    std::vector<unsigned> dimSizes;
    std::vector<int>       intVals;
    std::vector<double>  floatVals;
    Type                      type;
  };

  inline ir::Expr emitExpr(HIRNode::Ptr ptr) {
    retExpr = ir::Expr();
    ptr->accept(this);
    const ir::Expr ret = retExpr;
    retExpr = ir::Expr();
    return ret;
  }
  inline ir::Stmt emitStmt(Stmt::Ptr ptr) {
    retStmt = ir::Stmt();
    ptr->accept(this);
    const ir::Stmt ret = retStmt;
    retStmt = ir::Stmt();
    return ret;
  }
  inline ir::Type emitType(Type::Ptr ptr) {
    retType = ir::Type();
    ptr->accept(this);
    const ir::Type ret = retType;
    retType = ir::Type();
    return ret;
  }
  inline ir::IndexSet emitIndexSet(IndexSet::Ptr ptr) {
    retIndexSet = ir::IndexSet();
    ptr->accept(this);
    const ir::IndexSet ret = retIndexSet;
    retIndexSet = ir::IndexSet();
    return ret;
  }
  inline ir::Field emitField(Field::Ptr ptr) {
    retField = ir::Field("", ir::Type());
    ptr->accept(this);
    const ir::Field ret = retField;
    retField = ir::Field("", ir::Type());
    return ret;
  }
  inline ir::Var emitVar(IdentDecl::Ptr ptr) {
    retVar = ir::Var();
    ptr->accept(this);
    const ir::Var ret = retVar;
    retVar = ir::Var();
    return ret;
  }
  inline TensorValues emitTensorVals(DenseTensorElement::Ptr ptr) {
    retTensorVals = TensorValues();
    ptr->accept(this);
    const TensorValues ret = retTensorVals;
    retTensorVals = TensorValues();
    return ret;
  }
  inline Domain emitDomain(ForDomain::Ptr ptr) {
    retDomain = Domain();
    ptr->accept(this);
    const Domain ret = retDomain;
    retDomain = Domain();
    return ret;
  }
 
  void addFuncOrProc(FuncDecl::Ptr, const bool = false);
  void addVarOrConst(VarDecl::Ptr, const bool = false);
  void addWhileOrDoWhile(WhileStmt::Ptr, const bool = false);
  void addAssign(const std::vector<ir::Expr> &, ir::Expr);

  inline ir::Stmt getCallStmts() {
    const ir::Stmt callStmts = calls.empty() ? ir::Stmt() : 
                               ir::Block::make(calls);
    calls.clear();
    return callStmts;
  }

  std::vector<ir::Stmt> calls;

  ir::Expr retExpr;
  ir::Stmt retStmt;
  ir::Type retType;
  ir::IndexSet retIndexSet;
  ir::Field retField;
  ir::Var retVar;
  TensorValues retTensorVals;
  Domain retDomain;

  internal::ProgramContext *ctx;
};

}
}

#endif

