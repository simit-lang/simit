#include "lower.h"

#include <map>
#include <fstream>

#include "lower_maps.h"
#include "index_expressions/lower_index_expressions.h"

#include "lower_accesses.h"
#include "lower_prints.h"

#include "storage.h"
#include "timers.h"
#include "temps.h"
#include "flatten.h"
#include "ir_rewriter.h"
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
      simit::ir::IRVisitorCallGraph::visit(op);
      visitRule(*op);
    }
  };
  Visitor visitor(visitRule);
  func.accept(&visitor);
}

static inline
void timingCallGraph(string headerText, Func func, bool print) {
  stringstream ss;
  simit::ir::IRPrinterCallGraph(ss).print(func);
  TimerStorage::getInstance().addSourceLines(ss);
}

static inline
void printCallGraph(string headerText, Func func, bool print) {
  if (print) {
    cout << "--- " << headerText << endl;
    simit::ir::IRPrinterCallGraph(cout).print(func);
    cout << endl;
  }
}

Func lower(Func func, bool print) {
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
  printCallGraph("Insert Temporaries and Flatten Index Expressions", func, print);

  // Determine Storage
  func = rewriteCallGraph(func, [](Func func) -> Func {
    func.setStorage(getStorage(func));
    return func;
  });
  if (print) {
    cout << "--- Tensor storage" << endl;
    visitCallGraph(func, [](Func func) {
      cout << "func " << func.getName() << ":" << endl;
      for (auto &var : func.getStorage()) {
        cout << "  " << var <<" : "<< func.getStorage().getStorage(var) << endl;
      }
      cout << endl;
    });
    cout << endl;
  }

  func = rewriteCallGraph(func, lowerPrints);
  printCallGraph("Lower Prints", func, print);

  func = rewriteCallGraph(func, lowerFieldAccesses);
  printCallGraph("Lower Field Accesses", func, print);

  // Lower maps
  func = rewriteCallGraph(func, lowerMaps);
  printCallGraph("Lower Maps", func, print);

  // Lower Index Expressions
  func = rewriteCallGraph(func, lowerIndexExpressions);
  printCallGraph("Lower Index Expressions", func, print);

  // Lower Tensor Reads and Writes
  func = rewriteCallGraph(func, lowerTensorAccesses);
  printCallGraph("Lower Tensor Reads and Writes", func, print);
  
  // Lower to GPU Kernels
#if GPU
  if (kBackend == "gpu") {
    func = rewriteCallGraph(func, shardLoops);
    printCallGraph("Shard Loops", func, print);
    func = rewriteCallGraph(func, rewriteVarDecls);
    printCallGraph("Rewritten Var Decls", func, print);
    func = rewriteCallGraph(func, localizeTemps);
    printCallGraph("Localize Temps", func, print);
    func = rewriteCallGraph(func, kernelRWAnalysis);
    printCallGraph("Kernel RW Analysis", func, print);
    func = rewriteCallGraph(func, fuseKernels);
    printCallGraph("Fuse Kernels", func, print);
  }
#endif
  return func;
}

Func lowerWithTimers(Func func, bool print) {
  // Include Timers 
  timingCallGraph("Insert Timers", func, print);
  func = rewriteCallGraph(func, insertTimers);
  printCallGraph("Insert Timers", func, print);
  return func;
}

}}
