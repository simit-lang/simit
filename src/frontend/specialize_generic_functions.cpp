#include <sstream>
#include <unordered_map>

#include "ir.h"
#include "specialize_generic_functions.h"

namespace simit {
namespace hir {

void SpecializeGenericFunctions::specialize(Program::Ptr program) {
  program->accept(this);

  std::vector<HIRNode::Ptr> newElems;
  for (const auto &elem : program->elems) {
    newElems.push_back(elem);
    
    if (!isa<FuncDecl>(elem)) {
      continue;
    }

    const std::string funcName = to<FuncDecl>(elem)->name->ident;
    if (genericFuncs.find(funcName) != genericFuncs.end()) {
      const auto it = specializedFuncs.find(funcName);
      newElems.insert(newElems.end(), it->second.begin(), it->second.end());
    }
  }
  program->elems = newElems;
}

void SpecializeGenericFunctions::visit(FuncDecl::Ptr decl) {
  HIRVisitor::visit(decl);

  if (decl->typeParams.size() > 0) {
    genericFuncs[decl->name->ident] = decl;
  }
}

void SpecializeGenericFunctions::visit(CallExpr::Ptr expr) {
  HIRVisitor::visit(expr);

  const auto it = genericFuncs.find(expr->func->ident);
  if (it != genericFuncs.end()) {
    std::stringstream newName;
    newName << expr->func->ident << "_" << expr->getLineBegin()
            << "_" << expr->getColBegin();
    expr->func->ident = newName.str();
    
    const auto newFunc = it->second->clone<FuncDecl>();
    newFunc->name->ident = newName.str();
    newFunc->doTypeCheck = false;
    specializedFuncs[it->first].push_back(newFunc);
  }
}

}
}

