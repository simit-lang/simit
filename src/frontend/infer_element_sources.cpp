#include <memory>
#include <string>

#include "infer_element_sources.h"
#include "fir.h"
#include "fir_rewriter.h"
#include "util/scopedmap.h"

namespace simit {
namespace fir {

void InferElementSources::visit(IdentDecl::Ptr decl) {
  decls.insert(decl->name->ident, decl->type);
}

void InferElementSources::visit(FuncDecl::Ptr decl) {
  decls.scope();
  
  for (const auto genericParam : decl->genericParams) {
    decls.insert(genericParam->name, Type::Ptr());
  }
  
  FIRVisitor::visit(decl);
  decls.unscope();
}

void InferElementSources::visit(VarDecl::Ptr decl) {
  decls.insert(decl->name->ident, Type::Ptr());
}

void InferElementSources::visit(WhileStmt::Ptr stmt) {
  stmt->cond->accept(this);
  
  decls.scope();
  stmt->body->accept(this);
  decls.unscope();
}
  
void InferElementSources::visit(IfStmt::Ptr stmt) {
  stmt->cond->accept(this);

  decls.scope();
  stmt->ifBody->accept(this);
  decls.unscope();

  if (stmt->elseBody) {
    decls.scope();
    stmt->elseBody->accept(this);
    decls.unscope();
  }
}

void InferElementSources::visit(ForStmt::Ptr stmt) {
  decls.scope();
  decls.insert(stmt->loopVar->ident, Type::Ptr());
 
  stmt->body->accept(this);
  decls.unscope();
}

void InferElementSources::visit(AssignStmt::Ptr stmt) {
  stmt->expr->accept(this);
  
  for (auto lhs : stmt->lhs) {
    lhs->accept(this);
    
    if (!isa<VarExpr>(lhs)) {
      continue;
    }
    
    const std::string varName = to<VarExpr>(lhs)->ident;
    
    if (!decls.contains(varName)) {
      decls.insert(varName, Type::Ptr());
    }
  }
}

void InferElementSources::visit(TensorReadExpr::Ptr expr) {
  FIRVisitor::visit(expr);

  // TODO: For now, we don't consider nested tensor reads as sparse inner 
  // blocks are not currently supported. When support for sparse inner blocks 
  // is added, this will also need to be fixed accordingly.
  if (!isa<VarExpr>(expr->tensor)) {
    return;
  }

  const auto var = to<VarExpr>(expr->tensor);
  const auto name = var->ident;
  
  if (!decls.contains(name) || !isa<NDTensorType>(decls.get(name))) {
    return;
  }

  const auto tensorType = to<NDTensorType>(decls.get(name));
  const unsigned numIndices = std::min(tensorType->indexSets.size(), 
                                       expr->indices.size());

  for (unsigned i = 0; i < numIndices; ++i) {
    if (!isa<ExprParam>(expr->indices[i])) {
      continue;
    }

    const auto idx = to<ExprParam>(expr->indices[i])->expr;

    std::string idxVarName;
    if (isa<VarExpr>(idx)) {
      idxVarName = to<VarExpr>(idx)->ident;
    } else if (isa<TupleReadExpr>(idx)) {
      const auto idxTupleRead = to<TupleReadExpr>(idx);
      if (isa<VarExpr>(idxTupleRead->tuple)) {
        idxVarName = to<VarExpr>(idxTupleRead->tuple)->ident;
      } else {
        continue;
      }
    } else {
      continue;
    }

    if (!decls.contains(idxVarName)) {
      continue;
    }

    ElementType::Ptr idxElemType;
    const auto idxVarType = decls.get(idxVarName);
    
    if (isa<ElementType>(idxVarType)) {
      idxElemType = to<ElementType>(idxVarType);
    } else if (isa<TupleType>(idxVarType)) {
      idxElemType = to<TupleType>(idxVarType)->element;
    } else {
      continue;
    }

    const auto indexSet = tensorType->indexSets[i];
    
    if (!idxElemType->source && isa<SetIndexSet>(indexSet)) {
      idxElemType->source = to<SetIndexSet>(indexSet);
    }
  }
}

}
}

