#include "ir_codegen.h"
#include "ir_transforms.h"
#include "ir_rewriter.h"

#include "gpu_ir.h"

namespace simit {
namespace ir {

class LiftVarDecls : public IRRewriter {
public:
  std::vector<Stmt> varDecls;

  void visit(const VarDecl *op) {
    varDecls.push_back(op);
    stmt = Stmt();
  }

  // Lift GPUKernel vars, but don't delve into the kernel
  void visit(const GPUKernel *op) {
    Stmt newBody = moveVarDeclsToFront(op->body);
    stmt = GPUKernel::make(newBody, op->sharding);
  }
};

Func rewriteVarDecls(Func func) {
  LiftVarDecls rewriter;
  Func out = rewriter.rewrite(func);
  if (rewriter.varDecls.size() > 0) {
    return Func(out, Block::make(Block::make(
        rewriter.varDecls), out.getBody()));
  }
  else {
    return out;
  }
}

}}  // namespace simit::ir
