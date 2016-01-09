#ifndef SIMIT_HIR_H
#define SIMIT_HIR_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>

#include "types.h"
#include "scanner.h"
#include "hir_visitor.h"

namespace simit {
namespace hir {

// Base class for higher-level intermediate representation used by frontend.
struct HIRNode : public std::enable_shared_from_this<HIRNode> {
protected:
  unsigned lineBegin;
  unsigned colBegin;
  unsigned lineEnd;
  unsigned colEnd;

public:
  typedef std::shared_ptr<HIRNode> Ptr;

  HIRNode() : lineBegin(0), colBegin(0), lineEnd(0), colEnd(0) {}

  virtual void accept(HIRVisitor *) = 0; 

  virtual unsigned getLineBegin() { return lineBegin; }
  virtual unsigned getColBegin() { return colBegin; }
  virtual unsigned getLineEnd() { return lineEnd; }
  virtual unsigned getColEnd() { return colEnd; }

  void setBeginLoc(const internal::Token &);
  void setEndLoc(const internal::Token &);
  void setLoc(const internal::Token &);
  void setLoc(HIRNode::Ptr);

  friend std::ostream &operator<<(std::ostream &, HIRNode &);
};

template <typename T>
inline bool isa(HIRNode::Ptr ptr) {
  return (bool)std::dynamic_pointer_cast<T>(ptr);
}

template <typename T>
inline const std::shared_ptr<T> to(HIRNode::Ptr ptr) {
  iassert(isa<T>(ptr));
  return std::static_pointer_cast<T>(ptr);
}

struct Program : public HIRNode {
  std::vector<HIRNode::Ptr> elems;

  typedef std::shared_ptr<Program> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<Program>(shared_from_this()));
  }
};

struct Stmt : public HIRNode {
  typedef std::shared_ptr<Stmt> Ptr;
};

struct StmtBlock : public Stmt {
  std::vector<Stmt::Ptr> stmts;
  
  typedef std::shared_ptr<StmtBlock> Ptr;
  
  virtual void accept(HIRVisitor *visitor) { 
    visitor->visit(to<StmtBlock>(shared_from_this()));
  }
};

struct Expr : public HIRNode {
  typedef std::vector<ir::Type> Type;

  Type type;

  typedef std::shared_ptr<Expr> Ptr;
};

struct IndexSet : public HIRNode {
  typedef std::shared_ptr<IndexSet> Ptr;
};

struct RangeIndexSet : public IndexSet {
  int range;
  
  typedef std::shared_ptr<RangeIndexSet> Ptr;
  
  virtual void accept(HIRVisitor *visitor) { 
    visitor->visit(to<RangeIndexSet>(shared_from_this()));
  }
};

struct SetIndexSet : public IndexSet {
  std::string setName;
  
  typedef std::shared_ptr<SetIndexSet> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<SetIndexSet>(shared_from_this()));
  }
};

struct DynamicIndexSet : public IndexSet {
  typedef std::shared_ptr<DynamicIndexSet> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<DynamicIndexSet>(shared_from_this()));
  }
};

struct Type : public HIRNode {
  typedef std::shared_ptr<Type> Ptr;
};

struct ElementType : public Type {
  std::string ident;
  
  typedef std::shared_ptr<ElementType> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<ElementType>(shared_from_this()));
  }
};

struct Endpoint : public HIRNode {
  std::string setName;
  
  typedef std::shared_ptr<Endpoint> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<Endpoint>(shared_from_this()));
  }
};

struct SetType : public Type {
  ElementType::Ptr           element;
  std::vector<Endpoint::Ptr> endpoints;
  
  typedef std::shared_ptr<SetType> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<SetType>(shared_from_this()));
  }
};

struct TupleLength : public HIRNode {
  int val;

  typedef std::shared_ptr<TupleLength> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<TupleLength>(shared_from_this()));
  }
};

struct TupleType : public Type {
  ElementType::Ptr element;
  TupleLength::Ptr length;
  
