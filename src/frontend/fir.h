#ifndef SIMIT_FIR_H
#define SIMIT_FIR_H

#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <unordered_set>

#include "types.h"
#include "scanner.h"
#include "fir_visitor.h"
#include "program_context.h"

namespace simit {
namespace fir {

struct FIRNode;
struct SetType;

template <typename T>
inline bool isa(std::shared_ptr<FIRNode> ptr) {
  return (bool)std::dynamic_pointer_cast<T>(ptr);
}

template <typename T>
inline const std::shared_ptr<T> to(std::shared_ptr<FIRNode> ptr) {
  std::shared_ptr<T> ret = std::dynamic_pointer_cast<T>(ptr);
  iassert((bool)ret);
  return ret;
}

// Base class for front-end intermediate representation.
struct FIRNode : public std::enable_shared_from_this<FIRNode> {
  typedef std::shared_ptr<FIRNode> Ptr;

  FIRNode() : lineBegin(0), colBegin(0), lineEnd(0), colEnd(0) {}

  template <typename T = FIRNode> std::shared_ptr<T> clone() {
    return to<T>(cloneNode());
  }

  virtual void accept(FIRVisitor *) = 0; 

  virtual unsigned getLineBegin() { return lineBegin; }
  virtual unsigned getColBegin() { return colBegin; }
  virtual unsigned getLineEnd() { return lineEnd; }
  virtual unsigned getColEnd() { return colEnd; }

  void setBeginLoc(const internal::Token &);
  void setEndLoc(const internal::Token &);
  void setLoc(const internal::Token &);

  friend std::ostream &operator<<(std::ostream &, FIRNode &);

protected:
  template <typename T = FIRNode> std::shared_ptr<T> self() {
    return to<T>(shared_from_this());
  }

  virtual void copy(FIRNode::Ptr);
  
  virtual FIRNode::Ptr cloneNode() = 0;

private:
  unsigned lineBegin;
  unsigned colBegin;
  unsigned lineEnd;
  unsigned colEnd;
};

struct Program : public FIRNode {
  std::vector<FIRNode::Ptr> elems;

  typedef std::shared_ptr<Program> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<Program>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct Stmt : public FIRNode {
  typedef std::shared_ptr<Stmt> Ptr;
};

struct StmtBlock : public Stmt {
  std::vector<Stmt::Ptr> stmts;
  
  typedef std::shared_ptr<StmtBlock> Ptr;

