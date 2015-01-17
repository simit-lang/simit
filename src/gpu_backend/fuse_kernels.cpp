#include "fuse_kernels.h"
#include "ir_rewriter.h"

#include "kernel_rw_analysis.h"

namespace simit {
namespace ir {

class CanonicalizeBlocksRewriter : public IRRewriter {
public:
  using IRRewriter::visit;

  void visit(const Block *op) {
    Stmt first = IRRewriter::rewrite(op->first);
    iassert(first.defined());
    if (isa<Block>(first)) {
      const Block *firstBlock = to<Block>(first);
      Stmt innerFirst = firstBlock->first;
      iassert(innerFirst.defined());
      if (firstBlock->rest.defined()) {
        stmt = Block::make(
            innerFirst,
            IRRewriter::rewrite(
                Block::make(firstBlock->rest, op->rest)));
      }
      else {
        stmt = Block::make(innerFirst, IRRewriter::rewrite(op->rest));
      }
    }
    else {
      stmt = Block::make(first, IRRewriter::rewrite(op->rest));
    }
    iassert(isa<Block>(stmt));
    iassert(!isa<Block>(to<Block>(stmt)->first));
  }
};

Func canonicalizeBlocks(Func func) {
  Func out = CanonicalizeBlocksRewriter().rewrite(func);
  return out;
}

class KernelDownSwapRewriter : public IRRewriter {
public:
  using IRRewriter::visit;

  // Do not recurse into kernels
  void visit(const GPUKernel *op) {}

  void visit(const Block *op) {
    // Case 1: 
  }
};

Func fuseKernels(Func func) {
  std::set<Var> rootVars = findRootVars(func);
  Func fixedBlocks = canonicalizeBlocks(func);
  std::cout << "fuseKernels" << std::endl;
  // Pass 1: Move kernels as far down in blocks as possible,
  // starting from the top.
  return fixedBlocks;
}

}}  // namespace simit::ir
