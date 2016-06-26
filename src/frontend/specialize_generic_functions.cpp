#include <sstream>
#include <unordered_map>

#include "ir.h"
#include "specialize_generic_functions.h"

namespace simit {
namespace hir {

void SpecializeGenericFunctions::specialize(Program::Ptr program) {
  FindGenericFuncs(genericFuncs).find(program);
  program->accept(this);
}

void SpecializeGenericFunctions::visit(Program::Ptr program) {
  for (auto it = program->elems.crbegin(); it != program->elems.crend(); ++it) {
    (*it)->accept(this);
  }
  
  std::vector<HIRNode::Ptr> newElems;
  for (const auto &elem : program->elems) {
    newElems.push_back(elem);
    
    if (!isa<FuncDecl>(elem)) {
      continue;
    }

    const std::string funcName = to<FuncDecl>(elem)->name->ident;
    if (genericFuncs.find(funcName) != genericFuncs.end()) {
      const auto it = specializedFuncs.find(funcName);
      if (it != specializedFuncs.end()) {
        newElems.insert(newElems.end(), it->second.begin(), it->second.end());
      }
    }
  }
  program->elems = newElems;
}

void SpecializeGenericFunctions::visit(FuncDecl::Ptr decl) {
  if (decl->typeParams.size() == 0 || !decl->originalName.empty()) {
    HIRVisitor::visit(decl);
  }
}

void SpecializeGenericFunctions::visit(CallExpr::Ptr expr) {
  HIRVisitor::visit(expr);

  const auto it = genericFuncs.find(expr->func->ident);
  if (it != genericFuncs.end()) {
    std::stringstream newName;
    newName << expr->func->ident << "_" << (++count);
    expr->func->ident = newName.str();
    
    const auto newFunc = it->second->clone<FuncDecl>();
    if (newFunc->originalName.empty()) {
      newFunc->originalName = newFunc->name->ident;
    }
    newFunc->name->ident = newName.str();
    specializedFuncs[newFunc->originalName].push_back(newFunc);
    genericFuncs[newName.str()] = newFunc;
    newFunc->accept(this);
  }
}

void SpecializeGenericFunctions::FindGenericFuncs::visit(FuncDecl::Ptr decl) {
  if (decl->typeParams.size() > 0) {
    genericFuncs[decl->name->ident] = decl;
  }
}

}
}

