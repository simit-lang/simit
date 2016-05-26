#ifndef SIMIT_TYPE_CHECKER_H
#define SIMIT_TYPE_CHECKER_H

#include <vector>
#include <memory>
#include <string>
#include <exception>
#include <set>

#include "hir.h"
#include "hir_visitor.h"
#include "types.h"
#include "domain.h"
#include "program_context.h"
#include "error.h"
#include "ir.h"

namespace simit {
namespace hir {

// Type checking pass for identifying type errors, redefinitions, and 
// undeclared identifiers.
class TypeChecker : public HIRVisitor {
public:
  TypeChecker(std::vector<ParseError> *errors) : 
    retField(ir::Field("", ir::Type())), 
    skipCheckDeclared(false),
    errors(errors) {}

  void check(Program::Ptr program) { program->accept(this); }

private:
  virtual void visit(Program::Ptr);
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
  virtual void visit(Field::Ptr);
  virtual void visit(ElementTypeDecl::Ptr);
  virtual void visit(ExternDecl::Ptr);
  virtual void visit(FuncDecl::Ptr);
  virtual void visit(VarDecl::Ptr);
  virtual void visit(ConstDecl::Ptr);
  virtual void visit(WhileStmt::Ptr);
  virtual void visit(IfStmt::Ptr);
  virtual void visit(RangeDomain::Ptr);
  virtual void visit(ForStmt::Ptr);
  virtual void visit(PrintStmt::Ptr);
  virtual void visit(AssignStmt::Ptr);
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
  virtual void visit(SetReadExpr::Ptr);
  virtual void visit(TupleReadExpr::Ptr);
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

private:
  template <typename T> using Ptr = std::shared_ptr<T>;

  class DimError : public std::exception {
    const char *what() const noexcept { return "mismatched dimension sizes"; }
  };
  class TypeError : public std::exception {
    const char *what() const noexcept { return "mismatched element types"; }
  };

  struct DenseTensorType {
    enum class Type {UNKNOWN, INT, FLOAT, COMPLEX};

    DenseTensorType() : dimSizes(1), type(Type::UNKNOWN) {};

    void addDimension() { dimSizes.push_back(1); }
    void addIntValues(unsigned);
    void addFloatValues(unsigned);
    void addComplexValues(unsigned);
    void merge(const DenseTensorType &);

    std::vector<unsigned> dimSizes;
    Type                      type;
  };

private:
  void typeCheckVarOrConstDecl(VarDecl::Ptr, bool = false, bool = false);
  void typeCheckMapOrApply(MapExpr::Ptr, bool = false);
  void typeCheckBinaryElwise(BinaryExpr::Ptr, bool = false);
  void typeCheckBinaryBoolean(BinaryExpr::Ptr);
  void typeCheckDenseTensorLiteral(DenseTensorLiteral::Ptr);

  DenseTensorType getDenseTensorType(DenseTensorLiteral::Ptr);

  Ptr<Expr::Type> inferType(Expr::Ptr ptr) {
    retType.reset();
    ptr->accept(this);
    const Ptr<Expr::Type> ret = retType;
    retType.reset();
    if (ret) {
      ptr->type = *ret;
    }
    return ret;
  }
  Ptr<ir::IndexSet> getIndexSet(IndexSet::Ptr ptr) {
    retIndexSet.reset();
    ptr->accept(this);
    const Ptr<ir::IndexSet> ret = retIndexSet;
    retIndexSet.reset();
    return ret;
  }
  ir::Expr getExpr(Endpoint::Ptr ptr) {
    retExpr = ir::Expr();
    ptr->accept(this);
    const ir::Expr ret = retExpr;
    retExpr = ir::Expr();
    return ret;
  }
  ir::Type getIRType(hir::Type::Ptr ptr) {
    retIRType = ir::Type();
    ptr->accept(this);
    const ir::Type ret = retIRType;
    retIRType = ir::Type();
    return ret;
  }
  ir::Field getField(Field::Ptr ptr) {
    retField = ir::Field("", ir::Type());
    ptr->accept(this);
    const ir::Field ret = retField;
    retField = ir::Field("", ir::Type());
    return ret;
  }
  ir::Var getVar(IdentDecl::Ptr ptr) {
    retVar = ir::Var();
    ptr->accept(this);
    const ir::Var ret = retVar;
    retVar = ir::Var();
    return ret;
  }

  static bool compareTypes(const ir::Type &, const ir::Type &);
  
  static std::string typeString(const ir::Type &);
  static std::string typeString(const Ptr<Expr::Type> &);
  
  void reportError(std::string, HIRNode::Ptr);
  void reportUndeclared(std::string, std::string, HIRNode::Ptr);
  void reportMultipleDefs(std::string, std::string, HIRNode::Ptr);

private:
  Ptr<Expr::Type>   retType;
  Ptr<ir::IndexSet> retIndexSet;
  ir::Expr          retExpr;
  ir::Type          retIRType;
  ir::Field         retField;
  ir::Var           retVar;
 
  bool                     skipCheckDeclared;
  std::set<ir::Var>        writableArgs;
  internal::ProgramContext ctx;
  std::vector<ParseError> *errors;
};

}
}

#endif

