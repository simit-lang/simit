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
  CanonicalizeBlocksRewriter(bool forward) : forward(forward) {}

  using IRRewriter::visit;

  void visit(const Block *op) {
    // Guard against undefined rest
    if (!op->rest.defined()) {
      stmt = IRRewriter::rewrite(op->first);
      return;
    }

    // TODO(gkanwar): The naming with forward vs. reverse here is horrible.
    Stmt first, rest;
    if (forward) {
      first = IRRewriter::rewrite(op->first);
      rest = op->rest;
    }
    else {
      first = IRRewriter::rewrite(op->rest);
      rest = op->first;
    }
    iassert(first.defined());
    if (isa<Block>(first)) {
      const Block *firstBlock = to<Block>(first);
      Stmt innerFirst, innerLeftover;
      if (forward) {
        innerFirst = firstBlock->first;
        innerLeftover= firstBlock->rest;
      }
      else {
        innerFirst = firstBlock->rest;
        innerLeftover = firstBlock->first;
      }
      iassert(innerFirst.defined());
      if (innerLeftover.defined()) {
        if (forward) {
          stmt = Block::make(
              innerFirst,
              IRRewriter::rewrite(
                  Block::make(innerLeftover, rest)));
        }
        else {
          stmt = Block::make(IRRewriter::rewrite(
              Block::make(rest, innerLeftover)), innerFirst);
        }
      }
      else {
        if (forward) {
          stmt = Block::make(innerFirst, IRRewriter::rewrite(rest));
        }
        else {
          stmt = Block::make(IRRewriter::rewrite(rest), innerFirst);
        }
      }
    }
    else {
      if (forward) {
        stmt = Block::make(first, IRRewriter::rewrite(rest));
      }
      else {
        stmt = Block::make(IRRewriter::rewrite(rest), first);
      }
    }

    iassert(isa<Block>(stmt));
    if (forward) {
      iassert(!isa<Block>(to<Block>(stmt)->first));
    }
    else {
      iassert(!isa<Block>(to<Block>(stmt)->rest));
    }
  }

private:
  bool forward;
};

Func canonicalizeBlocks(Func func, bool forward) {
  return CanonicalizeBlocksRewriter(forward).rewrite(func);
}

class VarReplaceRewriter : public IRRewriter {
public:
  VarReplaceRewriter(Var init, Var final) : init(init), final(final) {}

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

class KernelSwapRewriter : public IRRewriter {
public:
  KernelSwapRewriter(std::set<Var> rootVars, bool forward)
      : rootVars(rootVars), forward(forward) {}

  using IRRewriter::visit;

  // Do not recurse into kernels
  void visit(const GPUKernel *op) {
    stmt = op;
  }

  // Assumption: blocks are already canonicalized, using the rewriter above.
  // Swaps only happen in a block that contains GPUKernel as first, and another
  // block as rest.
  void visit(const Block *op) {
    Stmt maybeKernel, rest;
    // Ensure canonicalization in the correct direction
    if (forward) {
      iassert(!isa<Block>(op->first));
      maybeKernel = op->first;
      rest = op->rest;
    }
    else {
      iassert(!isa<Block>(op->rest));
      maybeKernel = op->rest;
      rest = op->first;
    }

    if (isa<GPUKernel>(maybeKernel) && isa<Block>(rest)) {
      const GPUKernel *kernel = to<GPUKernel>(maybeKernel);
      std::set<Var> kernelReads = kernel->reads;
      std::set<Var> kernelWrites = kernel->writes;
      const Block *block = to<Block>(rest);
      Stmt other, leftover;
      if (forward) {
        other = block->first;
        leftover = block->rest;
      }
      else {
        other = block->rest;
        leftover = block->first;
      }
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
          Stmt combinedBody;
          if (forward) {
            combinedBody = Block::make(kernel->body, replacedOtherBody);
          }
          else {
            combinedBody = Block::make(replacedOtherBody, kernel->body);
          }
          Stmt finalKernel = GPUKernel::make(combinedBody, kernel->sharding,
                                             combinedReads, combinedWrites);
          if (forward) {
            stmt = Block::make(finalKernel, IRRewriter::rewrite(leftover));
          }
          else {
            stmt = Block::make(IRRewriter::rewrite(leftover), finalKernel);
          }
        }
        else {
          // Swappable, but not fusable
          // TODO:
          // First, push other kernel as far as possible, then if that caused a
          // change in the neighboring stmt, recurse on this block again.
          // Progress guarantee: A kernel will have moved further in the block
          // on every recursion.
          // FOR NOW: Just recurse on that kernel
          if (forward) {
            stmt = Block::make(kernel, IRRewriter::rewrite(rest));
          }
          else {
            stmt = Block::make(IRRewriter::rewrite(rest), kernel);
          }
        }
      }
      else {
        // Do the swap and recurse
        if (leftover.defined()) {
          if (forward) {
            stmt = Block::make(other, IRRewriter::rewrite(
                Block::make(kernel, leftover)));
          }
          else {
            stmt = Block::make(IRRewriter::rewrite(
                Block::make(leftover, kernel)), other);
          }
        }
        else {
          stmt = Block::make(other, kernel);
        }
      }
    }
    // TODO(gkanwar): If we are a kernel, and rest is a kernel, try fusion
    else {
      IRRewriter::visit(op);
    }
  }

private:
  std::set<Var> rootVars;
  bool forward;
};

Func fuseKernels(Func func) {
  std::set<Var> rootVars = findRootVars(func);
  std::cout << "fuseKernels" << std::endl;
  // Pass 1: Move kernels as far down in blocks as possible,
  // starting from the top.
  func = canonicalizeBlocks(func, true);
  func = KernelSwapRewriter(rootVars, true).rewrite(func);

  // Pass 2: Move kernels as far up in blocks as possible, starting
  // from the bottom.
  func = canonicalizeBlocks(func, false);
  func = KernelSwapRewriter(rootVars, false).rewrite(func);
  return func;
}

}}  // namespace simit::ir
