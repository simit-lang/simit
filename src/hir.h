#ifndef SIMIT_HIR_H
#define SIMIT_HIR_H

#include <string>
#include <vector>
#include <memory>

#include "types.h"
#include "scanner.h"
#include "hir_visitor.h"

namespace simit {
namespace hir {

struct HIRNode : public std::enable_shared_from_this<HIRNode> {
  unsigned int lineNum;
  unsigned int colNum;

  typedef std::shared_ptr<HIRNode> Ptr;

  virtual void accept(HIRVisitor *) = 0; 

  inline void setLoc(const internal::Token &token) {
    lineNum = token.lineNum;
    colNum = token.colNum;
  }
  inline void setLoc(const std::shared_ptr<HIRNode> &node) {
    lineNum = node->lineNum;
    colNum = node->colNum;
  }

  friend std::ostream &operator<<(std::ostream &, HIRNode &);
};

template <typename T>
inline bool isa(HIRNode::Ptr ptr) {
  return (bool)std::dynamic_pointer_cast<T>(ptr);
}

template <typename T>
inline const std::shared_ptr<T> to(HIRNode::Ptr ptr) {
  iassert(isa<T>(ptr)) << "Wrong type";
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
  ElementType::Ptr element;
  std::vector<Endpoint::Ptr> endpoints;
  
  typedef std::shared_ptr<SetType> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<SetType>(shared_from_this()));
  }
};

struct TupleType : public Type {
  ElementType::Ptr element;
  int length;
  
  typedef std::shared_ptr<TupleType> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<TupleType>(shared_from_this()));
  }
};

struct TensorType : public Type {
  typedef std::shared_ptr<TensorType> Ptr;
};

struct ScalarTensorType : public TensorType {
  enum class Type {INT, FLOAT, BOOL};

  Type type;
  
  typedef std::shared_ptr<ScalarTensorType> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<ScalarTensorType>(shared_from_this()));
  }
};

struct NonScalarTensorType : public TensorType {
  std::vector<IndexSet::Ptr> indexSets;
  TensorType::Ptr blockType;  
  bool transposed;
  
  typedef std::shared_ptr<NonScalarTensorType> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<NonScalarTensorType>(shared_from_this()));
  }
};

struct Field : public HIRNode {
  std::string name;
  TensorType::Ptr type;
  
  typedef std::shared_ptr<Field> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<Field>(shared_from_this()));
  }
};

struct ElementTypeDecl : public HIRNode {
  std::string ident;
  std::vector<Field::Ptr> fields; 
  
  typedef std::shared_ptr<ElementTypeDecl> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<ElementTypeDecl>(shared_from_this()));
  }
};

struct IdentDecl : public HIRNode {
  std::string ident;
  Type::Ptr type;
  
  typedef std::shared_ptr<IdentDecl> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<IdentDecl>(shared_from_this()));
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
  std::string name;
  std::vector<Argument::Ptr> args;
  std::vector<IdentDecl::Ptr> results;
  StmtBlock::Ptr body;
  
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
  Expr::Ptr initVal;
 
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
  Expr::Ptr cond;
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
};

struct IfStmt : public Stmt {
  Expr::Ptr cond;
  Stmt::Ptr ifBody;
  Stmt::Ptr elseBody;
  
  typedef std::shared_ptr<IfStmt> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<IfStmt>(shared_from_this()));
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
};

struct RangeDomain : public ForDomain {
  Expr::Ptr lower;
  Expr::Ptr upper;

  typedef std::shared_ptr<RangeDomain> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<RangeDomain>(shared_from_this()));
  }
};

struct ForStmt : public Stmt {
  std::string loopVarName;
  ForDomain::Ptr domain;
  StmtBlock::Ptr body;
  
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
};

struct AssignStmt : public ExprStmt {
  std::vector<Expr::Ptr> lhs;
 
  typedef std::shared_ptr<AssignStmt> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<AssignStmt>(shared_from_this()));
  }
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
};

struct MapExpr : public Expr {
  enum class ReductionOp {SUM, NONE};

  std::string funcName;
  std::vector<Expr::Ptr> partialActuals;
  std::string targetName;
  ReductionOp op;

  unsigned int funcNameLineNum;
  unsigned int funcNameColNum;
  unsigned int targetNameLineNum;
  unsigned int targetNameColNum;
 
  typedef std::shared_ptr<MapExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<MapExpr>(shared_from_this()));
  }
  
  inline void setFuncNameLoc(const internal::Token &token) {
    funcNameLineNum = token.lineNum;
    funcNameColNum = token.colNum;
  }
  inline void setTargetNameLoc(const internal::Token &token) {
    targetNameLineNum = token.lineNum;
    targetNameColNum = token.colNum;
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
};

struct NotExpr : public UnaryExpr {
  typedef std::shared_ptr<NotExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<NotExpr>(shared_from_this()));
  }
};

// TODO: SolveExpr?

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
};

struct CallExpr : public NaryExpr {
  std::string funcName;
  
  typedef std::shared_ptr<CallExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<CallExpr>(shared_from_this()));
  }
};

struct TensorReadExpr : public Expr {
  Expr::Ptr tensor;
  std::vector<ReadParam::Ptr> indices;
  
  typedef std::shared_ptr<TensorReadExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<TensorReadExpr>(shared_from_this()));
  }
};

struct FieldReadExpr : public Expr {
  Expr::Ptr setOrElem;
  std::string fieldName;
  
  typedef std::shared_ptr<FieldReadExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<FieldReadExpr>(shared_from_this()));
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

// TODO: StringLiteral?

struct DenseTensorElement : public HIRNode {
  typedef std::shared_ptr<DenseTensorElement> Ptr;
};

struct DenseIntVector : public DenseTensorElement {
  std::vector<int> vals;
  
  typedef std::shared_ptr<DenseIntVector> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<DenseIntVector>(shared_from_this()));
  }
};

struct DenseFloatVector : public DenseTensorElement {
  std::vector<double> vals;
  
  typedef std::shared_ptr<DenseFloatVector> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<DenseFloatVector>(shared_from_this()));
  }
};

struct DenseNDTensor : public DenseTensorElement {
  std::vector<DenseTensorElement::Ptr> elems;
  
  typedef std::shared_ptr<DenseNDTensor> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<DenseNDTensor>(shared_from_this()));
  }
};

struct DenseTensorLiteral : public TensorLiteral {
  DenseTensorElement::Ptr tensor;

  typedef std::shared_ptr<DenseTensorLiteral> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<DenseTensorLiteral>(shared_from_this()));
  }
};

struct Test : public HIRNode {
  std::string funcName;
  std::vector<Expr::Ptr> args;
  Expr::Ptr expected;
  
  typedef std::shared_ptr<Test> Ptr;
  
  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<Test>(shared_from_this()));
  }
};

}
}

#endif

