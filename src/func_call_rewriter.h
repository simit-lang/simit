#ifndef SIMIT_FUNC_CALL_REWRITER_H
#define SIMIT_FUNC_CALL_REWRITER_H

#include <set>
#include <vector>

#include "hir.h"
#include "hir_rewriter.h"
#include "error.h"

namespace simit {
namespace hir {

class FuncCallRewriter : public HIRRewriter {
public:
  FuncCallRewriter(std::vector<ParseError> *errors) : 
    errors(errors) {}
  
  virtual void visit(FuncDecl::Ptr);
  virtual void visit(TensorReadExpr::Ptr);

private:
  std::set<std::string> funcs;
  std::vector<ParseError> *errors;
};

}
}

#endif

