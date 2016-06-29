#ifndef SIMIT_HIR_H
#define SIMIT_HIR_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <unordered_set>

#include "types.h"
#include "scanner.h"
#include "hir_visitor.h"
#include "program_context.h"

namespace simit {
namespace hir {

struct HIRNode;

template <typename T>
inline bool isa(std::shared_ptr<HIRNode> ptr) {
  return (bool)std::dynamic_pointer_cast<T>(ptr);
}

template <typename T>
inline const std::shared_ptr<T> to(std::shared_ptr<HIRNode> ptr) {
  std::shared_ptr<T> ret = std::dynamic_pointer_cast<T>(ptr);
  iassert((bool)ret);
  return ret;
}

// Base class for higher-level intermediate representation used by frontend.
struct HIRNode : public std::enable_shared_from_this<HIRNode> {
protected:
  unsigned lineBegin;
  unsigned colBegin;
  unsigned lineEnd;
  unsigned colEnd;

  template <typename T = HIRNode> std::shared_ptr<T> self() {
    return to<T>(shared_from_this());
  }

public:
  typedef std::shared_ptr<HIRNode> Ptr;

  HIRNode() : lineBegin(0), colBegin(0), lineEnd(0), colEnd(0) {}

  virtual void copy(HIRNode::Ptr node) { setLoc(node); }
  
  virtual HIRNode::Ptr cloneImpl() = 0;

  template <typename T = HIRNode> std::shared_ptr<T> clone() {
    return to<T>(cloneImpl());
  }

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

struct Program : public HIRNode {
  std::vector<HIRNode::Ptr> elems;

  typedef std::shared_ptr<Program> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl(); 

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<Program>());
  }
};

struct Stmt : public HIRNode {
  typedef std::shared_ptr<Stmt> Ptr;
};

struct StmtBlock : public Stmt {
  std::vector<Stmt::Ptr> stmts;
  
  typedef std::shared_ptr<StmtBlock> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) { 
    visitor->visit(self<StmtBlock>());
  }
};

struct Expr : public HIRNode {
  typedef std::vector<ir::Type>    Type;
  typedef internal::Symbol::Access Access;

  Type   type;
  Access access;

  typedef std::shared_ptr<Expr> Ptr;

  Expr() : access(internal::Symbol::Read) {}

  virtual void copy(HIRNode::Ptr);

  bool isReadable();
  bool isWritable();
};

struct IndexSet : public HIRNode {
  typedef std::shared_ptr<IndexSet> Ptr;
};

struct RangeIndexSet : public IndexSet {
  int range;
  
  typedef std::shared_ptr<RangeIndexSet> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) { 
    visitor->visit(self<RangeIndexSet>());
  }
};

struct SetIndexSet : public IndexSet {
  std::string setName;
  
  typedef std::shared_ptr<SetIndexSet> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<SetIndexSet>());
  }
};

struct GenericIndexSet : public SetIndexSet {
  typedef std::shared_ptr<GenericIndexSet> Ptr;

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<GenericIndexSet>());
  }
};

struct DynamicIndexSet : public IndexSet {
  typedef std::shared_ptr<DynamicIndexSet> Ptr;
  
  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<DynamicIndexSet>());
  }
};

struct Type : public HIRNode {
  typedef std::shared_ptr<Type> Ptr;
};

struct ElementType : public Type {
  std::string                     ident;
  std::string                     sourceSet;
  std::unordered_set<std::string> sourceGenericSets;
  
  typedef std::shared_ptr<ElementType> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ElementType>());
  }
};

struct Endpoint : public HIRNode {
  std::string setName;
  
  typedef std::shared_ptr<Endpoint> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<Endpoint>());
  }
};

struct SetType : public Type {
  ElementType::Ptr           element;
  std::vector<Endpoint::Ptr> endpoints;
  
  typedef std::shared_ptr<SetType> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<SetType>());
  }
};

struct TupleLength : public HIRNode {
  int val;

  typedef std::shared_ptr<TupleLength> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<TupleLength>());
  }
};

