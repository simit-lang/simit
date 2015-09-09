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
  not_supported_yet;
  return pathExpressions;
}

void PathExpressionBuilder::computePathExpression(const Map* map) {
  iassert(isa<VarExpr>(map->target))
      << "can't compute path expressions from dynamic sets (yet?)";

  const Var& targetSet = to<VarExpr>(map->target)->var;
  pe::Var e = pe::Var(targetSet.getName());
  pathExpressionVars[targetSet].push_back(e);

  for (const Var& var : map->vars) {
    iassert(var.getType().isTensor());
    const TensorType* type = var.getType().toTensor();

    std::vector<IndexSet> dims = type->getOuterDimensions();
    std::vector<pe::Var> peVars;
    for (const IndexSet& dim : dims) {
      iassert(dim.getKind() == IndexSet::Set);
      iassert(isa<VarExpr>(dim.getSet()))
          << "can't compute path expressions from dynamic sets (yet?)";

      ir::Var dimensionSet = to<VarExpr>(dim.getSet())->var;
      pe::Var v = pe::Var(dimensionSet.getName());
      pathExpressionVars[dimensionSet].push_back(v);
      peVars.push_back(v);
    }

    tassert(type->order()==2) << "path expressions only supported for matrices";
    pe::Var u = peVars[0];
    pe::Var v = peVars[1];

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

      PathExpression pe = pe::Or::make(peVars, {}, a, b);
      peStack.push(pe);
    }),
    function<void(const Mul*, Matcher* ctx)>([&](const Mul* op, Matcher* ctx) {
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

void PathExpressionBuilder::bind(ir::Var var, const Set* set) {
  iassert(util::contains(pathExpressionVars, var));
  for (pe::Var& peVar : pathExpressionVars.at(var)) {
    peVar.bind(set);
  }
}

void PathExpressionBuilder::addPathExpression(Var target,
                                              const pe::PathExpression& pe) {
  pathExpressions.insert({target, pe});
}

}}
