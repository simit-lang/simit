#include <memory>
#include <string>
#include <set>
#include <vector>

#include "func_call_rewriter.h"
#include "hir.h"
#include "ir.h"
#include "program_context.h"

namespace simit {
namespace hir {

void FuncCallRewriter::visit(FuncDecl::Ptr decl) {
  ctx.addFunction(ir::Func(decl->name->ident, {}, {}, ir::Stmt()));
  
  HIRRewriter::visit(decl);
}

void FuncCallRewriter::visit(TensorReadExpr::Ptr expr) {
  HIRRewriter::visit(expr);
  
  if (!isa<VarExpr>(expr->tensor)) {
    return;
  }

  const auto var = to<VarExpr>(expr->tensor);
  if (!ctx.containsFunction(var->ident)) {
    return;
  }

  auto func = std::make_shared<Identifier>();
  func->setLoc(var);
  func->ident = var->ident;

  auto call = std::make_shared<CallExpr>();
  call->setLoc(expr);
  call->func = func;

  bool validCall = true;
  for (auto param : expr->indices) {
    if (isa<ExprParam>(param)) {
      call->operands.push_back(to<ExprParam>(param)->expr);
    } else {
      reportError("argument to function call must be an expression", param);
      validCall = false; 
    }
  }

  if (validCall) {
    node = call;
  }
}

}
}

