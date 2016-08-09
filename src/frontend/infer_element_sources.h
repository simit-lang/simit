#ifndef SIMIT_INFER_ELEMENT_SOURCES_H
#define SIMIT_INFER_ELEMENT_SOURCES_H

#include "fir.h"
#include "fir_visitor.h"
#include "util/scopedmap.h"

namespace simit {
namespace fir {

// As the name of the class suggests, the purpose of this analysis is to infer, 
// based on how elements are used to index into tensor function arguments and 
// result variables, what the source of an element can/must be. As an example, 
// for the following example, the analysis would infer that p must actually be 
// an element of the `points` set (rather than just any arbitrary Point) and 
// annotate `p` (or more precisely the type of `p`) appropriately.
//
// func f(p : Point) -> (M : matrix[points,points](float))
//   M(p,p) = 1.0;
// end

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

