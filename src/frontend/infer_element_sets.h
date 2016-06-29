#ifndef SIMIT_INFER_ELEMENT_SETS_H
#define SIMIT_INFER_ELEMENT_SETS_H

#include <list>
#include <unordered_map>

#include "hir.h"
#include "hir_visitor.h"
#include "util/scopedmap.h"

namespace simit {
namespace hir {

class InferElementSets : public HIRVisitor {
public:
  InferElementSets(std::vector<ParseError> *errors) : errors(errors) {}
  
  void infer(Program::Ptr program) { program->accept(this); }

private:
  virtual void visit(IdentDecl::Ptr);
  virtual void visit(FuncDecl::Ptr);
  virtual void visit(WhileStmt::Ptr);
  virtual void visit(IfStmt::Ptr);
  virtual void visit(ForStmt::Ptr);
  virtual void visit(AssignStmt::Ptr);
  virtual void visit(TensorReadExpr::Ptr);
  
  void reportError(std::string, HIRNode::Ptr);

private:
  util::ScopedMap<std::string, Type::Ptr> decls;
  std::vector<ParseError>                *errors;
};

}
}

#endif