struct TupleType : public Type {
  ElementType::Ptr element;
  TupleLength::Ptr length;
  
  typedef std::shared_ptr<TupleType> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<TupleType>());
  }
};

struct TensorType : public Type {
  typedef std::shared_ptr<TensorType> Ptr;
};

struct ScalarType : public TensorType {
  enum class Type {INT, FLOAT, BOOL, COMPLEX, STRING};

  Type type;
  
  typedef std::shared_ptr<ScalarType> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ScalarType>());
  }
};

struct NDTensorType : public TensorType {
  std::vector<IndexSet::Ptr> indexSets;
  TensorType::Ptr            blockType;
  bool                       transposed;
  
  typedef std::shared_ptr<NDTensorType> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<NDTensorType>());
  }
};

struct Identifier : public HIRNode {
  std::string ident;

  typedef std::shared_ptr<Identifier> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<Identifier>());
  }
};

struct IdentDecl : public HIRNode {
  Identifier::Ptr name;
  Type::Ptr       type;
  
  typedef std::shared_ptr<IdentDecl> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<IdentDecl>());
  }
  
  virtual unsigned getLineBegin() { return name->getLineBegin(); }
  virtual unsigned getColBegin() { return name->getColBegin(); }
  virtual unsigned getLineEnd() { return type->getLineEnd(); }
  virtual unsigned getColEnd() { return type->getColEnd(); }
};

struct FieldDecl : public HIRNode {
  IdentDecl::Ptr field;
  
  typedef std::shared_ptr<FieldDecl> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<FieldDecl>());
  }

  virtual unsigned getLineBegin() { return field->getLineBegin(); }
  virtual unsigned getColBegin() { return field->getColBegin(); }
};

struct ElementTypeDecl : public HIRNode {
  Identifier::Ptr             name;
  std::vector<FieldDecl::Ptr> fields; 
  
  typedef std::shared_ptr<ElementTypeDecl> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ElementTypeDecl>());
  }
};

struct Argument : public HIRNode {
  IdentDecl::Ptr arg;
  
  typedef std::shared_ptr<Argument> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<Argument>());
  }
  
  virtual unsigned getLineBegin() { return arg->getLineBegin(); }
  virtual unsigned getColBegin() { return arg->getColBegin(); }
  virtual unsigned getLineEnd() { return arg->getLineEnd(); }
  virtual unsigned getColEnd() { return arg->getColEnd(); }

  virtual bool isInOut() { return false; }
};

struct InOutArgument : public Argument {
  typedef std::shared_ptr<InOutArgument> Ptr;
  
  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<InOutArgument>());
  }
  
  virtual unsigned getLineBegin() { return HIRNode::getLineBegin(); }
  virtual unsigned getColBegin() { return HIRNode::getColBegin(); }

  virtual bool isInOut() { return true; }
};

struct ExternDecl : public HIRNode {
  IdentDecl::Ptr var;
  
  typedef std::shared_ptr<ExternDecl> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ExternDecl>());
  }
};

struct FuncDecl : public HIRNode {
  Identifier::Ptr                   name;
  std::vector<GenericIndexSet::Ptr> typeParams;
  std::vector<Argument::Ptr>        args;
  std::vector<IdentDecl::Ptr>       results;
  StmtBlock::Ptr                    body;
  bool                              exported;
  bool                              external;
  std::string                       originalName;
  
  typedef std::shared_ptr<FuncDecl> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<FuncDecl>());
  }
};

struct VarDecl : public Stmt {
  Identifier::Ptr name;
  Type::Ptr       type;
  Expr::Ptr       initVal;

  typedef std::shared_ptr<VarDecl> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<VarDecl>());
  }
};

struct ConstDecl : public VarDecl {
  typedef std::shared_ptr<ConstDecl> Ptr;

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ConstDecl>());
  }
};

struct WhileStmt : public Stmt {
  Expr::Ptr      cond;
  StmtBlock::Ptr body;
  
  typedef std::shared_ptr<WhileStmt> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<WhileStmt>());
  }
};

