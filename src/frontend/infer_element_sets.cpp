#include <memory>
#include <string>

#include "infer_element_sets.h"
#include "hir.h"
#include "hir_rewriter.h"
#include "intrinsics.h"
#include "ir.h"

namespace simit {
namespace hir {

void InferElementSets::visit(IdentDecl::Ptr decl) {
  decls.insert(decl->name->ident, decl->type);
}

void InferElementSets::visit(FuncDecl::Ptr decl) {
  decls.insert(decl->name->ident, Type::Ptr());
  decls.scope();
  for (const auto typeParam : decl->typeParams) {
    decls.insert(typeParam->ident, Type::Ptr());
  }
  HIRVisitor::visit(decl);
  decls.unscope();
}

void InferElementSets::visit(WhileStmt::Ptr stmt) {
  stmt->cond->accept(this);
  
  decls.scope();
  stmt->body->accept(this);
  decls.unscope();
}
  
void InferElementSets::visit(IfStmt::Ptr stmt) {
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

void InferElementSets::visit(ForStmt::Ptr stmt) {
  decls.scope();
  HIRVisitor::visit(stmt);
  decls.unscope();
}

void InferElementSets::visit(AssignStmt::Ptr stmt) {
  stmt->expr->accept(this);
  for (auto &lhs : stmt->lhs) {
    lhs->accept(this);
    if (isa<VarExpr>(lhs)) {
      const std::string varName = to<VarExpr>(lhs)->ident;
      if (!decls.contains(varName)) {
        decls.insert(varName, Type::Ptr());
      }
    }
  }
}

void InferElementSets::visit(TensorReadExpr::Ptr expr) {
  HIRVisitor::visit(expr);

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
    
    if (isa<SetIndexSet>(indexSet)) {
      const auto indexSetName = to<SetIndexSet>(indexSet)->setName;
      if (isa<GenericIndexSet>(indexSet)) {
        idxElemType->sourceGenericSets.insert(indexSetName);
      } else if (idxElemType->sourceSet.empty()) {
        idxElemType->sourceSet = indexSetName;
      } else if (idxElemType->sourceSet != indexSetName) {
        std::stringstream errMsg;
        errMsg << idxVarName << " cannot be an element of distinct sets "
               << idxElemType->sourceSet << " and " << indexSetName;
        reportError(errMsg.str(), expr);
      }
    }
  }
}

void InferElementSets::reportError(std::string msg, HIRNode::Ptr loc) {
  const auto err = ParseError(loc->getLineBegin(), loc->getColBegin(), 
                              loc->getLineEnd(), loc->getColEnd(), msg);
  errors->push_back(err);
}

}
}
