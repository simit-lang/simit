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
  vector<Var> getBuffers() const {
    return buffers;
  }

private:
  vector<Var> buffers;
  set<Var> found;

  void visit(const Store *op) {
    if (isa<VarExpr>(op->buffer)) {
      const Var &buffer = to<VarExpr>(op->buffer)->var;

      if (found.find(buffer) == found.end()) {
        found.insert(buffer);
        buffers.push_back(buffer);
      }
    }
  }

  void visit(const Func *f) {
    buffers.clear();
    found.clear();

    for (auto &arg : f->getArguments()) {
      found.insert(arg);
    }

    for (auto &res : f->getResults()) {
      found.insert(res);
    }

    f->getBody().accept(this);
  }
};

vector<Var> gatherLocalBuffers(Func func) {
  GatherLocalBuffers gatherBuffers;
  func.accept(&gatherBuffers);
  return gatherBuffers.getBuffers();
}

}}
