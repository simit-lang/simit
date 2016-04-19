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
  pe::Set E = getPathExpressionSet(targetSet);
  pe::Var e = pe::Var("e", E);

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

      pe::Set V = getPathExpressionSet(dimensionSet);
      pe::Var v = pe::Var("v", V);

      peVars.push_back(v);
    }

    if (type->order() >= 2) {
      tassert(type->order() == 2)
          << "path expressions only supported for matrices";
      pe::Var u = peVars[0];
      pe::Var v = peVars[1];

      pe::PathExpression ve = Link::make(u, e, Link::ve);
      pe::PathExpression ev = Link::make(e, v, Link::ev);

      PathExpression vev = pe::And::make({u,v}, {{QuantifiedVar::Exist,e}},
                                         ve(u,e), ev(e,v));
      addPathExpression(var, vev);
    }
  }
}

static PathExpression orPathExpressions(PathExpression a, PathExpression b,
                                        vector<pe::Var> peVars,
                                        vector<QuantifiedVar> qvars) {
  if (a.defined() && b.defined()) {
    return pe::Or::make(peVars, qvars, a, b);
  }
  else if (a.defined()) {
    return a;
  }
  else if (b.defined()) {
    return b;
  }
  else {
    return PathExpression();
  }
}

static PathExpression andPathExpressions(PathExpression a, PathExpression b,
                                         vector<pe::Var> peVars,
                                         vector<QuantifiedVar> qvars) {
  if (a.defined() && b.defined()) {
    return pe::And::make(peVars, qvars, a, b);
  }
  else if (a.defined()) {
    return a;
  }
  else if (b.defined()) {
    return b;
  }
  else {
    return PathExpression();
  }
}

void PathExpressionBuilder::computePathExpression(Var target,
                                                  const IndexExpr* iexpr){
  vector<pe::Var> peVars;
  map<IndexVar,pe::Var> peVarMap;
  for (const IndexVar& indexVar : iexpr->resultVars) {
    pe::Var peVar = pe::Var(indexVar.getName(), pe::Set());
    peVars.push_back(peVar);
    peVarMap.insert({indexVar, peVar});
  }

  stack<PathExpression> peStack;
  vector<QuantifiedVar> qvars;

  match(Expr(iexpr),
    function<void(const IndexedTensor*)>([&](const IndexedTensor* op) {
      iassert(op->tensor.type().isTensor());
      const TensorType* type = op->tensor.type().toTensor();
      tassert(type->order()<=2) << "we do not support higher-order tensors yet";

      if (type->order() == 2) {
        // Perhaps continue the traversal into op->tensor and match on VarExpr,
        // literal, FieldRead, etc, to get op->tensor's path expression.
        tassert(isa<VarExpr>(op->tensor))
            << "generalize to work with indexed literals, field reads, etc.";

        // Retrieve the indexed tensor's path expression
        PathExpression pe = getPathExpression(to<VarExpr>(op->tensor)->var);
        if (pe.defined()) {
          tassert(op->indexVars.size() == 2)
              << "only matrices are currently supported";

          // We must check for, and add to the map, any reduction variables
          for (const IndexVar& indexVar : op->indexVars) {
            if (indexVar.isReductionVar() && !util::contains(peVarMap,indexVar)) {
              pe::Var peVar = pe::Var(indexVar.getName(), pe::Set());
              peVarMap.insert({indexVar, peVar});
              qvars.push_back(QuantifiedVar(QuantifiedVar::Exist, peVar));
            }
          }

          peStack.push(pe(peVarMap.at(op->indexVars[0]),
                          peVarMap.at(op->indexVars[1])));
        }
        else {
          // Matrices without path expressions (e.g. Diagonal matrices)
          peStack.push(PathExpression());
        }
      }
      else {
        // Scalars and vectors are dense and do not have path expressions,
        // so we push an undefined path expression on the stack.
        peStack.push(PathExpression());
      }
    }),
    function<void(const Add*, Matcher*)>([&](const Add* op, Matcher* ctx) {
      ctx->match(op->a);
      PathExpression a = peStack.top();
      peStack.pop();

      ctx->match(op->b);
      PathExpression b = peStack.top();
      peStack.pop();

      peStack.push(orPathExpressions(a, b, peVars, qvars));
      qvars.clear();
    }),
    function<void(const Sub*, Matcher* ctx)>([&](const Sub* op, Matcher* ctx) {
      ctx->match(op->a);
      PathExpression a = peStack.top();
      peStack.pop();

      ctx->match(op->b);
      PathExpression b = peStack.top();
      peStack.pop();

      peStack.push(orPathExpressions(a, b, peVars, qvars));
      qvars.clear();
    }),
    function<void(const Mul*, Matcher* ctx)>([&](const Mul* op, Matcher* ctx) {
      ctx->match(op->a);
      PathExpression a = peStack.top();
      peStack.pop();

      ctx->match(op->b);
      PathExpression b = peStack.top();
      peStack.pop();

      peStack.push(andPathExpressions(a, b, peVars, qvars));
      qvars.clear();
    }),
    function<void(const Div*, Matcher* ctx)>([&](const Div* op, Matcher* ctx) {
      ctx->match(op->a);
      PathExpression a = peStack.top();
      peStack.pop();

      ctx->match(op->b);
      PathExpression b = peStack.top();
      peStack.pop();

      peStack.push(andPathExpressions(a, b, peVars, qvars));
      qvars.clear();
    })
  );

  iassert(peStack.size() == 1) << "incorrect stack size " << peStack.size();
  addPathExpression(target, peStack.top());
}

pe::PathExpression PathExpressionBuilder::getPathExpression(Var target) {
  return pathExpressions[target];
}

void PathExpressionBuilder::addPathExpression(Var target,
                                              const pe::PathExpression& pe) {
  pathExpressions.insert({target, pe});
}

const pe::Set& PathExpressionBuilder::getPathExpressionSet(Var irSetVar) {
  if (!util::contains(pathExpressionSets, irSetVar)) {
    pathExpressionSets.insert({irSetVar, pe::Set(irSetVar.getName())});
  }
  return pathExpressionSets.at(irSetVar);
}

}}
