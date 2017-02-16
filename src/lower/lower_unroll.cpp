#include "lower_unroll.h"

#include "ir_rewriter.h"
#include "var_replace_rewriter.h"

namespace simit {
namespace ir {

class LowerUnroll : public IRRewriter {
  using IRRewriter::visit;

public :
  LowerUnroll() {};

private :
  const int unrollMax=4;

  void visit(const For *op) {
    IRRewriter::visit(op);
    const auto forDomain = op->domain;
    if (forDomain.kind == ForDomain::IndexSet &&
        forDomain.indexSet.getKind() == IndexSet::Range) {
      const unsigned range = forDomain.indexSet.getSize();
      if (range > 0 && range <= unrollMax) {
        const Stmt body = op->body;
        const Var var = op->var;
        stmt = replaceVarByExpr(body, var, 0);
        for (size_t i = 1; i < range; ++i) {
          stmt = Block::make(stmt, replaceVarByExpr(body, var, (int)i));
        }
      }
    }
  }
  void visit(const ForRange *op) {
    IRRewriter::visit(op);
    if (isa<Literal>(op->start) && isa<Literal>(op->end)) {
      const unsigned range = to<Literal>(op->end)->getIntVal(0)
	                     - to<Literal>(op->start)->getIntVal(0);
      if (range > 0 && range <= unrollMax) {
        const Stmt body = op->body;
        const Var var = op->var;
        stmt = replaceVarByExpr(body, var, 0);
        for (size_t i = 1; i < range; ++i) {
          stmt = Block::make(stmt, replaceVarByExpr(body, var, (int)i));
        }
      }
    }
  }
};


Func lowerUnroll(Func func) {
  func = LowerUnroll().rewrite(func);
  return func;
}

}}

