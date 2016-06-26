#include <memory>
#include <string>

#include "hir.h"
#include "ir.h"
#include "hir_rewriter.h"
#include "type_param_rewriter.h"
#include "util/scopedmap.h"

namespace simit {
namespace hir {
        
void TypeParamRewriter::visit(SetIndexSet::Ptr set) {
  const std::string setName = set->setName;
  if (decls.contains(setName) && decls.get(setName)) {
    auto newSet = std::make_shared<GenericIndexSet>();
    newSet->setLoc(set);
    newSet->setName = setName;
    node = newSet;
  } else {
    node = set;
  }
}

void TypeParamRewriter::visit(IdentDecl::Ptr decl) {
  HIRRewriter::visit(decl);
  decls.insert(decl->name->ident, false);
}

void TypeParamRewriter::visit(FuncDecl::Ptr decl) {
  if (decl->typeParams.size() == 0) {
    node = decl;
    return;
  }

  decls.scope();
  for (const auto typeParam : decl->typeParams) {
    decls.insert(typeParam->ident, true);
  }
  HIRRewriter::visit(decl);
  decls.unscope();
}

void TypeParamRewriter::visit(WhileStmt::Ptr stmt) {
  stmt->cond = rewrite<Expr>(stmt->cond);
  
  decls.scope();
  stmt->body = rewrite<StmtBlock>(stmt->body);
  decls.unscope();
  
  node = stmt;
}
  
void TypeParamRewriter::visit(IfStmt::Ptr stmt) {
  stmt->cond = rewrite<Expr>(stmt->cond);

  decls.scope();
  stmt->ifBody = rewrite<Stmt>(stmt->ifBody);
  decls.unscope();

  if (stmt->elseBody) {
    decls.scope();
    stmt->elseBody = rewrite<Stmt>(stmt->elseBody);
    decls.unscope();
  }

  node = stmt;
}

void TypeParamRewriter::visit(ForStmt::Ptr stmt) {
  decls.scope();
  stmt->loopVar = rewrite<Identifier>(stmt->loopVar);
  stmt->domain = rewrite<ForDomain>(stmt->domain);
  decls.insert(stmt->loopVar->ident, false);
  
  stmt->body = rewrite<StmtBlock>(stmt->body);
  decls.unscope();

  node = stmt;
}

void TypeParamRewriter::visit(AssignStmt::Ptr stmt) {
  for (auto &lhs : stmt->lhs) {
    lhs = rewrite<Expr>(lhs);
    if (isa<VarExpr>(lhs)) {
      const std::string varName = to<VarExpr>(lhs)->ident;
      if (!decls.contains(varName)) {
        decls.insert(varName, false);
      }
    }
  }

  stmt->expr = rewrite<Expr>(stmt->expr);
  node = stmt;
}

}
}

