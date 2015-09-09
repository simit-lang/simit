#include "path_expression_analysis.h"

#include <stack>

#include "path_expressions.h"
#include "var.h"
#include "func.h"
#include "ir.h"
#include "ir_visitor.h"
#include "util/collections.h"

using namespace std;
using namespace simit::pe;

namespace simit {
namespace ir {

map<Var, pe::PathExpression> getPathExpressions(const Func& func) {
  map<Var, pe::PathExpression> pathExpressions;

  return pathExpressions;
}

void PathExpressionBuilder::computePathExpression(const Map* map) {
  iassert(isa<VarExpr>(map->target))
      << "can't compute path expressions from dynamic sets (yet?)";

  const Var& targetSet = to<VarExpr>(map->target)->var;
  pe::Var e = pe::Var(targetSet.getName());

  for (const Var& var : map->vars) {
    iassert(var.getType().isTensor());
    const TensorType* type = var.getType().toTensor();
    tassert(type->order()==2) << "path expressions only supported for matrices";

    std::vector<IndexSet> dims = type->getOuterDimensions();
    iassert(dims[0].getKind() == IndexSet::Set &&
            dims[1].getKind() == IndexSet::Set);
    iassert(isa<VarExpr>(dims[0].getSet()) && isa<VarExpr>(dims[1].getSet()))
        << "can't compute path expressions from dynamic sets (yet?)";

    Var dim0 = to<VarExpr>(dims[0].getSet())->var;
    Var dim1 = to<VarExpr>(dims[1].getSet())->var;

    pe::Var u = pe::Var(dim0.getName());
    pe::Var v = pe::Var(dim1.getName());

    pe::PathExpression ve = Link::make(u, e, Link::ve);
    pe::PathExpression ev = Link::make(e, v, Link::ev);

    PathExpression vev = pe::And::make({u,v}, {{QuantifiedVar::Exist,e}},
                                       ve(u,e), ev(e,v));
    addPathExpression(var, vev);
  }
}

void PathExpressionBuilder::computePathExpression(Var target,
                                                  const IndexExpr* iexpr){
  vector<pe::Var> peVars;
  map<IndexVar,pe::Var> peVarMap;
  for (const IndexVar& indexVar : iexpr->resultVars) {
    pe::Var peVar = pe::Var(indexVar.getName());
    peVars.push_back(peVar);
    peVarMap.insert({indexVar, peVar});
  }

  stack<PathExpression> peStack;
  match(Expr(iexpr),
    function<void(const IndexedTensor*)>([&](const IndexedTensor* op){
      iassert(isa<VarExpr>(op->tensor))
          << "index expression should have been flattened by now";
      const Var& tensor = to<VarExpr>(op->tensor)->var;
      iassert(tensor.getType().isTensor());

      // Retrieve the indexed tensor's path expression
      PathExpression pe = getPathExpression(to<VarExpr>(op->tensor)->var);

      tassert(op->indexVars.size()==2)<<"only matrices are currently supported";

      peStack.push(pe(peVarMap.at(op->indexVars[0]),
                      peVarMap.at(op->indexVars[1])));

    }),
    function<void(const Add*, Matcher* ctx)>([&](const Add* op, Matcher* ctx) {
      ctx->match(op->a);
      PathExpression a = peStack.top();
      peStack.pop();

      ctx->match(op->b);
      PathExpression b = peStack.top();
      peStack.pop();

      PathExpression pe = pe::And::make(peVars, {}, a, b);
      peStack.push(pe);
    })
  );

  iassert(peStack.size() == 1);
  addPathExpression(target, peStack.top());
}

pe::PathExpression PathExpressionBuilder::getPathExpression(Var target) {
  iassert(util::contains(pathExpressions, target));
  return pathExpressions.at(target);
}

void PathExpressionBuilder::addPathExpression(Var target,
                                              const pe::PathExpression& pe) {
  pathExpressions.insert({target, pe});
}

}}