  typedef std::shared_ptr<TupleType> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<TupleType>(shared_from_this()));
  }
};

struct TensorType : public Type {
  typedef std::shared_ptr<TensorType> Ptr;
};

struct ScalarType : public TensorType {
  enum class Type {INT, FLOAT, BOOL};

  Type type;
  
  typedef std::shared_ptr<ScalarType> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<ScalarType>(shared_from_this()));
  }
};

struct NDTensorType : public TensorType {
  std::vector<IndexSet::Ptr> indexSets;
  TensorType::Ptr            blockType;
  bool                       columnVector;
  
  typedef std::shared_ptr<NDTensorType> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<NDTensorType>(shared_from_this()));
  }
};

struct Identifier : public HIRNode {
  std::string ident;

  typedef std::shared_ptr<Identifier> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<Identifier>(shared_from_this()));
  }
};

struct IdentDecl : public HIRNode {
  Identifier::Ptr name;
  Type::Ptr       type;
  
  typedef std::shared_ptr<IdentDecl> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<IdentDecl>(shared_from_this()));
  }
  
  virtual unsigned getLineBegin() { return name->getLineBegin(); }
  virtual unsigned getColBegin() { return name->getColBegin(); }
  virtual unsigned getLineEnd() { return type->getLineEnd(); }
  virtual unsigned getColEnd() { return type->getColEnd(); }
};

struct Field : public HIRNode {
  IdentDecl::Ptr field;
  
  typedef std::shared_ptr<Field> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<Field>(shared_from_this()));
  }

  virtual unsigned getLineBegin() { return field->getLineBegin(); }
  virtual unsigned getColBegin() { return field->getColBegin(); }
};

struct ElementTypeDecl : public HIRNode {
  Identifier::Ptr         name;
  std::vector<Field::Ptr> fields; 
  
  typedef std::shared_ptr<ElementTypeDecl> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<ElementTypeDecl>(shared_from_this()));
  }
};

struct Argument : public IdentDecl {
  bool inout;
  
  typedef std::shared_ptr<Argument> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<Argument>(shared_from_this()));
  }
};

struct ExternDecl : public HIRNode {
  Argument::Ptr var;
  
  typedef std::shared_ptr<ExternDecl> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<ExternDecl>(shared_from_this()));
  }
};

struct FuncDecl : public HIRNode {
  Identifier::Ptr             name;
  std::vector<Argument::Ptr>  args;
  std::vector<IdentDecl::Ptr> results;
  StmtBlock::Ptr              body;
  
  typedef std::shared_ptr<FuncDecl> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<FuncDecl>(shared_from_this()));
  }
};

struct ProcDecl : public FuncDecl {
  typedef std::shared_ptr<ProcDecl> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<ProcDecl>(shared_from_this()));
  }
};

struct VarDecl : public Stmt {
  IdentDecl::Ptr var;
  Expr::Ptr      initVal;
 
  typedef std::shared_ptr<VarDecl> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<VarDecl>(shared_from_this()));
  }
};

struct ConstDecl : public VarDecl {
  typedef std::shared_ptr<ConstDecl> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<ConstDecl>(shared_from_this()));
  }
};

struct WhileStmt : public Stmt {
  Expr::Ptr      cond;
  StmtBlock::Ptr body;
  
  typedef std::shared_ptr<WhileStmt> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<WhileStmt>(shared_from_this()));
  }
};

struct DoWhileStmt : public WhileStmt {
  typedef std::shared_ptr<DoWhileStmt> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<DoWhileStmt>(shared_from_this()));
  }

  virtual unsigned getLineEnd() { return cond->getLineEnd(); }
  virtual unsigned getColEnd() { return cond->getColEnd(); }
};

struct IfStmt : public Stmt {
  Expr::Ptr cond;
  Stmt::Ptr ifBody;
  Stmt::Ptr elseBody;
  