  virtual void accept(FIRVisitor *visitor) { 
    visitor->visit(self<StmtBlock>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct Type : public FIRNode {
  typedef std::shared_ptr<Type> Ptr;
};

struct Expr : public FIRNode {
  typedef std::shared_ptr<Expr> Ptr;
};

struct IndexSet : public FIRNode {
  typedef std::shared_ptr<IndexSet> Ptr;
};

struct RangeIndexSet : public IndexSet {
  int range;
  
  typedef std::shared_ptr<RangeIndexSet> Ptr;
  
  virtual void accept(FIRVisitor *visitor) { 
    visitor->visit(self<RangeIndexSet>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct SetIndexSet : public IndexSet {
  std::string              setName;
  std::shared_ptr<SetType> setDef; // Reference to original definition of set.
  
  typedef std::shared_ptr<SetIndexSet> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<SetIndexSet>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct GenericIndexSet : public SetIndexSet {
  enum class Type {UNKNOWN, RANGE};

  Type type;
  
  typedef std::shared_ptr<GenericIndexSet> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<GenericIndexSet>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct DynamicIndexSet : public IndexSet {
  typedef std::shared_ptr<DynamicIndexSet> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<DynamicIndexSet>());
  }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct ElementType : public Type {
  std::string      ident;
  SetIndexSet::Ptr source; // Reference to inferred source index set.
  
  typedef std::shared_ptr<ElementType> Ptr;
 
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ElementType>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct Endpoint : public FIRNode {
  SetIndexSet::Ptr set;
  ElementType::Ptr element;
  
  typedef std::shared_ptr<Endpoint> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<Endpoint>());
  }

  virtual unsigned getLineBegin() { return set->getLineBegin(); }
  virtual unsigned getColBegin() { return set->getColBegin(); }
  virtual unsigned getLineEnd() { return set->getLineEnd(); }
  virtual unsigned getColEnd() { return set->getColEnd(); }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct SetType : public Type {
  ElementType::Ptr element;

  typedef std::shared_ptr<SetType> Ptr;

  static Ptr getUndefinedSetType();
};

struct UnstructuredSetType : public SetType {
  std::vector<Endpoint::Ptr> endpoints;

  typedef std::shared_ptr<UnstructuredSetType> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<UnstructuredSetType>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode();
};

struct LatticeLinkSetType : public SetType {
  Endpoint::Ptr latticePointSet;
  size_t        dimensions;
  
  typedef std::shared_ptr<LatticeLinkSetType> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<LatticeLinkSetType>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct TupleLength : public FIRNode {
  int val;

  typedef std::shared_ptr<TupleLength> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<TupleLength>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct TupleType : public Type {
  ElementType::Ptr element;
  TupleLength::Ptr length;
  
  typedef std::shared_ptr<TupleType> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<TupleType>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct TensorType : public Type {
  typedef std::shared_ptr<TensorType> Ptr;
};

struct ScalarType : public TensorType {
  enum class Type {INT, FLOAT, BOOL, COMPLEX, STRING};

  Type type;
  
  typedef std::shared_ptr<ScalarType> Ptr;
 
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ScalarType>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct NDTensorType : public TensorType {
  std::vector<IndexSet::Ptr> indexSets;
  TensorType::Ptr            blockType;
  bool                       transposed;
  
  typedef std::shared_ptr<NDTensorType> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<NDTensorType>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct Identifier : public FIRNode {
  std::string ident;

  typedef std::shared_ptr<Identifier> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<Identifier>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct IdentDecl : public FIRNode {
  Identifier::Ptr name;
  Type::Ptr       type;
  
  typedef std::shared_ptr<IdentDecl> Ptr;
 
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<IdentDecl>());
  }
  
  virtual unsigned getLineBegin() { return name->getLineBegin(); }
  virtual unsigned getColBegin() { return name->getColBegin(); }
  virtual unsigned getLineEnd() { return type->getLineEnd(); }
  virtual unsigned getColEnd() { return type->getColEnd(); }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct FieldDecl : public IdentDecl {
  typedef std::shared_ptr<FieldDecl> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<FieldDecl>());
  }

  virtual unsigned getLineEnd() { return FIRNode::getLineEnd(); }
  virtual unsigned getColEnd() { return FIRNode::getColEnd(); }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct ElementTypeDecl : public FIRNode {
  Identifier::Ptr             name;
  std::vector<FieldDecl::Ptr> fields; 
  
  typedef std::shared_ptr<ElementTypeDecl> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ElementTypeDecl>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct Argument : public IdentDecl {
  typedef std::shared_ptr<Argument> Ptr;
 
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<Argument>());
  }
  
  virtual bool isInOut() { return false; }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct InOutArgument : public Argument {
  typedef std::shared_ptr<InOutArgument> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<InOutArgument>());
  }
  
  virtual unsigned getLineBegin() { return FIRNode::getLineBegin(); }
  virtual unsigned getColBegin() { return FIRNode::getColBegin(); }

  virtual bool isInOut() { return true; }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct ExternDecl : public IdentDecl {
  typedef std::shared_ptr<ExternDecl> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ExternDecl>());
  }
  
  virtual unsigned getLineBegin() { return FIRNode::getLineBegin(); }
  virtual unsigned getColBegin() { return FIRNode::getColBegin(); }
  virtual unsigned getLineEnd() { return FIRNode::getLineEnd(); }
  virtual unsigned getColEnd() { return FIRNode::getColEnd(); }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct GenericParam : public FIRNode {
  enum class Type {UNKNOWN, RANGE};

  std::string name;
  Type        type;

  typedef std::shared_ptr<GenericParam> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<GenericParam>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct FuncDecl : public FIRNode {
  enum class Type {INTERNAL, EXPORTED, EXTERNAL};

  Identifier::Ptr                name;
  std::vector<GenericParam::Ptr> genericParams;
  std::vector<Argument::Ptr>     args;
  std::vector<IdentDecl::Ptr>    results;
  StmtBlock::Ptr                 body;
  Type                           type;
  std::string                    originalName;
  
  typedef std::shared_ptr<FuncDecl> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<FuncDecl>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct VarDecl : public Stmt {
  Identifier::Ptr name;
  Type::Ptr       type;
  Expr::Ptr       initVal;

  typedef std::shared_ptr<VarDecl> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<VarDecl>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct ConstDecl : public VarDecl {
  typedef std::shared_ptr<ConstDecl> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ConstDecl>());
  }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct WhileStmt : public Stmt {
  Expr::Ptr      cond;
  StmtBlock::Ptr body;
  
  typedef std::shared_ptr<WhileStmt> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<WhileStmt>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct DoWhileStmt : public WhileStmt {
  typedef std::shared_ptr<DoWhileStmt> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<DoWhileStmt>());
  }

  virtual unsigned getLineEnd() { return cond->getLineEnd(); }
  virtual unsigned getColEnd() { return cond->getColEnd(); }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct IfStmt : public Stmt {
  Expr::Ptr cond;
  Stmt::Ptr ifBody;
  Stmt::Ptr elseBody;
  
  typedef std::shared_ptr<IfStmt> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<IfStmt>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct ForDomain : public FIRNode {
  typedef std::shared_ptr<ForDomain> Ptr;
};

struct IndexSetDomain : public ForDomain {
  SetIndexSet::Ptr set;

  typedef std::shared_ptr<IndexSetDomain> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<IndexSetDomain>());
  }

  virtual unsigned getLineBegin() { return set->getLineBegin(); }
  virtual unsigned getColBegin() { return set->getColBegin(); }
  virtual unsigned getLineEnd() { return set->getLineEnd(); }
  virtual unsigned getColEnd() { return set->getColEnd(); }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct RangeDomain : public ForDomain {
  Expr::Ptr lower;
  Expr::Ptr upper;

  typedef std::shared_ptr<RangeDomain> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<RangeDomain>());
  }
  
  virtual unsigned getLineBegin() { return lower->getLineBegin(); }
  virtual unsigned getColBegin() { return lower->getColBegin(); }
  virtual unsigned getLineEnd() { return upper->getLineEnd(); }
  virtual unsigned getColEnd() { return upper->getColEnd(); }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct ForStmt : public Stmt {
  Identifier::Ptr loopVar;
  ForDomain::Ptr  domain;
  StmtBlock::Ptr  body;
  
  typedef std::shared_ptr<ForStmt> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ForStmt>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct PrintStmt : public Stmt {
  std::vector<Expr::Ptr> args;
  bool                   printNewline;
  
  typedef std::shared_ptr<PrintStmt> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<PrintStmt>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct ExprStmt : public Stmt {
  Expr::Ptr expr;
  
  typedef std::shared_ptr<ExprStmt> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ExprStmt>());
  }

  virtual unsigned getLineBegin() { return expr->getLineBegin(); }
  virtual unsigned getColBegin() { return expr->getColBegin(); }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct AssignStmt : public ExprStmt {
  std::vector<Expr::Ptr> lhs;
 
  typedef std::shared_ptr<AssignStmt> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<AssignStmt>());
  }

  virtual unsigned getLineBegin() { return lhs.front()->getLineBegin(); }
  virtual unsigned getColBegin() { return lhs.front()->getColBegin(); }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct ReadParam : public FIRNode {
  typedef std::shared_ptr<ReadParam> Ptr;

  virtual bool isSlice() { return false; }
};

struct Slice : public ReadParam {
  typedef std::shared_ptr<Slice> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<Slice>());
  }

  virtual bool isSlice() { return true; }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct ExprParam : public ReadParam {
  Expr::Ptr expr;
  
  typedef std::shared_ptr<ExprParam> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ExprParam>());
  }
  
  virtual unsigned getLineBegin() { return expr->getLineBegin(); }
  virtual unsigned getColBegin() { return expr->getColBegin(); }
  virtual unsigned getLineEnd() { return expr->getLineEnd(); }
  virtual unsigned getColEnd() { return expr->getColEnd(); }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct MapExpr : public Expr {
  enum class ReductionOp {NONE, SUM};
  
  Identifier::Ptr            func;
  std::vector<IndexSet::Ptr> genericArgs;
  std::vector<Expr::Ptr>     partialActuals;
  SetIndexSet::Ptr           target;
  SetIndexSet::Ptr           through;

  typedef std::shared_ptr<MapExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<MapExpr>());
  }

  virtual ReductionOp getReductionOp() = 0;

protected:
  virtual void copy(FIRNode::Ptr);
};

struct ReducedMapExpr : public MapExpr {
  ReductionOp op;

  typedef std::shared_ptr<ReducedMapExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ReducedMapExpr>());
  }

  virtual ReductionOp getReductionOp() { return op; }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct UnreducedMapExpr : public MapExpr {
  typedef std::shared_ptr<UnreducedMapExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<UnreducedMapExpr>());
  }
  
  virtual unsigned getLineEnd() { return target->getLineEnd(); }
  virtual unsigned getColEnd() { return target->getColEnd(); }

  virtual ReductionOp getReductionOp() { return ReductionOp::NONE; }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};


struct UnaryExpr : public Expr {
  Expr::Ptr operand;

  typedef std::shared_ptr<UnaryExpr> Ptr;
  
protected:
  virtual void copy(FIRNode::Ptr);
};

struct BinaryExpr : public Expr {
  Expr::Ptr lhs;
  Expr::Ptr rhs;

  typedef std::shared_ptr<BinaryExpr> Ptr;
  
  virtual unsigned getLineBegin() { return lhs->getLineBegin(); }
  virtual unsigned getColBegin() { return lhs->getColBegin(); }
  virtual unsigned getLineEnd() { return rhs->getLineEnd(); }
  virtual unsigned getColEnd() { return rhs->getColEnd(); }

protected:
  virtual void copy(FIRNode::Ptr);
};

struct NaryExpr : public Expr {
  std::vector<Expr::Ptr> operands;

  typedef std::shared_ptr<NaryExpr> Ptr;
  
protected:
  virtual void copy(FIRNode::Ptr);
};

struct OrExpr : public BinaryExpr {
  typedef std::shared_ptr<OrExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<OrExpr>());
  }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct AndExpr : public BinaryExpr {
  typedef std::shared_ptr<AndExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<AndExpr>());
  }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct XorExpr : public BinaryExpr {
  typedef std::shared_ptr<XorExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<XorExpr>());
  }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct EqExpr : public NaryExpr {
  enum class Op {LT, LE, GT, GE, EQ, NE};

  std::vector<Op> ops;
  
  typedef std::shared_ptr<EqExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<EqExpr>());
  }
  
