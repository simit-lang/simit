#include "path_expression_analysis.h"

#include <stack>

#include "path_expressions.h"
#include "var.h"
#include "func.h"
#include "ir.h"
#include "ir_visitor.h"

using namespace std;
using namespace simit::pe;

namespace simit {
namespace ir {

map<Var, pe::PathExpression> computePathExpressions(const Func& func) {
  map<Var, pe::PathExpression> pathExpressions;

  return pathExpressions;
}

pe::PathExpression
PathExpressionBuilder::computePathExpression(Var target, const Map* iexpr) {

}


pe::PathExpression
PathExpressionBuilder::computePathExpression(Var target,
                                             const IndexExpr* iexpr) {
  map<IndexVar,pe::Var> pathExpressionVars;

  stack<PathExpression> peStack;
  match(Expr(iexpr),
    function<void(const IndexedTensor*)>([&](const IndexedTensor* op){
      iassert(isa<VarExpr>(op->tensor))
          << "Index expression should have been flattened by now";

      // Retrieve the indexed tensor's path expression


    }),
    function<void(const Add*, Matcher* ctx)>([&](const Add* op, Matcher* ctx) {
      ctx->match(op->a);
      ctx->match(op->b);
    })
  );

  return PathExpression();
}

}}
