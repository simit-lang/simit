#ifndef SIMIT_HIR_PRINTER_H
#define SIMIT_HIR_PRINTER_H

#include <iostream>

#include "hir.h"
#include "hir_visitor.h"

namespace simit {
namespace hir {

struct HIRPrinter : public HIRVisitor {
  HIRPrinter(std::ostream &oss) : oss(oss), indentLevel(0) {}

protected:
  virtual void visit(Program::Ptr);
  virtual void visit(StmtBlock::Ptr);
  virtual void visit(RangeIndexSet::Ptr);
  virtual void visit(SetIndexSet::Ptr);
  virtual void visit(DynamicIndexSet::Ptr);
  virtual void visit(ElementType::Ptr);
  virtual void visit(Endpoint::Ptr);
  virtual void visit(UnstructuredSetType::Ptr);
  virtual void visit(LatticeLinkSetType::Ptr);
  virtual void visit(TupleLength::Ptr);
  virtual void visit(TupleType::Ptr);
  virtual void visit(ScalarType::Ptr);
  virtual void visit(NDTensorType::Ptr);
  virtual void visit(Identifier::Ptr);
  virtual void visit(IdentDecl::Ptr);
  virtual void visit(FieldDecl::Ptr);
  virtual void visit(ElementTypeDecl::Ptr);
  virtual void visit(Argument::Ptr);
  virtual void visit(ExternDecl::Ptr);
  virtual void visit(GenericParam::Ptr);
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
  virtual void visit(Slice::Ptr);
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
  virtual void visit(TupleReadExpr::Ptr);
  virtual void visit(FieldReadExpr::Ptr);
  virtual void visit(VarExpr::Ptr);
  virtual void visit(IntLiteral::Ptr);
  virtual void visit(FloatLiteral::Ptr);
  virtual void visit(BoolLiteral::Ptr);
  virtual void visit(ComplexLiteral::Ptr);
  virtual void visit(IntVectorLiteral::Ptr);
  virtual void visit(FloatVectorLiteral::Ptr);
  virtual void visit(ComplexVectorLiteral::Ptr);
  virtual void visit(NDTensorLiteral::Ptr);
  virtual void visit(ApplyStmt::Ptr);
  virtual void visit(Test::Ptr);

  void indent() { ++indentLevel; }
  void dedent() { --indentLevel; }
  void printIndent() { oss << std::string(2 * indentLevel, ' '); }
  void printBoolean(bool val) { oss << (val ? "true" : "false"); }
  void printComplex(double_complex val) {
    oss << "<" << val.real << "," << val.imag << ">";
  }
 
  void printIdentDecl(IdentDecl::Ptr);
  void printVarOrConstDecl(VarDecl::Ptr, const bool = false);
  void printMapOrApply(MapExpr::Ptr, const bool = false);
  void printUnaryExpr(UnaryExpr::Ptr, const std::string, const bool = false);
  void printBinaryExpr(BinaryExpr::Ptr, const std::string);

  std::ostream &oss;
  unsigned      indentLevel;
};

}
}

#endif

