#include "lower.h"

#include <map>

#include "storage.h"
#include "temps.h"
#include "flatten.h"
#include "ir_rewriter.h"

#ifdef GPU
#include "gpu_backend/gpu_backend.h"
#endif

using namespace std;

namespace simit {
extern std::string kBackend;

namespace ir {

Func lower(Func func, bool print) {
  // Flatten and insert temporaries
  class FlattenRewriter : public ir::IRRewriterCallGraph {
    using IRRewriter::visit;
    void visit(const ir::Func *op) {
      if (op->getKind() != ir::Func::Internal) {
        func = *op;
        return;
      }
      func = ir::Func(*op, rewrite(op->getBody()));

#ifdef GPU
      if (kBackend == "gpu") {
        func = rewriteSystemAssigns(func);
      }
#endif
      func = flattenIndexExpressions(func);
      func = insertTemporaries(func);
    }
  };
  func = FlattenRewriter().rewrite(func);

  if (print) {
    cout << "--- Insert Temporaries and Flatten Index Expressions" << endl;
    simit::ir::IRPrinterCallGraph(cout).print(func);
    cout << endl;
  }


  // Determine Storage
  class SetStorageRewriter : public simit::ir::IRRewriterCallGraph {
    using IRRewriter::visit;
    void visit(const simit::ir::Func *op) {
      if (op->getKind() != simit::ir::Func::Internal) {
        func = *op;
        return;
      }
      func = simit::ir::Func(*op, rewrite(op->getBody()));
      func.setStorage(getStorage(func));
    }
  };
  func = SetStorageRewriter().rewrite(func);

  if (print) {
    cout << "--- Tensor storage" << endl;
    class StoragePrinter : public simit::ir::IRVisitorCallGraph {
      using simit::ir::IRVisitor::visit;
      void visit(const simit::ir::Func *func) {
        simit::ir::IRVisitorCallGraph::visit(func);
        cout << "func " << func->getName() << ":" << endl;

        for (auto &var : func->getStorage()) {
          cout << "  " << var << " : " << func->getStorage().get(var) << endl;
        }
        cout << endl;
      }
    };
    StoragePrinter storagePrinter;
    func.accept(&storagePrinter);
    cout << endl;
  }


  // Lower maps
  class LowerMapsRewriter : public simit::ir::IRRewriterCallGraph {
    using IRRewriter::visit;
    void visit(const simit::ir::Func *op) {
      if (op->getKind() != simit::ir::Func::Internal) {
        func = *op;
        return;
      }
      func = simit::ir::Func(*op, rewrite(op->getBody()));
      func = lowerMaps(func);
    }
  };
  func = LowerMapsRewriter().rewrite(func);

  if (print) {
    cout << "--- Lower Maps" << endl;
    simit::ir::IRPrinterCallGraph(cout).print(func);
    cout << endl;
  }


  // Lower Index Expressions
  class LowerIndexExpressionsRewriter : public simit::ir::IRRewriterCallGraph {
    using IRRewriter::visit;
    void visit(const simit::ir::Func *op) {
      if (op->getKind() != simit::ir::Func::Internal) {
        func = *op;
        return;
      }
      func = simit::ir::Func(*op, rewrite(op->getBody()));
      func = lowerIndexExpressions(func);
    }
  };
  func = LowerIndexExpressionsRewriter().rewrite(func);

  if (print) {
    cout << "--- Lower Index Expressions" << endl;
    simit::ir::IRPrinterCallGraph(cout).print(func);
    cout << endl;
  }


  // Lower Tensor Reads and Writes
  class LowerTensorAccessesRewriter : public simit::ir::IRRewriterCallGraph {
    using IRRewriter::visit;
    void visit(const simit::ir::Func *op) {
      if (op->getKind() != simit::ir::Func::Internal) {
        func = *op;
        return;
      }
      func = simit::ir::Func(*op, rewrite(op->getBody()));
      func = lowerTensorAccesses(func);
    }
  };
  func = LowerTensorAccessesRewriter().rewrite(func);

  if (print) {
    cout << "--- Lower Tensor Reads and Writes" << endl;
    simit::ir::IRPrinterCallGraph(cout).print(func);
    cout << endl;
  }


#if GPU
  if (kBackend == "gpu") {
    class ShardLoopsRewriter : public simit::ir::IRRewriterCallGraph {
      using IRRewriter::visit;
      void visit(const simit::ir::Func *op) {
        if (op->getKind() != simit::ir::Func::Internal) {
          func = *op;
          return;
        }
        func = simit::ir::Func(*op, rewrite(op->getBody()));
        func = shardLoops(func);
      }
    };
    func = ShardLoopsRewriter().rewrite(func);

    class VarDeclsRewriter : public simit::ir::IRRewriterCallGraph {
      using IRRewriter::visit;
      void visit(const simit::ir::Func *op) {
        if (op->getKind() != simit::ir::Func::Internal) {
          func = *op;
          return;
        }
        func = simit::ir::Func(*op, rewrite(op->getBody()));
        func = rewriteVarDecls(func);
      }
    };
    func = VarDeclsRewriter().rewrite(func);


    class KernelRWAnalysis : public simit::ir::IRRewriterCallGraph {
      using IRRewriter::visit;
      void visit(const simit::ir::Func *op) {
        if (op->getKind() != simit::ir::Func::Internal) {
          func = *op;
          return;
        }
        func = simit::ir::Func(*op, rewrite(op->getBody()));
        func = kernelRWAnalysis(func);
      }
    };
    func = KernelRWAnalysis().rewrite(func);

    class FuseKernelsRewriter : public simit::ir::IRRewriterCallGraph {
      using IRRewriter::visit;
      void visit(const simit::ir::Func *op) {
        if (op->getKind() != simit::ir::Func::Internal) {
          func = *op;
          return;
        }
        func = simit::ir::Func(*op, rewrite(op->getBody()));
        func = fuseKernels(func);
      }
    };
    func = FuseKernelsRewriter().rewrite(func);
  }
#endif
  return func;
}

}}
