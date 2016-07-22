#include <memory>
#include <string>

#include "context_sensitive_rewriter.h"
#include "hir.h"
#include "hir_rewriter.h"
#include "intrinsics.h"

namespace simit {
namespace hir {

ContextSensitiveRewriter::ContextSensitiveRewriter(
    std::vector<ParseError> *errors) : errors(errors) {
  for (const auto kv : ir::intrinsics::byNames()) {
    decls.insert(kv.first, IdentType::FUNCTION);
  }
}
        
void ContextSensitiveRewriter::visit(SetIndexSet::Ptr set) {
  HIRRewriter::visit(set);

  const std::string setName = set->setName;
  
  if (decls.contains(setName)) {
    const auto type = decls.get(setName);

    switch (type) {
      case IdentType::GENERIC_PARAM:
      case IdentType::RANGE_GENERIC_PARAM:
      {
        auto newSet = std::make_shared<GenericIndexSet>();
        
        newSet->setLoc(set);
        newSet->setName = setName;
        newSet->type = (type == IdentType::RANGE_GENERIC_PARAM) ? 
                       GenericIndexSet::Type::RANGE :
                       GenericIndexSet::Type::UNKNOWN;
        
        node = newSet;
      }
      default:
        break;
    }
  }
}

void ContextSensitiveRewriter::visit(IdentDecl::Ptr decl) {
  HIRRewriter::visit(decl);
  
  const auto type = isa<TupleType>(decl->type) ? IdentType::TUPLE : 
                    IdentType::OTHER;
  decls.insert(decl->name->ident, type);
}

void ContextSensitiveRewriter::visit(FuncDecl::Ptr decl) {
  decls.insert(decl->name->ident, IdentType::FUNCTION);
  decls.scope();
  
  for (const auto genericParam : decl->genericParams) {
    const auto type = (genericParam->type == GenericParam::Type::RANGE) ?
                      IdentType::RANGE_GENERIC_PARAM : IdentType::GENERIC_PARAM;
    decls.insert(genericParam->name, type);
  }
  
  HIRRewriter::visit(decl);
  decls.unscope();
}

void ContextSensitiveRewriter::visit(VarDecl::Ptr decl) {
  HIRRewriter::visit(decl);
  decls.insert(decl->name->ident, IdentType::OTHER);
}

void ContextSensitiveRewriter::visit(WhileStmt::Ptr stmt) {
  stmt->cond = rewrite<Expr>(stmt->cond);
  
  decls.scope();
  stmt->body = rewrite<StmtBlock>(stmt->body);
  decls.unscope();
  
  node = stmt;
}
  
void ContextSensitiveRewriter::visit(IfStmt::Ptr stmt) {
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

void ContextSensitiveRewriter::visit(ForStmt::Ptr stmt) {
  decls.scope();
  stmt->loopVar = rewrite<Identifier>(stmt->loopVar);
  stmt->domain = rewrite<ForDomain>(stmt->domain);
  decls.insert(stmt->loopVar->ident, IdentType::OTHER);
  
  stmt->body = rewrite<StmtBlock>(stmt->body);
  decls.unscope();

  node = stmt;
}

void ContextSensitiveRewriter::visit(AssignStmt::Ptr stmt) {
  stmt->expr = rewrite<Expr>(stmt->expr);
  
  for (auto &lhs : stmt->lhs) {
    lhs = rewrite<Expr>(lhs);
    
    if (!isa<VarExpr>(lhs)) {
      continue;
    }
    
    const std::string varName = to<VarExpr>(lhs)->ident;
    
    if (!decls.contains(varName)) {
      decls.insert(varName, IdentType::OTHER);
    }
  }

  node = stmt;
}

void ContextSensitiveRewriter::visit(TensorReadExpr::Ptr expr) {
  HIRRewriter::visit(expr);
  
  if (!isa<VarExpr>(expr->tensor)) {
    return;
  }

  const auto var = to<VarExpr>(expr->tensor);
  const auto name = var->ident;
  
  if (!decls.contains(name)) {
    return;
  }
    
  switch (decls.get(name)) {
    case IdentType::FUNCTION:
    {
      auto func = std::make_shared<Identifier>();
      func->setLoc(var);
      func->ident = name;

      auto call = std::make_shared<CallExpr>();
      call->setLoc(expr);
      call->func = func;

      for (auto param : expr->indices) {
        if (isa<ExprParam>(param)) {
          call->args.push_back(to<ExprParam>(param)->expr);
        } else {
          reportError("argument to function call must be an expression", param);
          call->args.push_back(Expr::Ptr());
        }
      }

      node = call;
      break;
    }
    case IdentType::TUPLE:
    {
      auto tupleRead = std::make_shared<TupleReadExpr>();
      tupleRead->setLoc(expr);
      tupleRead->tuple = expr->tensor;
      
      if (expr->indices.size() != 1) {
        std::stringstream errMsg;
        errMsg << "tuple access expects exactly one index but got " 
               << expr->indices.size();
        reportError(errMsg.str(), expr);
      } else if (expr->indices[0]->isSlice()) {
        reportError("tuple access expects an integral index", expr->indices[0]);
      } else {
        tupleRead->index = to<ExprParam>(expr->indices[0])->expr;
      }
      
      node = tupleRead;
      break;
    }
    default:
      break;
  }
}

void ContextSensitiveRewriter::visit(VarExpr::Ptr expr) {
  HIRRewriter::visit(expr);

  if (decls.contains(expr->ident) && 
      decls.get(expr->ident) == IdentType::RANGE_GENERIC_PARAM) {
    const auto rangeConst = std::make_shared<RangeConst>();
    rangeConst->setLoc(expr);
    rangeConst->ident = expr->ident;
    node = rangeConst;
  }
}

void ContextSensitiveRewriter::reportError(std::string msg, HIRNode::Ptr loc) {
  const auto err = ParseError(loc->getLineBegin(), loc->getColBegin(), 
                              loc->getLineEnd(), loc->getColEnd(), msg);
  errors->push_back(err);
}

}
}

