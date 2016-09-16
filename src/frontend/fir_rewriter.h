#ifndef SIMIT_FIR_REWRITER_H
#define SIMIT_FIR_REWRITER_H

#include <memory>

#include "fir.h"
#include "fir_visitor.h"

namespace simit {
namespace fir {

struct FIRRewriter : public FIRVisitor {
  virtual void visit(Program::Ptr);
  virtual void visit(StmtBlock::Ptr);
  virtual void visit(RangeIndexSet::Ptr op) { node = op; }
  virtual void visit(SetIndexSet::Ptr op) { node = op; }
  virtual void visit(DynamicIndexSet::Ptr op) { node = op; }
  virtual void visit(ElementType::Ptr op) { node = op; }
  virtual void visit(Endpoint::Ptr);
  virtual void visit(HomogeneousEdgeSetType::Ptr);
  virtual void visit(HeterogeneousEdgeSetType::Ptr);
  virtual void visit(GridSetType::Ptr);
  virtual void visit(TupleElement::Ptr);
  virtual void visit(NamedTupleType::Ptr);
  virtual void visit(TupleLength::Ptr op) { node = op; }
  virtual void visit(UnnamedTupleType::Ptr);
  virtual void visit(ScalarType::Ptr op) { node = op; }
  virtual void visit(NDTensorType::Ptr);
  virtual void visit(OpaqueType::Ptr op) { node = op; }
  virtual void visit(Identifier::Ptr op) { node = op; }
  virtual void visit(IdentDecl::Ptr);
  virtual void visit(ElementTypeDecl::Ptr);
  virtual void visit(GenericParam::Ptr op) { node = op; }
  virtual void visit(FuncDecl::Ptr);
  virtual void visit(VarDecl::Ptr);
  virtual void visit(WhileStmt::Ptr);
  virtual void visit(IfStmt::Ptr);
  virtual void visit(IndexSetDomain::Ptr);
  virtual void visit(RangeDomain::Ptr);
  virtual void visit(ForStmt::Ptr);
  virtual void visit(PrintStmt::Ptr);
  virtual void visit(ExprStmt::Ptr);
  virtual void visit(AssignStmt::Ptr);
  virtual void visit(Slice::Ptr op) { node = op; }
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
  virtual void visit(ParenExpr::Ptr);
  virtual void visit(VarExpr::Ptr op) { node = op; }
  virtual void visit(IntLiteral::Ptr op) { node = op; }
  virtual void visit(FloatLiteral::Ptr op) { node = op; }
  virtual void visit(BoolLiteral::Ptr op) { node = op; }
  virtual void visit(ComplexLiteral::Ptr op) { node = op; }
  virtual void visit(StringLiteral::Ptr op) { node = op; }
  virtual void visit(IntVectorLiteral::Ptr op) { node = op; }
  virtual void visit(FloatVectorLiteral::Ptr op) { node = op; }
  virtual void visit(ComplexVectorLiteral::Ptr op) { node = op; }
  virtual void visit(NDTensorLiteral::Ptr);
  virtual void visit(ApplyStmt::Ptr);
  virtual void visit(Test::Ptr);

  template <typename T = Program>
  std::shared_ptr<T> rewrite(std::shared_ptr<T> ptr) {
    node.reset();
    ptr->accept(this);
    auto ret = std::static_pointer_cast<T>(node);
    node.reset();
    return ret;
  }

private:
  virtual void visitUnaryExpr(UnaryExpr::Ptr);
  virtual void visitBinaryExpr(BinaryExpr::Ptr);
  virtual void visitNaryExpr(NaryExpr::Ptr);

protected:
  FIRNode::Ptr node;  
};

}
}

#endif