  typedef std::shared_ptr<IfStmt> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<IfStmt>(shared_from_this()));
  }

  virtual unsigned getLineEnd() {
    return (lineEnd == 0) ? elseBody->getLineEnd() : lineEnd;
  }
  virtual unsigned getColEnd() {
    return (lineEnd == 0) ? elseBody->getColEnd() : colEnd;
  }
};

struct ForDomain : public HIRNode {
  typedef std::shared_ptr<ForDomain> Ptr;
};

struct IndexSetDomain : public ForDomain {
  SetIndexSet::Ptr set;

  typedef std::shared_ptr<IndexSetDomain> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<IndexSetDomain>(shared_from_this()));
  }

  virtual unsigned getLineBegin() { return set->getLineBegin(); }
  virtual unsigned getColBegin() { return set->getColBegin(); }
  virtual unsigned getLineEnd() { return set->getLineEnd(); }
  virtual unsigned getColEnd() { return set->getColEnd(); }
};

struct RangeDomain : public ForDomain {
  Expr::Ptr lower;
  Expr::Ptr upper;

  typedef std::shared_ptr<RangeDomain> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<RangeDomain>(shared_from_this()));
  }
  
  virtual unsigned getLineBegin() { return lower->getLineBegin(); }
  virtual unsigned getColBegin() { return lower->getColBegin(); }
  virtual unsigned getLineEnd() { return upper->getLineEnd(); }
  virtual unsigned getColEnd() { return upper->getColEnd(); }
};

struct ForStmt : public Stmt {
  Identifier::Ptr loopVar;
  ForDomain::Ptr  domain;
  StmtBlock::Ptr  body;
  
  typedef std::shared_ptr<ForStmt> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<ForStmt>(shared_from_this()));
  }
};

struct PrintStmt : public Stmt {
  Expr::Ptr expr;
  
  typedef std::shared_ptr<PrintStmt> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<PrintStmt>(shared_from_this()));
  }
};

struct ExprStmt : public Stmt {
  Expr::Ptr expr;
  
  typedef std::shared_ptr<ExprStmt> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<ExprStmt>(shared_from_this()));
  }

  virtual unsigned getLineBegin() { return expr->getLineBegin(); }
  virtual unsigned getColBegin() { return expr->getColBegin(); }
};

struct AssignStmt : public ExprStmt {
  std::vector<Expr::Ptr> lhs;
 
  typedef std::shared_ptr<AssignStmt> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<AssignStmt>(shared_from_this()));
  }

  virtual unsigned getLineBegin() { return lhs.front()->getLineBegin(); }
  virtual unsigned getColBegin() { return lhs.front()->getColBegin(); }
};

struct ReadParam : public HIRNode {
  typedef std::shared_ptr<ReadParam> Ptr;

  virtual bool isSlice() { return false; }
};

struct Slice : public ReadParam {
  typedef std::shared_ptr<Slice> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<Slice>(shared_from_this()));
  }

  virtual bool isSlice() { return true; }
};

struct ExprParam : public ReadParam {
  Expr::Ptr expr;
  
  typedef std::shared_ptr<ExprParam> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<ExprParam>(shared_from_this()));
  }
  
  virtual unsigned getLineBegin() { return expr->getLineBegin(); }
  virtual unsigned getColBegin() { return expr->getColBegin(); }
  virtual unsigned getLineEnd() { return expr->getLineEnd(); }
  virtual unsigned getColEnd() { return expr->getColEnd(); }
};

struct MapExpr : public Expr {
  enum class ReductionOp {SUM, NONE};

  Identifier::Ptr        func;
  std::vector<Expr::Ptr> partialActuals;
  Identifier::Ptr        target;
  ReductionOp            op;

  typedef std::shared_ptr<MapExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<MapExpr>(shared_from_this()));
  }

  virtual unsigned getLineEnd() {
    return (op == ReductionOp::NONE) ? target->getLineEnd() : lineEnd;
  }
  virtual unsigned getColEnd() {
    return (op == ReductionOp::NONE) ? target->getColEnd() : colEnd;
  }
};

