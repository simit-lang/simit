#ifndef SIMIT_TYPE_PARAM_REWRITER_H
#define SIMIT_TYPE_PARAM_REWRITER_H

#include <string>

#include "hir.h"
#include "hir_rewriter.h"

namespace simit {
namespace hir {

class TypeParamRewriter : public HIRRewriter {
private:
  virtual void visit(SetIndexSet::Ptr);
  virtual void visit(IdentDecl::Ptr);
  virtual void visit(FuncDecl::Ptr);
  virtual void visit(WhileStmt::Ptr);
  virtual void visit(IfStmt::Ptr);
  virtual void visit(ForStmt::Ptr);
  virtual void visit(AssignStmt::Ptr);

private:
  util::ScopedMap<std::string, bool> decls;
};

}
}

#endif

