#ifndef SIMIT_IR_EMITTER_H
#define SIMIT_IR_EMITTER_H

#include <unordered_map>
#include <vector>

#include "fir.h"
#include "fir_visitor.h"
#include "types.h"
#include "domain.h"
#include "program_context.h"
#include "error.h"
#include "ir.h"

namespace simit {
namespace fir {

// Handles translation from higher-level IR to Simit IR.
class IREmitter : public FIRVisitor {
public:
  IREmitter(internal::ProgramContext *ctx) : 
    retField(ir::Field("", ir::Type())), ctx(ctx) {}

  void emitIR(Program::Ptr program) { program->accept(this); }

private:
  virtual void visit(StmtBlock::Ptr);
  virtual void visit(RangeIndexSet::Ptr);
  virtual void visit(SetIndexSet::Ptr);
  virtual void visit(DynamicIndexSet::Ptr);
  virtual void visit(ElementType::Ptr);
  virtual void visit(Endpoint::Ptr);
  virtual void visit(UnstructuredSetType::Ptr);
  virtual void visit(HomogeneousEdgeSetType::Ptr);
  virtual void visit(HeterogeneousEdgeSetType::Ptr);
  virtual void visit(GridSetType::Ptr);
  virtual void visit(TupleElement::Ptr);
  virtual void visit(NamedTupleType::Ptr);
  virtual void visit(UnnamedTupleType::Ptr);
  virtual void visit(ScalarType::Ptr);
  virtual void visit(NDTensorType::Ptr);
  virtual void visit(OpaqueType::Ptr);
  virtual void visit(IdentDecl::Ptr);
  virtual void visit(ElementTypeDecl::Ptr);
  virtual void visit(ExternDecl::Ptr);
  virtual void visit(FuncDecl::Ptr);
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
  virtual void visit(LeftDivExpr::Ptr);
  virtual void visit(ElwiseMulExpr::Ptr);
  virtual void visit(ElwiseDivExpr::Ptr);
  virtual void visit(NegExpr::Ptr);
  virtual void visit(ExpExpr::Ptr);
  virtual void visit(TransposeExpr::Ptr);
  virtual void visit(CallExpr::Ptr);
  virtual void visit(TensorReadExpr::Ptr);
  virtual void visit(SetReadExpr::Ptr);
  virtual void visit(NamedTupleReadExpr::Ptr);
  virtual void visit(UnnamedTupleReadExpr::Ptr);
  virtual void visit(FieldReadExpr::Ptr);
  virtual void visit(VarExpr::Ptr);
  virtual void visit(IntLiteral::Ptr);
  virtual void visit(FloatLiteral::Ptr);
  virtual void visit(BoolLiteral::Ptr);
  virtual void visit(ComplexLiteral::Ptr);
  virtual void visit(StringLiteral::Ptr);
  virtual void visit(IntVectorLiteral::Ptr);
  virtual void visit(FloatVectorLiteral::Ptr);
  virtual void visit(ComplexVectorLiteral::Ptr);
  virtual void visit(NDTensorLiteral::Ptr);
  virtual void visit(ApplyStmt::Ptr);
  virtual void visit(Test::Ptr);

private:
  struct Domain {
    enum class Type {UNKNOWN, SET, RANGE};

    Domain() = default;
    Domain(ir::IndexSet set) : type(Type::SET), set(set) {}
    Domain(ir::Expr lower, ir::Expr upper) : 
        type(Type::RANGE), lower(lower), upper(upper) {}

    Type         type;
    ir::IndexSet set;
    ir::Expr     lower;
    ir::Expr     upper;
  };

  // Used to build up dense tensor literals.
  struct DenseTensorValues {
    enum class Type {UNKNOWN, INT, FLOAT, COMPLEX};

    DenseTensorValues() : dimSizes(1), type(Type::UNKNOWN) {};

    void addDimension() { dimSizes.push_back(1); }
    void addIntValues(const std::vector<int> &);
    void addFloatValues(const std::vector<double> &);
    void addComplexValues(const std::vector<double_complex> &);
    void merge(const DenseTensorValues &);

    std::vector<unsigned> dimSizes;
    std::vector<int>      intVals;
    std::vector<double>   floatVals;
    std::vector<double>   complexVals; // pairs are flattened to pass as void*
    Type                  type;
  };

  typedef std::unordered_map<SetType::Ptr, ir::Expr> SetExprMap;

private:
  ir::Expr     emitExpr(FIRNode::Ptr);
  ir::Stmt     emitStmt(Stmt::Ptr);
  ir::Type     emitType(Type::Ptr);
  ir::IndexSet emitIndexSet(IndexSet::Ptr);
  ir::Field    emitField(FieldDecl::Ptr);
  ir::Field    emitTupleElement(TupleElement::Ptr);
  ir::Var      emitVar(IdentDecl::Ptr);
  Domain       emitDomain(ForDomain::Ptr);
 
  void addVarOrConst(VarDecl::Ptr, bool = false);
  void addWhileOrDoWhile(WhileStmt::Ptr, bool = false);
  void addAssign(const std::vector<ir::Expr> &, ir::Expr);

  void addSymbol(ir::Var, IdentDecl::Ptr = IdentDecl::Ptr());
  void addSymbol(const std::string&, ir::Var, internal::Symbol::Access, 
                 IdentDecl::Ptr = IdentDecl::Ptr());

  void              emitUnstructuredSetType(UnstructuredSetType::Ptr);
  void              emitDenseTensorLiteral(DenseTensorLiteral::Ptr);
  DenseTensorValues emitTensorValues(DenseTensorLiteral::Ptr);

  ir::Stmt getCallStmts();

private:
  // Used during IR generation to store call and map statements that have to be 
  // emitted before an expression can be fully evaluated at runtime. Needed to 
  // get around lack of call and map expressions.
  std::vector<ir::Stmt> calls;

  ir::Expr     retExpr;
  ir::Stmt     retStmt;
  ir::Type     retType;
  ir::IndexSet retIndexSet;
  ir::Field    retField;
  ir::Var      retVar;
  Domain       retDomain;

  internal::ProgramContext *ctx;
  SetExprMap                setExprs;
};

}
}

#endif