struct UnaryExpr : public Expr {
  Expr::Ptr operand;

  typedef std::shared_ptr<UnaryExpr> Ptr;
};

struct BinaryExpr : public Expr {
  Expr::Ptr lhs;
  Expr::Ptr rhs;

  typedef std::shared_ptr<BinaryExpr> Ptr;
  
  virtual unsigned getLineBegin() { return lhs->getLineBegin(); }
  virtual unsigned getColBegin() { return lhs->getColBegin(); }
  virtual unsigned getLineEnd() { return rhs->getLineEnd(); }
  virtual unsigned getColEnd() { return rhs->getColEnd(); }
};
typedef std::shared_ptr<BinaryExpr> BinaryExprPtr;

struct NaryExpr : public Expr {
  std::vector<Expr::Ptr> operands;

  typedef std::shared_ptr<NaryExpr> Ptr;
};

struct OrExpr : public BinaryExpr {
  typedef std::shared_ptr<OrExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<OrExpr>(shared_from_this()));
  }
};

struct AndExpr : public BinaryExpr {
  typedef std::shared_ptr<AndExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<AndExpr>(shared_from_this()));
  }
};

struct XorExpr : public BinaryExpr {
  typedef std::shared_ptr<XorExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<XorExpr>(shared_from_this()));
  }
};

struct EqExpr : public NaryExpr {
  enum class Op {LT, LE, GT, GE, EQ, NE};

  std::vector<Op> ops;
  
  typedef std::shared_ptr<EqExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<EqExpr>(shared_from_this()));
  }
  
  virtual unsigned getLineBegin() { return operands.front()->getLineBegin(); }
  virtual unsigned getColBegin() { return operands.front()->getColBegin(); }
  virtual unsigned getLineEnd() { return operands.back()->getLineEnd(); }
  virtual unsigned getColEnd() { return operands.back()->getColEnd(); }
};

struct NotExpr : public UnaryExpr {
  typedef std::shared_ptr<NotExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<NotExpr>(shared_from_this()));
  }

  virtual unsigned getLineEnd() { return operand->getLineEnd(); }
  virtual unsigned getColEnd() { return operand->getColEnd(); }
};

struct AddExpr : public BinaryExpr {
  typedef std::shared_ptr<AddExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<AddExpr>(shared_from_this()));
  }
};

struct SubExpr : public BinaryExpr {
  typedef std::shared_ptr<SubExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<SubExpr>(shared_from_this()));
  }
};

struct MulExpr : public BinaryExpr {
  typedef std::shared_ptr<MulExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<MulExpr>(shared_from_this()));
  }
};

struct DivExpr : public BinaryExpr {
  typedef std::shared_ptr<DivExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<DivExpr>(shared_from_this()));
  }
};

struct ElwiseMulExpr : public BinaryExpr {
  typedef std::shared_ptr<ElwiseMulExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<ElwiseMulExpr>(shared_from_this()));
  }
};

struct ElwiseDivExpr : public BinaryExpr {
  typedef std::shared_ptr<ElwiseDivExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<ElwiseDivExpr>(shared_from_this()));
  }
};

struct NegExpr : public UnaryExpr {
  bool negate;
  
  typedef std::shared_ptr<NegExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<NegExpr>(shared_from_this()));
  }

  virtual unsigned getLineEnd() { return operand->getLineEnd(); }
  virtual unsigned getColEnd() { return operand->getColEnd(); }
};

struct ExpExpr : public BinaryExpr {
  typedef std::shared_ptr<ExpExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<ExpExpr>(shared_from_this()));
  }
};

struct TransposeExpr : public UnaryExpr {
  typedef std::shared_ptr<TransposeExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<TransposeExpr>(shared_from_this()));
  }

  virtual unsigned getLineBegin() { return operand->getLineBegin(); }
  virtual unsigned getColBegin() { return operand->getColBegin(); }
};

