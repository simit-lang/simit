#include <sstream>
#include <unordered_map>

#include "specialize_generic_functions.h"
#include "util/collections.h"

namespace simit {
namespace fir {

void SpecializeGenericFunctions::specialize(Program::Ptr program) {
  FindGenericFuncs(genericFuncs).find(program);
  for (auto intrinsic : intrinsics) {
    if (intrinsic->genericParams.size() > 0) {
      genericFuncs[intrinsic->name->ident] = intrinsic;
    }
  }

  program->accept(this);
}

void SpecializeGenericFunctions::visit(Program::Ptr program) {
  for (auto it = program->elems.crbegin(); it != program->elems.crend(); ++it) {
    (*it)->accept(this);
  }

  std::vector<FIRNode::Ptr> newElems;
  for (auto intrinsic : intrinsics) {
    const std::string funcName = to<FuncDecl>(intrinsic)->name->ident;
    if (genericFuncs.find(funcName) != genericFuncs.end()) {
      const auto it = specializedFuncs.find(funcName);
      if (it != specializedFuncs.end()) {
        newElems.insert(newElems.end(), it->second.begin(), it->second.end());
      }
    }
  }

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
  if (decl->genericParams.size() == 0 || !decl->originalName.empty()) {
    FIRVisitor::visit(decl);
  }
}

void SpecializeGenericFunctions::visit(CallExpr::Ptr expr) {
  FIRVisitor::visit(expr);
  cloneIfGeneric(expr->func);
}

void SpecializeGenericFunctions::visit(MapExpr::Ptr expr) {
  FIRVisitor::visit(expr);
  cloneIfGeneric(expr->func);
}

void SpecializeGenericFunctions::clone(FuncDecl::Ptr decl, 
                                       const std::string &newName) {
  const auto newFunc = decl->clone<FuncDecl>();

  if (newFunc->originalName.empty()) {
    newFunc->originalName = newFunc->name->ident;
  }
  newFunc->name->ident = newName;

  specializedFuncs[newFunc->originalName].push_back(newFunc);
  genericFuncs[newName] = newFunc;

  newFunc->accept(this);
}

void
SpecializeGenericFunctions::cloneIfGeneric(const Identifier::Ptr &funcName) {
  const auto it = genericFuncs.find(funcName->ident);
  if (it != genericFuncs.end()) {
    std::stringstream newName;
    newName << funcName->ident << "@" << (++count);
    funcName->ident = newName.str();
    clone(it->second, newName.str());
  }
}

void SpecializeGenericFunctions::FindGenericFuncs::visit(FuncDecl::Ptr decl) {
  if (decl->genericParams.size() > 0) {
    genericFuncs[decl->name->ident] = decl;
  }
}

}
}

