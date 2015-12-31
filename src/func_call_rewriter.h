#ifndef SIMIT_FUNC_CALL_REWRITER_H
#define SIMIT_FUNC_CALL_REWRITER_H

#include <vector>

#include "hir.h"
#include "hir_rewriter.h"
#include "error.h"
#include "program_context.h"

namespace simit {
namespace hir {

class FuncCallRewriter : public HIRRewriter {
public:
  FuncCallRewriter(std::vector<ParseError> *errors) : errors(errors) {}
  
  virtual void visit(FuncDecl::Ptr);
  virtual void visit(TensorReadExpr::Ptr);

private:
  void reportError(const std::string msg, HIRNode::Ptr loc) {
    const auto err = ParseError(loc->getLineBegin(), loc->getColBegin(), 
                                loc->getLineEnd(), loc->getColEnd(), msg);
    errors->push_back(err);
  }

  internal::ProgramContext ctx;
  std::vector<ParseError> *errors;
};

}
}

#endif

