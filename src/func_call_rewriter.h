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
  FuncCallRewriter(std::vector<ParseError> *errors) : 
    errors(errors) {}
  
  virtual void visit(FuncDecl::Ptr);
  virtual void visit(TensorReadExpr::Ptr);

private:
  internal::ProgramContext ctx;
  std::vector<ParseError> *errors;
};

}
}

#endif

