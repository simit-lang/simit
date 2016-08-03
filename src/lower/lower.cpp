#include "lower.h"

#include <map>
#include <fstream>

#include "lower_maps.h"
#include "index_expressions/lower_index_expressions.h"

#include "lower_accesses.h"
#include "lower_prints.h"
#include "lower_string_ops.h"
#include "lower_stencil_assemblies.h"

#include "storage.h"
#include "timers.h"
#include "temps.h"
#include "flatten.h"
#include "insert_frees.h"
#include "ir_rewriter.h"
#include "ir_transforms.h"
#include "ir_printer.h"
#include "path_expressions.h"

#ifdef GPU
#include "backend/gpu/gpu_backend.h"
#endif

using namespace std;

namespace simit {
extern std::string kBackend;

namespace ir {

static
Func rewriteCallGraph(const Func& func, const function<Func(Func)>& rewriter) {
  class Rewriter : public simit::ir::IRRewriterCallGraph {
  public:
    Rewriter(const function<Func(Func)>& rewriter) : rewriter(rewriter) {}
    const function<Func(Func)>& rewriter;

    using IRRewriter::visit;
    void visit(const simit::ir::Func *op) {
      if (op->getKind() != simit::ir::Func::Internal) {
        func = *op;
        return;
      }
      func = simit::ir::Func(*op, rewrite(op->getBody()));
      func = rewriter(func);
    }
  };
  return Rewriter(rewriter).rewrite(func);
}

void visitCallGraph(Func func, const function<void(Func)>& visitRule) {
  class Visitor : public simit::ir::IRVisitorCallGraph {
  public:
    Visitor(const function<void(Func)>& visitRule) : visitRule(visitRule) {}
    const function<void(Func)>& visitRule;

    using simit::ir::IRVisitor::visit;
    void visit(const simit::ir::Func *op) {
      if (op->getKind() != simit::ir::Func::Internal) {
        return;
      }
      simit::ir::IRVisitorCallGraph::visit(op);
      visitRule(*op);
    }
  };
  Visitor visitor(visitRule);
  func.accept(&visitor);
}

static inline
void printTimedCallGraph(string headerText, Func func, ostream* os) {
  stringstream ss;
  simit::ir::IRPrinterCallGraph(ss).print(func);
  TimerStorage::getInstance().addSourceLines(ss);
  *os << ss.rdbuf();
}

static inline
void printCallGraph(string headerText, Func func, ostream* os) {
  if (os) {
    *os << "%% " << headerText << endl;
    simit::ir::IRPrinterCallGraph(*os).print(func);
    *os << endl;
  }
}

Func lower(Func func, std::ostream* os, bool time) {
#ifdef GPU
  // Rewrite system assignments
  if (kBackend == "gpu") {
    func = rewriteCallGraph(func, rewriteSystemAssigns);
    printCallGraph("Rewrite System Assigns (GPU)", func, print);
  }
#endif

  // Flatten index expressions and insert temporaries
  func = rewriteCallGraph(func, (Func(*)(Func))flattenIndexExpressions);
  func = rewriteCallGraph(func, insertTemporaries);
  printCallGraph("Insert Temporaries and Flatten Index Expressions", func, os);

  // Determine Storage
  func = rewriteCallGraph(func, [](Func func) -> Func {
    updateStorage(func, &func.getStorage(), &func.getEnvironment());
    return func;
  });
  if (os) {
    *os << "%% Tensor storage" << endl;
    visitCallGraph(func, [os](Func func) {
      *os << "func " << func.getName() << ":" << endl;
      for (auto &var : func.getStorage()) {
        *os << "  " << var <<" : "<< func.getStorage().getStorage(var) << endl;
      }
      *os << endl;
    });
    *os << endl;
  }

  func = rewriteCallGraph(func, insertFrees);
  printCallGraph("Insert Frees", func, os);

  func = rewriteCallGraph(func, lowerStringOps);
  func = rewriteCallGraph(func, lowerPrints);
  printCallGraph("Lower String Operations and Prints", func, os);

  func = rewriteCallGraph(func, lowerFieldAccesses);
  printCallGraph("Lower Field Accesses", func, os);

  // Lower stencil assemblies
  func = rewriteCallGraph(func, lowerStencilAssemblies);
  printCallGraph("Normalize Row Indices", func, os);

  // Lower maps
  func = rewriteCallGraph(func, lowerMaps);
  printCallGraph("Lower Maps", func, os);

  // Lower Index Expressions
  func = rewriteCallGraph(func, lowerIndexExpressions);
  printCallGraph("Lower Index Expressions", func, os);

  // Lower Tensor Reads and Writes
  func = rewriteCallGraph(func, lowerTensorAccesses);
  printCallGraph("Lower Tensor Reads and Writes", func, os);

  if (time) {
    printTimedCallGraph("Insert Timers", func, os);
    func = rewriteCallGraph(func, insertTimers);
    printCallGraph("Insert Timers", func, os);
  }

  // Lower to GPU Kernels
#if GPU
  if (kBackend == "gpu") {
    func = rewriteCallGraph(func, rewriteCompoundOps);
    printCallGraph("Rewrite Compound Ops (GPU)", func, print);
    func = rewriteCallGraph(func, shardLoops);
    printCallGraph("Shard Loops", func, os);
    func = rewriteCallGraph(func, rewriteVarDecls);
    printCallGraph("Rewritten Var Decls", func, os);
    func = rewriteCallGraph(func, localizeTemps);
    printCallGraph("Localize Temps", func, os);
    func = rewriteCallGraph(func, kernelRWAnalysis);
    printCallGraph("Kernel RW Analysis", func, os);
    func = rewriteCallGraph(func, fuseKernels);
    printCallGraph("Fuse Kernels", func, os);
  }
#endif
  return func;
}

}}
