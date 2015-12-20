#include <memory>
#include <string>
#include <set>
#include <vector>

#include "func_call_rewriter.h"
#include "hir.h"

namespace simit {
namespace hir {

void FuncCallRewriter::visit(FuncDecl::Ptr decl) {
  funcs.insert(decl->name);
  HIRRewriter::visit(decl);
}

void FuncCallRewriter::visit(TensorReadExpr::Ptr expr) {
  for (unsigned i = 0; i < expr->indices.size(); ++i) {
    expr->indices[i] = rewrite<ReadParam>(expr->indices[i]);
  }

  node = expr;
  if (!isa<VarExpr>(expr->tensor)) {
    return;
  }
  
  const auto var = to<VarExpr>(expr->tensor);
  if (funcs.find(var->ident) == funcs.end()) {
    return;
  }

  auto call = std::make_shared<CallExpr>();
  call->setLoc(expr);
  call->funcName = var->ident;

  bool validCall = true;
  for (auto param : expr->indices) {
    if (isa<ExprParam>(param)) {
      call->operands.push_back(to<ExprParam>(param)->expr);
    } else {
      const auto err = ParseError(param->lineNum, param->colNum,
        param->lineNum, param->colNum, "Invalid argument to function call");
      errors->push_back(err);
      validCall = false;  
    }
  }

  if (validCall) {
    node = call;
  }
}

}
}

