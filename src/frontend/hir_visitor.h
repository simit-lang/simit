#ifndef SIMIT_HIR_VISITOR_H
#define SIMIT_HIR_VISITOR_H

#include <memory>

namespace simit {
namespace hir {

struct Program;
struct StmtBlock;
struct RangeIndexSet;
struct SetIndexSet;
struct GenericIndexSet;
struct DynamicIndexSet;
struct ElementType;
struct Endpoint;
struct SetType;
struct TupleLength;
struct TupleType;
struct ScalarType;
struct NDTensorType;
struct Identifier;
struct IdentDecl;
struct FieldDecl;
struct ElementTypeDecl;
struct Argument;
struct InOutArgument;
struct ExternDecl;
struct FuncDecl;
struct VarDecl;
struct ConstDecl;
struct WhileStmt;
struct DoWhileStmt;
struct IfStmt;
struct IndexSetDomain;
struct RangeDomain;
struct ForStmt;
struct PrintStmt;
struct ExprStmt;
struct AssignStmt;
struct Slice;
struct ExprParam;
struct MapExpr;
struct ReducedMapExpr;
struct UnreducedMapExpr;
struct UnaryExpr;
struct BinaryExpr;
struct NaryExpr;
struct OrExpr;
struct AndExpr;
struct XorExpr;
struct EqExpr;
struct NotExpr;
struct AddExpr;
struct SubExpr;
struct MulExpr;
struct DivExpr;
struct LeftDivExpr;
struct ElwiseMulExpr;
struct ElwiseDivExpr;
struct NegExpr;
struct ExpExpr;
struct TransposeExpr;
struct CallExpr;
struct TensorReadExpr;
struct TupleReadExpr;
struct FieldReadExpr;
struct ParenExpr;
struct VarExpr;
struct IntLiteral;
struct FloatLiteral;
struct BoolLiteral;
struct ComplexLiteral;
struct StringLiteral;
struct IntVectorLiteral;
struct FloatVectorLiteral;
struct ComplexVectorLiteral;
struct NDTensorLiteral;
struct ApplyStmt;
struct Test;

class HIRVisitor {
public:
  virtual void visit(std::shared_ptr<Program>);
  virtual void visit(std::shared_ptr<StmtBlock>);
  virtual void visit(std::shared_ptr<RangeIndexSet> op) {}
  virtual void visit(std::shared_ptr<SetIndexSet> op) {}
  virtual void visit(std::shared_ptr<GenericIndexSet> op) {}
  virtual void visit(std::shared_ptr<DynamicIndexSet> op) {}
  virtual void visit(std::shared_ptr<ElementType> op) {}
  virtual void visit(std::shared_ptr<Endpoint> op) {}
  virtual void visit(std::shared_ptr<SetType>);
  virtual void visit(std::shared_ptr<TupleLength> op) {}
  virtual void visit(std::shared_ptr<TupleType>);
  virtual void visit(std::shared_ptr<ScalarType> op) {}
  virtual void visit(std::shared_ptr<NDTensorType>);
  virtual void visit(std::shared_ptr<Identifier> op) {}
  virtual void visit(std::shared_ptr<IdentDecl>);
  virtual void visit(std::shared_ptr<FieldDecl>);
  virtual void visit(std::shared_ptr<ElementTypeDecl>);
  virtual void visit(std::shared_ptr<Argument>);
  virtual void visit(std::shared_ptr<InOutArgument>);
  virtual void visit(std::shared_ptr<ExternDecl>);
  virtual void visit(std::shared_ptr<FuncDecl>);
  virtual void visit(std::shared_ptr<VarDecl>);
  virtual void visit(std::shared_ptr<ConstDecl>);
  virtual void visit(std::shared_ptr<WhileStmt>);
  virtual void visit(std::shared_ptr<DoWhileStmt>);
  virtual void visit(std::shared_ptr<IfStmt>);
  virtual void visit(std::shared_ptr<IndexSetDomain>);
  virtual void visit(std::shared_ptr<RangeDomain>);
  virtual void visit(std::shared_ptr<ForStmt>);
  virtual void visit(std::shared_ptr<PrintStmt>);
  virtual void visit(std::shared_ptr<ExprStmt>);
  virtual void visit(std::shared_ptr<AssignStmt>);
  virtual void visit(std::shared_ptr<Slice> op) {}
  virtual void visit(std::shared_ptr<ExprParam>);
  virtual void visit(std::shared_ptr<MapExpr>);
  virtual void visit(std::shared_ptr<ReducedMapExpr>);
  virtual void visit(std::shared_ptr<UnreducedMapExpr>);
  virtual void visit(std::shared_ptr<OrExpr>);
  virtual void visit(std::shared_ptr<AndExpr>);
  virtual void visit(std::shared_ptr<XorExpr>);
  virtual void visit(std::shared_ptr<EqExpr>);
  virtual void visit(std::shared_ptr<NotExpr>);
  virtual void visit(std::shared_ptr<AddExpr>);
  virtual void visit(std::shared_ptr<SubExpr>);
  virtual void visit(std::shared_ptr<MulExpr>);
  virtual void visit(std::shared_ptr<DivExpr>);
  virtual void visit(std::shared_ptr<LeftDivExpr>);
  virtual void visit(std::shared_ptr<ElwiseMulExpr>);
  virtual void visit(std::shared_ptr<ElwiseDivExpr>);
  virtual void visit(std::shared_ptr<NegExpr>);
  virtual void visit(std::shared_ptr<ExpExpr>);
  virtual void visit(std::shared_ptr<TransposeExpr>);
  virtual void visit(std::shared_ptr<CallExpr>);
  virtual void visit(std::shared_ptr<TensorReadExpr>);
  virtual void visit(std::shared_ptr<TupleReadExpr>);
  virtual void visit(std::shared_ptr<FieldReadExpr>);
  virtual void visit(std::shared_ptr<ParenExpr>);
  virtual void visit(std::shared_ptr<VarExpr> op) {}
  virtual void visit(std::shared_ptr<IntLiteral> op) {}
  virtual void visit(std::shared_ptr<FloatLiteral> op) {}
  virtual void visit(std::shared_ptr<BoolLiteral> op) {}
  virtual void visit(std::shared_ptr<ComplexLiteral> op) {}
  virtual void visit(std::shared_ptr<StringLiteral> op) {}
  virtual void visit(std::shared_ptr<IntVectorLiteral> op) {}
  virtual void visit(std::shared_ptr<FloatVectorLiteral> op) {}
  virtual void visit(std::shared_ptr<ComplexVectorLiteral> op) {}
  virtual void visit(std::shared_ptr<NDTensorLiteral>);
  virtual void visit(std::shared_ptr<ApplyStmt>);
  virtual void visit(std::shared_ptr<Test>);

private:
  void visitUnaryExpr(std::shared_ptr<UnaryExpr>); 
  void visitBinaryExpr(std::shared_ptr<BinaryExpr>); 
  void visitNaryExpr(std::shared_ptr<NaryExpr>); 
};

}
}

#endif

