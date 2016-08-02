#ifndef SIMIT_INFER_ELEMENT_SOURCES_H
#define SIMIT_INFER_ELEMENT_SOURCES_H

#include "fir.h"
#include "fir_visitor.h"
#include "util/scopedmap.h"

namespace simit {
namespace fir {

class InferElementSources : public FIRVisitor {
public:
  void infer(Program::Ptr program) { program->accept(this); }

private:
  virtual void visit(IdentDecl::Ptr);
  virtual void visit(FieldDecl::Ptr op) {}
  virtual void visit(ExternDecl::Ptr op) {}
  virtual void visit(FuncDecl::Ptr);
  virtual void visit(VarDecl::Ptr);
  virtual void visit(WhileStmt::Ptr);
  virtual void visit(IfStmt::Ptr);
  virtual void visit(ForStmt::Ptr);
  virtual void visit(AssignStmt::Ptr);
  virtual void visit(TensorReadExpr::Ptr);
  
  void reportError(std::string, FIRNode::Ptr);

private:
  typedef util::ScopedMap<std::string, Type::Ptr> SymbolTable;

  SymbolTable decls;
};

}
}

#endif

