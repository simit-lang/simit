#include "fuse_kernels.h"

#include <algorithm>

#include "ir_rewriter.h"
#include "rw_analysis.h"

#include "kernel_rw_analysis.h"

namespace simit {
namespace ir {

// Rewrite all blocks into tail-linked-list format (i.e. each block may only
// directly contain other blocks in rest, not first).
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
  return CanonicalizeBlocksRewriter().rewrite(func);
}

class VarReplaceRewriter : public IRRewriter {
public:
  VarReplaceRewriter(Var init, Var final) {
    this->init = init;
    this->final = final;
  }

  using IRRewriter::visit;

  void visit(const VarExpr *op) {
    if (op->var == init) {
      expr = VarExpr::make(final);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const VarDecl *op) {
    if (op->var == init) {
      stmt = VarDecl::make(final);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const AssignStmt *op) {
    Expr value = rewrite(op->value);
    if (op->var == init) {
      stmt = AssignStmt::make(final, value, op->cop);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const CallStmt *op) {
    std::vector<Var> finalResults;
    for (Var res : op->results) {
      if (res == init)
        finalResults.push_back(final);
      else
        finalResults.push_back(res);
    }
    std::vector<Expr> actuals(op->actuals.size());
    for (size_t i=0; i < op->actuals.size(); ++i) {
      actuals[i] = rewrite(op->actuals[i]);
    }
    stmt = CallStmt::make(finalResults, op->callee, actuals);
  }

  void visit(const ForRange *op) {
    Expr start = rewrite(op->start);
    Expr end = rewrite(op->end);
    Stmt body = rewrite(op->body);
    if (op->var == init) {
      stmt = ForRange::make(final, start, end, body);
    }
    else {
      IRRewriter::visit(op);
    }
  }

  void visit(const For *op) {
    Stmt body = rewrite(op->body);
    if (op->domain.var == init) {

      ForDomain domain = ForDomain(op->domain.set, final,
                                   op->domain.kind, op->domain.indexSet);
      stmt = For::make(op->var, domain, body);
    }
    else if (op->var == init) {
      stmt = For::make(final, op->domain, body);
    }
    else {
      IRRewriter::visit(op);
    }
  }

private:
  Var init, final;
};

Stmt replaceVar(Stmt stmt, Var init, Var final) {
  return VarReplaceRewriter(init, final).rewrite(stmt);
}

class KernelDownSwapRewriter : public IRRewriter {
public:
  KernelDownSwapRewriter(std::set<Var> rootVars) {
    this->rootVars = rootVars;
  }

  using IRRewriter::visit;

  // Do not recurse into kernels
  void visit(const GPUKernel *op) {
    stmt = op;
  }

  // Assumption: blocks are already canonicalized, using the rewriter above.
  // Swaps only happen in a block that contains GPUKernel as first, and another
  // block as rest.
  void visit(const Block *op) {
    if (isa<GPUKernel>(op->first) && isa<Block>(op->rest)) {
      const GPUKernel *kernel = to<GPUKernel>(op->first);
      std::set<Var> kernelReads = kernel->reads;
      std::set<Var> kernelWrites = kernel->writes;
      const Block *block = to<Block>(op->rest);
      Stmt other = block->first;
      ReadWriteAnalysis rwAnalysis(rootVars);
      other.accept(&rwAnalysis);
      std::set<Var> otherReads = rwAnalysis.getReads();
      std::set<Var> otherWrites = rwAnalysis.getWrites();

      std::vector<Var> conflicts;
      // Write-read dependency
      std::set_intersection(kernelWrites.begin(), kernelWrites.end(),
                            otherReads.begin(), otherReads.end(),
                            std::back_inserter(conflicts));
      // Read-write conflict
      std::set_intersection(kernelReads.begin(), kernelReads.end(),
                            otherWrites.begin(), otherWrites.end(),
                            std::back_inserter(conflicts));
      // Write-write conflict
      std::set_intersection(kernelWrites.begin(), kernelWrites.end(),
                            otherWrites.begin(), otherWrites.end(),
                            std::back_inserter(conflicts));
      if (conflicts.size() > 0) {
        // We have a write-read dependency or write-write conflict, so
        // no swapping allowed.
        IRRewriter::visit(op);
        return;
      }

      // Fallthrough: No conflicts
      // Check whether these are both kernels and can fuse
      if (isa<GPUKernel>(other)) {
        const GPUKernel *otherKernel = to<GPUKernel>(other);
        if (otherKernel->sharding == kernel->sharding) {
          std::set<Var> combinedReads;
          std::set_union(
              kernelReads.begin(), kernelReads.end(),
              otherReads.begin(), otherReads.end(),
              std::inserter(combinedReads, combinedReads.end()));
          std::set<Var> combinedWrites;
          std::set_union(
              kernelWrites.begin(), kernelWrites.end(),
              otherWrites.begin(), otherWrites.end(),
              std::inserter(combinedWrites, combinedWrites.end()));
              
          Stmt replacedOtherBody = otherKernel->body;
          if (kernel->sharding.xSharded) {
            iassert(otherKernel->sharding.xSharded);
            replacedOtherBody = replaceVar(
                replacedOtherBody,
                otherKernel->sharding.xVar, kernel->sharding.xVar);
          }
          iassert(!kernel->sharding.ySharded &&
                  !kernel->sharding.zSharded &&
                  !otherKernel->sharding.ySharded &&
                  !otherKernel->sharding.zSharded);
          Stmt finalKernel = GPUKernel::make(
              // Combined bodies
              Block::make(kernel->body, replacedOtherBody),
              kernel->sharding, combinedReads, combinedWrites);
          stmt = Block::make(finalKernel,
                             IRRewriter::rewrite(block->rest));
        }
        else {
          // Swappable, but not fusable
          // TODO:
          // First, push other kernel as far as possible, then if that caused a
          // change in the neighboring stmt, recurse on this block again.
          // Progress guarantee: A kernel will have moved further in the block
          // on every recursion.
          // FOR NOW: Just recurse on that kernel
          stmt = Block::make(kernel, IRRewriter::rewrite(op->rest));
        }
      }
      else {
        // Do the swap and recurse
        if (block->rest.defined()) {
          stmt = Block::make(other, IRRewriter::rewrite(
              Block::make(kernel, block->rest)));
        }
        else {
          stmt = Block::make(other, kernel);
        }
      }
    }
    else {
      IRRewriter::visit(op);
    }
  }

private:
  std::set<Var> rootVars;
};

Func fuseKernels(Func func) {
  std::set<Var> rootVars = findRootVars(func);
  func = canonicalizeBlocks(func);
  std::cout << "fuseKernels" << std::endl;
  // Pass 1: Move kernels as far down in blocks as possible,
  // starting from the top.
  func = KernelDownSwapRewriter(rootVars).rewrite(func);
  return func;
}

}}  // namespace simit::ir