  virtual unsigned getLineBegin() { return operands.front()->getLineBegin(); }
  virtual unsigned getColBegin() { return operands.front()->getColBegin(); }
  virtual unsigned getLineEnd() { return operands.back()->getLineEnd(); }
  virtual unsigned getColEnd() { return operands.back()->getColEnd(); }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct NotExpr : public UnaryExpr {
  typedef std::shared_ptr<NotExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<NotExpr>());
  }

  virtual unsigned getLineEnd() { return operand->getLineEnd(); }
  virtual unsigned getColEnd() { return operand->getColEnd(); }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct AddExpr : public BinaryExpr {
  typedef std::shared_ptr<AddExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<AddExpr>());
  }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct SubExpr : public BinaryExpr {
  typedef std::shared_ptr<SubExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<SubExpr>());
  }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct MulExpr : public BinaryExpr {
  typedef std::shared_ptr<MulExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<MulExpr>());
  }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct DivExpr : public BinaryExpr {
  typedef std::shared_ptr<DivExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<DivExpr>());
  }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct ElwiseMulExpr : public BinaryExpr {
  typedef std::shared_ptr<ElwiseMulExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ElwiseMulExpr>());
  }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct ElwiseDivExpr : public BinaryExpr {
  typedef std::shared_ptr<ElwiseDivExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ElwiseDivExpr>());
  }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct LeftDivExpr : public BinaryExpr {
  typedef std::shared_ptr<LeftDivExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<LeftDivExpr>());
  }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct NegExpr : public UnaryExpr {
  bool negate;
  
  typedef std::shared_ptr<NegExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<NegExpr>());
  }

  virtual unsigned getLineEnd() { return operand->getLineEnd(); }
  virtual unsigned getColEnd() { return operand->getColEnd(); }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct ExpExpr : public BinaryExpr {
  typedef std::shared_ptr<ExpExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ExpExpr>());
  }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct TransposeExpr : public UnaryExpr {
  typedef std::shared_ptr<TransposeExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<TransposeExpr>());
  }

  virtual unsigned getLineBegin() { return operand->getLineBegin(); }
  virtual unsigned getColBegin() { return operand->getColBegin(); }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct CallExpr : public Expr {
  Identifier::Ptr            func;
  std::vector<IndexSet::Ptr> genericArgs;
  std::vector<Expr::Ptr>     args;
  
  typedef std::shared_ptr<CallExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<CallExpr>());
  }

  virtual unsigned getLineBegin() { return func->getLineBegin(); }
  virtual unsigned getColBegin() { return func->getColBegin(); }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct TensorReadExpr : public Expr {
  Expr::Ptr                   tensor;
  std::vector<ReadParam::Ptr> indices;
  
  typedef std::shared_ptr<TensorReadExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<TensorReadExpr>());
  }
  
  virtual unsigned getLineBegin() { return tensor->getLineBegin(); }
  virtual unsigned getColBegin() { return tensor->getColBegin(); }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct SetReadExpr : public Expr {
  Expr::Ptr              set;
  std::vector<Expr::Ptr> indices;

  typedef std::shared_ptr<SetReadExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<SetReadExpr>());
  }

  virtual unsigned getLineBegin() { return set->getLineBegin(); }
  virtual unsigned getColBegin() { return set->getColBegin(); }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode();
};

