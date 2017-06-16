#ifndef SIMIT_IR_PROGRAM_ANALYSIS_NODE_REPLACER_H
#define SIMIT_IR_PROGRAM_ANALYSIS_NODE_REPLACER_H

#include "ir_rewriter.h"

namespace simit {
namespace ir {
namespace program_analysis {

class NodeReplacer : public IRRewriter {
  unsigned int disableLevel = 0;
public:
  virtual Expr rewrite(Expr expr);
  virtual Stmt rewrite(Stmt stmt);
  virtual Func rewrite(Func func);

  void enable();
  void disable();

protected:
  Expr exprReplacement;
  Stmt stmtReplacement;
  Func funcReplacement;
};

}
}
}

#endif
