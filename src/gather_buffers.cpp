#include "gather_buffers.h"

#include <vector>

#include "ir.h"
#include "ir_rewriter.h"

using namespace std;
using namespace simit::ir;

namespace simit {
namespace internal {

// TODO: GatherBuffers must take scopes into account
class GatherLocalBuffers : public IRVisitor {
public:
  vector<Var> getBuffers(const Func &func) {
    buffers.clear();
    found.clear();
    func.accept(this);
    return buffers;
  }

private:
  vector<Var> buffers;
  set<Var> found;

  // Function parameters are allocated by caller, so add them to the found set
  void visit(const Func *f) {
    for (auto &arg : f->getArguments()) {
      found.insert(arg);
    }

    for (auto &res : f->getResults()) {
      found.insert(res);
    }

    IRVisitor::visit(f);
  }

  void visit(const Store *op) {
    if (isa<VarExpr>(op->buffer)) {
      const Var &buffer = to<VarExpr>(op->buffer)->var;

      if (found.find(buffer) == found.end()) {
        found.insert(buffer);
        buffers.push_back(buffer);
      }
    }
  }
};

vector<Var> gatherLocalBuffers(Func func) {
  return GatherLocalBuffers().getBuffers(func);
}

}}