struct TupleReadExpr : public Expr {
  Expr::Ptr tuple;
  Expr::Ptr index;

  typedef std::shared_ptr<TupleReadExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<TupleReadExpr>());
  }

  virtual unsigned getLineBegin() { return tuple->getLineBegin(); }
  virtual unsigned getColBegin() { return tuple->getColBegin(); }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct FieldReadExpr : public Expr {
  Expr::Ptr       setOrElem;
  Identifier::Ptr field;
  
  typedef std::shared_ptr<FieldReadExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<FieldReadExpr>());
  }
  
  virtual unsigned getLineBegin() { return setOrElem->getLineBegin(); }
  virtual unsigned getColBegin() { return setOrElem->getColBegin(); }
  virtual unsigned getLineEnd() { return field->getLineEnd(); }
  virtual unsigned getColEnd() { return field->getColEnd(); }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct ParenExpr : public Expr {
  Expr::Ptr expr;

  typedef std::shared_ptr<ParenExpr> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ParenExpr>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct VarExpr : public Expr {
  std::string ident;
  
  typedef std::shared_ptr<VarExpr> Ptr;
 
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<VarExpr>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct RangeConst : public VarExpr {
  typedef std::shared_ptr<RangeConst> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<RangeConst>());
  }

protected:
  virtual FIRNode::Ptr cloneNode(); 
};

