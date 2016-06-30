#ifndef SIMIT_CONTEXT_SENSITIVE_REWRITER_H
#define SIMIT_CONTEXT_SENSITIVE_REWRITER_H

#include <string>
#include <vector>

#include "error.h"
#include "hir.h"
#include "hir_rewriter.h"
#include "util/scopedmap.h"

namespace simit {
namespace hir {

class ContextSensitiveRewriter : public HIRRewriter {
public:
  ContextSensitiveRewriter(std::vector<ParseError> *);

private:
  virtual void visit(SetIndexSet::Ptr);
  virtual void visit(IdentDecl::Ptr);
  virtual void visit(FuncDecl::Ptr);
  virtual void visit(VarDecl::Ptr);
  virtual void visit(WhileStmt::Ptr);
  virtual void visit(IfStmt::Ptr);
  virtual void visit(ForStmt::Ptr);
  virtual void visit(AssignStmt::Ptr);
  virtual void visit(TensorReadExpr::Ptr);
  
  void reportError(std::string, HIRNode::Ptr);

private:
  enum class IdentType {TYPE_PARAM, TUPLE, FUNCTION, OTHER};

private:
  util::ScopedMap<std::string, IdentType> decls;
  std::vector<ParseError>                *errors;
};

}
}

#endif