struct DoWhileStmt : public WhileStmt {
  typedef std::shared_ptr<DoWhileStmt> Ptr;
  
  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<DoWhileStmt>());
  }

  virtual unsigned getLineEnd() { return cond->getLineEnd(); }
  virtual unsigned getColEnd() { return cond->getColEnd(); }
};

struct IfStmt : public Stmt {
  Expr::Ptr cond;
  Stmt::Ptr ifBody;
  Stmt::Ptr elseBody;
  
  typedef std::shared_ptr<IfStmt> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<IfStmt>());
  }
};

struct ForDomain : public HIRNode {
  typedef std::shared_ptr<ForDomain> Ptr;
};

struct IndexSetDomain : public ForDomain {
  SetIndexSet::Ptr set;

  typedef std::shared_ptr<IndexSetDomain> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<IndexSetDomain>());
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
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<RangeDomain>());
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
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ForStmt>());
  }
};

struct PrintStmt : public Stmt {
  std::vector<Expr::Ptr> args;
  bool                   printNewline;
  
  typedef std::shared_ptr<PrintStmt> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<PrintStmt>());
  }
};

struct ExprStmt : public Stmt {
  Expr::Ptr expr;
  
  typedef std::shared_ptr<ExprStmt> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ExprStmt>());
  }

  virtual unsigned getLineBegin() { return expr->getLineBegin(); }
  virtual unsigned getColBegin() { return expr->getColBegin(); }
};

struct AssignStmt : public ExprStmt {
  std::vector<Expr::Ptr> lhs;
 
  typedef std::shared_ptr<AssignStmt> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<AssignStmt>());
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

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<Slice>());
  }

  virtual bool isSlice() { return true; }
};

struct ExprParam : public ReadParam {
  Expr::Ptr expr;
  
  typedef std::shared_ptr<ExprParam> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ExprParam>());
  }
  
  virtual unsigned getLineBegin() { return expr->getLineBegin(); }
  virtual unsigned getColBegin() { return expr->getColBegin(); }
  virtual unsigned getLineEnd() { return expr->getLineEnd(); }
  virtual unsigned getColEnd() { return expr->getColEnd(); }
};

struct MapExpr : public Expr {
  Identifier::Ptr        func;
  std::vector<Expr::Ptr> partialActuals;
  Identifier::Ptr        target;

  typedef std::shared_ptr<MapExpr> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<MapExpr>());
  }

  virtual bool isReduced() = 0;
};

struct ReducedMapExpr : public MapExpr {
  enum class ReductionOp {SUM};
  
  ReductionOp op;

  typedef std::shared_ptr<ReducedMapExpr> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ReducedMapExpr>());
  }

  virtual bool isReduced() { return true; }
};

struct UnreducedMapExpr : public MapExpr {
  typedef std::shared_ptr<UnreducedMapExpr> Ptr;

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<UnreducedMapExpr>());
  }
  
  virtual unsigned getLineEnd() { return target->getLineEnd(); }
  virtual unsigned getColEnd() { return target->getColEnd(); }

  virtual bool isReduced() { return false; }
};


struct UnaryExpr : public Expr {
  Expr::Ptr operand;

  typedef std::shared_ptr<UnaryExpr> Ptr;
  
  virtual void copy(HIRNode::Ptr);
};

struct BinaryExpr : public Expr {
  Expr::Ptr lhs;
  Expr::Ptr rhs;

  typedef std::shared_ptr<BinaryExpr> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual unsigned getLineBegin() { return lhs->getLineBegin(); }
  virtual unsigned getColBegin() { return lhs->getColBegin(); }
  virtual unsigned getLineEnd() { return rhs->getLineEnd(); }
  virtual unsigned getColEnd() { return rhs->getColEnd(); }
};

struct NaryExpr : public Expr {
  std::vector<Expr::Ptr> operands;

  typedef std::shared_ptr<NaryExpr> Ptr;
  
  virtual void copy(HIRNode::Ptr);
};

struct OrExpr : public BinaryExpr {
  typedef std::shared_ptr<OrExpr> Ptr;

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<OrExpr>());
  }
};

