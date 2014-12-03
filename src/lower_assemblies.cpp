#include "lower.h"

#include "ir_rewriter.h"

using namespace std;

namespace simit {
namespace ir {

class LowerAssemblies : public IRRewriter {
private:
  void visit(const Map *op) {
    // \todo We should only drop the map statements if it's bound Vars have
    // no uses (extend/invert UseDef to get DefUse info).
  }
};

Stmt lowerAssemblies(Stmt stmt) {
  return LowerAssemblies().mutate(stmt);
}

Func lowerAssemblies(Func func) {
  Stmt body = lowerAssemblies(func.getBody());
  return Func(func, body);
}

}}