struct CallExpr : public Expr {
  Identifier::Ptr        func;
  std::vector<Expr::Ptr> arguments;
  
  typedef std::shared_ptr<CallExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<CallExpr>(shared_from_this()));
  }

  virtual unsigned getLineBegin() { return func->getLineBegin(); }
  virtual unsigned getColBegin() { return func->getColBegin(); }
};

struct TensorReadExpr : public Expr {
  Expr::Ptr                   tensor;
  std::vector<ReadParam::Ptr> indices;
  
  typedef std::shared_ptr<TensorReadExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<TensorReadExpr>(shared_from_this()));
  }
  
  virtual unsigned getLineBegin() { return tensor->getLineBegin(); }
  virtual unsigned getColBegin() { return tensor->getColBegin(); }
};

struct TupleReadExpr : public Expr {
  Expr::Ptr tuple;
  Expr::Ptr index;

  typedef std::shared_ptr<TupleReadExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<TupleReadExpr>(shared_from_this()));
  }

  virtual unsigned getLineBegin() { return tuple->getLineBegin(); }
  virtual unsigned getColBegin() { return tuple->getColBegin(); }
};

struct FieldReadExpr : public Expr {
  Expr::Ptr       setOrElem;
  Identifier::Ptr field;
  
  typedef std::shared_ptr<FieldReadExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<FieldReadExpr>(shared_from_this()));
  }
  
  virtual unsigned getLineBegin() { return setOrElem->getLineBegin(); }
  virtual unsigned getColBegin() { return setOrElem->getColBegin(); }
  virtual unsigned getLineEnd() { return field->getLineEnd(); }
  virtual unsigned getColEnd() { return field->getColEnd(); }
};

struct ParenExpr : public Expr {
  Expr::Ptr expr;

  typedef std::shared_ptr<ParenExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<ParenExpr>(shared_from_this()));
  }
};

struct VarExpr : public Expr {
  std::string ident;
  
  typedef std::shared_ptr<VarExpr> Ptr;
 
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<VarExpr>(shared_from_this()));
  }
};

struct TensorLiteral : public Expr {
  typedef std::shared_ptr<TensorLiteral> Ptr;
};

struct IntLiteral : public TensorLiteral {
  int val;

  typedef std::shared_ptr<IntLiteral> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<IntLiteral>(shared_from_this()));
  }
};

struct FloatLiteral : public TensorLiteral {
  double val;
  
  typedef std::shared_ptr<FloatLiteral> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<FloatLiteral>(shared_from_this()));
  }
};

struct BoolLiteral : public TensorLiteral {
  bool val;
  
  typedef std::shared_ptr<BoolLiteral> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<BoolLiteral>(shared_from_this()));
  }
};

struct DenseTensorLiteral : public TensorLiteral {
  bool transposed;

  typedef std::shared_ptr<DenseTensorLiteral> Ptr;
};

struct IntVectorLiteral : public DenseTensorLiteral {
  std::vector<int> vals;
  
  typedef std::shared_ptr<IntVectorLiteral> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<IntVectorLiteral>(shared_from_this()));
  }
};

struct FloatVectorLiteral : public DenseTensorLiteral {
  std::vector<double> vals;
  
  typedef std::shared_ptr<FloatVectorLiteral> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<FloatVectorLiteral>(shared_from_this()));
  }
};

struct NDTensorLiteral : public DenseTensorLiteral {
  std::vector<DenseTensorLiteral::Ptr> elems;
  
  typedef std::shared_ptr<NDTensorLiteral> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<NDTensorLiteral>(shared_from_this()));
  }
};

struct Test : public HIRNode {
  Identifier::Ptr        func;
  std::vector<Expr::Ptr> args;
  Expr::Ptr              expected;
  
  typedef std::shared_ptr<Test> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<Test>(shared_from_this()));
  }
};

}
}

#endif