struct AndExpr : public BinaryExpr {
  typedef std::shared_ptr<AndExpr> Ptr;

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<AndExpr>());
  }
};

struct XorExpr : public BinaryExpr {
  typedef std::shared_ptr<XorExpr> Ptr;

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<XorExpr>());
  }
};

struct EqExpr : public NaryExpr {
  enum class Op {LT, LE, GT, GE, EQ, NE};

  std::vector<Op> ops;
  
  typedef std::shared_ptr<EqExpr> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<EqExpr>());
  }
  
  virtual unsigned getLineBegin() { return operands.front()->getLineBegin(); }
  virtual unsigned getColBegin() { return operands.front()->getColBegin(); }
  virtual unsigned getLineEnd() { return operands.back()->getLineEnd(); }
  virtual unsigned getColEnd() { return operands.back()->getColEnd(); }
};

struct NotExpr : public UnaryExpr {
  typedef std::shared_ptr<NotExpr> Ptr;

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<NotExpr>());
  }

  virtual unsigned getLineEnd() { return operand->getLineEnd(); }
  virtual unsigned getColEnd() { return operand->getColEnd(); }
};

struct AddExpr : public BinaryExpr {
  typedef std::shared_ptr<AddExpr> Ptr;

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<AddExpr>());
  }
};

struct SubExpr : public BinaryExpr {
  typedef std::shared_ptr<SubExpr> Ptr;

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<SubExpr>());
  }
};

struct MulExpr : public BinaryExpr {
  typedef std::shared_ptr<MulExpr> Ptr;

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<MulExpr>());
  }
};

struct DivExpr : public BinaryExpr {
  typedef std::shared_ptr<DivExpr> Ptr;

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<DivExpr>());
  }
};

struct ElwiseMulExpr : public BinaryExpr {
  typedef std::shared_ptr<ElwiseMulExpr> Ptr;

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ElwiseMulExpr>());
  }
};

struct ElwiseDivExpr : public BinaryExpr {
  typedef std::shared_ptr<ElwiseDivExpr> Ptr;

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ElwiseDivExpr>());
  }
};

struct LeftDivExpr : public BinaryExpr {
  typedef std::shared_ptr<LeftDivExpr> Ptr;

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(to<LeftDivExpr>(shared_from_this()));
  }
};

struct NegExpr : public UnaryExpr {
  bool negate;
  
  typedef std::shared_ptr<NegExpr> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<NegExpr>());
  }

  virtual unsigned getLineEnd() { return operand->getLineEnd(); }
  virtual unsigned getColEnd() { return operand->getColEnd(); }
};

struct ExpExpr : public BinaryExpr {
  typedef std::shared_ptr<ExpExpr> Ptr;

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ExpExpr>());
  }
};

struct TransposeExpr : public UnaryExpr {
  typedef std::shared_ptr<TransposeExpr> Ptr;

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<TransposeExpr>());
  }

  virtual unsigned getLineBegin() { return operand->getLineBegin(); }
  virtual unsigned getColBegin() { return operand->getColBegin(); }
};

struct CallExpr : public Expr {
  Identifier::Ptr        func;
  std::vector<Expr::Ptr> args;
  
  typedef std::shared_ptr<CallExpr> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<CallExpr>());
  }

  virtual unsigned getLineBegin() { return func->getLineBegin(); }
  virtual unsigned getColBegin() { return func->getColBegin(); }
};

struct TensorReadExpr : public Expr {
  Expr::Ptr                   tensor;
  std::vector<ReadParam::Ptr> indices;
  
  typedef std::shared_ptr<TensorReadExpr> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<TensorReadExpr>());
  }
  
  virtual unsigned getLineBegin() { return tensor->getLineBegin(); }
  virtual unsigned getColBegin() { return tensor->getColBegin(); }
};

struct TupleReadExpr : public Expr {
  Expr::Ptr tuple;
  Expr::Ptr index;

  typedef std::shared_ptr<TupleReadExpr> Ptr;