struct TensorLiteral : public Expr {
  typedef std::shared_ptr<TensorLiteral> Ptr;
};

struct IntLiteral : public TensorLiteral {
  int val;

  typedef std::shared_ptr<IntLiteral> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<IntLiteral>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct FloatLiteral : public TensorLiteral {
  double val;
  
  typedef std::shared_ptr<FloatLiteral> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<FloatLiteral>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct BoolLiteral : public TensorLiteral {
  bool val;
  
  typedef std::shared_ptr<BoolLiteral> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<BoolLiteral>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct ComplexLiteral : public TensorLiteral {
  double_complex val;

  typedef std::shared_ptr<ComplexLiteral> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ComplexLiteral>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct StringLiteral : public TensorLiteral {
  std::string val;
  
  typedef std::shared_ptr<StringLiteral> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<StringLiteral>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct DenseTensorLiteral : public TensorLiteral {
  bool transposed;

  typedef std::shared_ptr<DenseTensorLiteral> Ptr;
  
protected:
  virtual void copy(FIRNode::Ptr);
};

struct IntVectorLiteral : public DenseTensorLiteral {
  std::vector<int> vals;
  
  typedef std::shared_ptr<IntVectorLiteral> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<IntVectorLiteral>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct FloatVectorLiteral : public DenseTensorLiteral {
  std::vector<double> vals;
  
  typedef std::shared_ptr<FloatVectorLiteral> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<FloatVectorLiteral>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct ComplexVectorLiteral : public DenseTensorLiteral {
  std::vector<double_complex> vals;

  typedef std::shared_ptr<ComplexVectorLiteral> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ComplexVectorLiteral>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct NDTensorLiteral : public DenseTensorLiteral {
  std::vector<DenseTensorLiteral::Ptr> elems;
  
  typedef std::shared_ptr<NDTensorLiteral> Ptr;

  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<NDTensorLiteral>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct ApplyStmt : public Stmt {
  UnreducedMapExpr::Ptr map;
  
  typedef std::shared_ptr<ApplyStmt> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<ApplyStmt>());
  }

  virtual unsigned getLineBegin() { return map->getLineBegin(); }
  virtual unsigned getColBegin() { return map->getColBegin(); }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};

struct Test : public FIRNode {
  Identifier::Ptr        func;
  std::vector<Expr::Ptr> args;
  Expr::Ptr              expected;
  
  typedef std::shared_ptr<Test> Ptr;
  
  virtual void accept(FIRVisitor *visitor) {
    visitor->visit(self<Test>());
  }

protected:
  virtual void copy(FIRNode::Ptr);

  virtual FIRNode::Ptr cloneNode(); 
};


// Utility functions
typedef std::vector<IndexSet::Ptr> IndexDomain;
typedef std::vector<IndexDomain>   TensorDimensions;

TensorType::Ptr
makeTensorType(ScalarType::Type componentType,
               const TensorDimensions &dimensions = TensorDimensions(),
               bool transposed = false);
}
}

#endif

