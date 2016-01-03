#include <memory>
#include <string>
#include <vector>

#include "func_call_rewriter.h"
#include "hir.h"
#include "ir.h"
#include "program_context.h"

namespace simit {
namespace hir {

void FuncCallRewriter::visit(FuncDecl::Ptr decl) {
  HIRRewriter::visit(decl);
  
  ctx.addFunction(ir::Func(decl->name->ident, {}, {}, ir::Stmt()));
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

  for (auto param : expr->indices) {
    if (isa<ExprParam>(param)) {
      call->arguments.push_back(to<ExprParam>(param)->expr);
    } else {
      reportError("argument to function call must be an expression", param);
      call->arguments.push_back(Expr::Ptr());
    }
  }

  node = call;
}

}
}