  virtual void copy(HIRNode::Ptr);

  template <typename T> std::shared_ptr<T> cloneImpl2() { return to<T>(cloneImpl()); }
  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<TupleReadExpr>());
  }

  virtual unsigned getLineBegin() { return tuple->getLineBegin(); }
  virtual unsigned getColBegin() { return tuple->getColBegin(); }
};

struct FieldReadExpr : public Expr {
  Expr::Ptr       setOrElem;
  Identifier::Ptr field;
  
  typedef std::shared_ptr<FieldReadExpr> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<FieldReadExpr>());
  }
  
  virtual unsigned getLineBegin() { return setOrElem->getLineBegin(); }
  virtual unsigned getColBegin() { return setOrElem->getColBegin(); }
  virtual unsigned getLineEnd() { return field->getLineEnd(); }
  virtual unsigned getColEnd() { return field->getColEnd(); }
};

struct ParenExpr : public Expr {
  Expr::Ptr expr;

  typedef std::shared_ptr<ParenExpr> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ParenExpr>());
  }
};

struct VarExpr : public Expr {
  std::string ident;
  
  typedef std::shared_ptr<VarExpr> Ptr;
 
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<VarExpr>());
  }
};

struct TensorLiteral : public Expr {
  typedef std::shared_ptr<TensorLiteral> Ptr;
};

struct IntLiteral : public TensorLiteral {
  int val;

  typedef std::shared_ptr<IntLiteral> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<IntLiteral>());
  }
};

struct FloatLiteral : public TensorLiteral {
  double val;
  
  typedef std::shared_ptr<FloatLiteral> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<FloatLiteral>());
  }
};

struct BoolLiteral : public TensorLiteral {
  bool val;
  
  typedef std::shared_ptr<BoolLiteral> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<BoolLiteral>());
  }
};

struct ComplexLiteral : public TensorLiteral {
  double_complex val;

  typedef std::shared_ptr<ComplexLiteral> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ComplexLiteral>());
  }
};

struct StringLiteral : public TensorLiteral {
  std::string val;
  
  typedef std::shared_ptr<StringLiteral> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<StringLiteral>());
  }
};

struct DenseTensorLiteral : public TensorLiteral {
  bool transposed;

  typedef std::shared_ptr<DenseTensorLiteral> Ptr;
  
  virtual void copy(HIRNode::Ptr);
};

struct IntVectorLiteral : public DenseTensorLiteral {
  std::vector<int> vals;
  
  typedef std::shared_ptr<IntVectorLiteral> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<IntVectorLiteral>());
  }
};

struct FloatVectorLiteral : public DenseTensorLiteral {
  std::vector<double> vals;
  
  typedef std::shared_ptr<FloatVectorLiteral> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<FloatVectorLiteral>());
  }
};

struct ComplexVectorLiteral : public DenseTensorLiteral {
  std::vector<double_complex> vals;

  typedef std::shared_ptr<ComplexVectorLiteral> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ComplexVectorLiteral>());
  }
};

struct NDTensorLiteral : public DenseTensorLiteral {
  std::vector<DenseTensorLiteral::Ptr> elems;
  
  typedef std::shared_ptr<NDTensorLiteral> Ptr;

  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<NDTensorLiteral>());
  }
};

struct ApplyStmt : public Stmt {
  UnreducedMapExpr::Ptr map;
  
  typedef std::shared_ptr<ApplyStmt> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<ApplyStmt>());
  }

  virtual unsigned getLineBegin() { return map->getLineBegin(); }
  virtual unsigned getColBegin() { return map->getColBegin(); }
};

struct Test : public HIRNode {
  Identifier::Ptr        func;
  std::vector<Expr::Ptr> args;
  Expr::Ptr              expected;
  
  typedef std::shared_ptr<Test> Ptr;
  
  virtual void copy(HIRNode::Ptr);

  virtual HIRNode::Ptr cloneImpl();

  virtual void accept(HIRVisitor *visitor) {
    visitor->visit(self<Test>());
  }
};

}
}

#endif

