#ifndef SIMIT_HIR_VISITOR_H
#define SIMIT_HIR_VISITOR_H

#include <memory>

namespace simit {
namespace hir {

struct Program;
struct StmtBlock;
struct RangeIndexSet;
struct SetIndexSet;
struct DynamicIndexSet;
struct ElementType;
struct Endpoint;
struct SetType;
struct TupleType;
struct ScalarTensorType;
struct NonScalarTensorType;
struct Field;
struct ElementTypeDecl;
struct IdentDecl;
struct Argument;
struct ExternDecl;
struct FuncDecl;
struct ProcDecl;
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
struct ElwiseMulExpr;
struct ElwiseDivExpr;
struct NegExpr;
struct ExpExpr;
struct TransposeExpr;
struct CallExpr;
struct TensorReadExpr;
struct FieldReadExpr;
struct VarExpr;
struct IntLiteral;
struct FloatLiteral;
struct BoolLiteral;
struct DenseIntVector;
struct DenseFloatVector;
struct DenseNDTensor;
struct DenseTensorLiteral;
struct Test;

class HIRVisitor {
public:
  virtual void visit(std::shared_ptr<Program>);
  virtual void visit(std::shared_ptr<StmtBlock>);
  virtual void visit(std::shared_ptr<RangeIndexSet> op) {}
  virtual void visit(std::shared_ptr<SetIndexSet> op) {}
  virtual void visit(std::shared_ptr<DynamicIndexSet> op) {}
  virtual void visit(std::shared_ptr<ElementType> op) {}
  virtual void visit(std::shared_ptr<Endpoint> op) {}
  virtual void visit(std::shared_ptr<SetType>);
  virtual void visit(std::shared_ptr<TupleType>);
  virtual void visit(std::shared_ptr<ScalarTensorType> op) {}
  virtual void visit(std::shared_ptr<NonScalarTensorType>);
  virtual void visit(std::shared_ptr<Field>);
  virtual void visit(std::shared_ptr<ElementTypeDecl>);
  virtual void visit(std::shared_ptr<IdentDecl>);
  virtual void visit(std::shared_ptr<Argument>);
  virtual void visit(std::shared_ptr<ExternDecl>);
  virtual void visit(std::shared_ptr<FuncDecl>);
  virtual void visit(std::shared_ptr<ProcDecl>);
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
  virtual void visit(std::shared_ptr<UnaryExpr>);
  virtual void visit(std::shared_ptr<BinaryExpr>);
  virtual void visit(std::shared_ptr<NaryExpr>);
  virtual void visit(std::shared_ptr<OrExpr>);
  virtual void visit(std::shared_ptr<AndExpr>);
  virtual void visit(std::shared_ptr<XorExpr>);
  virtual void visit(std::shared_ptr<EqExpr>);
  virtual void visit(std::shared_ptr<NotExpr>);
  virtual void visit(std::shared_ptr<AddExpr>);
  virtual void visit(std::shared_ptr<SubExpr>);
  virtual void visit(std::shared_ptr<MulExpr>);
  virtual void visit(std::shared_ptr<DivExpr>);
  virtual void visit(std::shared_ptr<ElwiseMulExpr>);
  virtual void visit(std::shared_ptr<ElwiseDivExpr>);
  virtual void visit(std::shared_ptr<NegExpr>);
  virtual void visit(std::shared_ptr<ExpExpr>);
  virtual void visit(std::shared_ptr<TransposeExpr>);
  virtual void visit(std::shared_ptr<CallExpr>);
  virtual void visit(std::shared_ptr<TensorReadExpr>);
  virtual void visit(std::shared_ptr<FieldReadExpr>);
  virtual void visit(std::shared_ptr<VarExpr> op) {}
  virtual void visit(std::shared_ptr<IntLiteral> op) {}
  virtual void visit(std::shared_ptr<FloatLiteral> op) {}
  virtual void visit(std::shared_ptr<BoolLiteral> op) {}
  virtual void visit(std::shared_ptr<DenseIntVector> op) {}
  virtual void visit(std::shared_ptr<DenseFloatVector> op) {}
  virtual void visit(std::shared_ptr<DenseNDTensor>);
  virtual void visit(std::shared_ptr<DenseTensorLiteral>);
  virtual void visit(std::shared_ptr<Test>);
};

}
}

#endif

