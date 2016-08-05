#include <sstream>
#include <unordered_map>

#include "clone_generic_functions.h"
#include "util/collections.h"

namespace simit {
namespace fir {

void CloneGenericFunctions::specialize(Program::Ptr program) {
  FindGenericFuncs(genericFuncs).find(program);
  
  for (auto intrinsic : intrinsics) {
    if (intrinsic->genericParams.size() > 0) {
      genericFuncs[intrinsic->name->ident] = intrinsic;
    }
  }

  program->accept(this);
}

void CloneGenericFunctions::visit(Program::Ptr program) {
  for (auto it = program->elems.crbegin(); it != program->elems.crend(); ++it) {
    const auto elem = *it;
    
    if (isa<FuncDecl>(elem)) {
      currentFunc = to<FuncDecl>(elem)->name->ident;
    }

    elem->accept(this);
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
    if (isa<FuncDecl>(elem)) {
      const std::string funcName = to<FuncDecl>(elem)->name->ident;
      const auto it = specializedFuncs.find(funcName);
      
      if (it != specializedFuncs.end()) {
        newElems.insert(newElems.end(), it->second.rbegin(), it->second.rend());
      }
    }

    newElems.push_back(elem);
  }

  program->elems = newElems;
}

void CloneGenericFunctions::visit(FuncDecl::Ptr decl) {
  if (decl->genericParams.size() == 0 || !decl->originalName.empty()) {
    FIRVisitor::visit(decl);
  }
}

void CloneGenericFunctions::visit(CallExpr::Ptr expr) {
  FIRVisitor::visit(expr);
  cloneIfGeneric(expr->func);
}

void CloneGenericFunctions::visit(MapExpr::Ptr expr) {
  FIRVisitor::visit(expr);
  cloneIfGeneric(expr->func);
}

void CloneGenericFunctions::clone(FuncDecl::Ptr decl,
                                  const std::string &newName) {
  const auto newFunc = decl->clone<FuncDecl>();

  if (newFunc->originalName.empty()) {
    newFunc->originalName = newFunc->name->ident;
  }
  newFunc->name->ident = newName;

  specializedFuncs[currentFunc].push_back(newFunc);
  genericFuncs[newName] = newFunc;

  newFunc->accept(this);
}

void CloneGenericFunctions::cloneIfGeneric(const Identifier::Ptr &funcName) {
  const auto it = genericFuncs.find(funcName->ident);
  if (it != genericFuncs.end()) {
    std::stringstream newName;
    newName << funcName->ident << "@" << (++count);
    
    funcName->ident = newName.str();
    clone(it->second, newName.str());
  }
}

void CloneGenericFunctions::FindGenericFuncs::visit(FuncDecl::Ptr decl) {
  if (decl->genericParams.size() > 0) {
    genericFuncs[decl->name->ident] = decl;
  }
}

}
}

