#include "backend.h"

#include "ir.h"
#include "ir_visitor.h"
#include "util/scopedmap.h"

using namespace std;
using namespace simit::ir;

namespace simit {
namespace backend {

// class Backend
Function* Backend::compile(const Stmt& stmt) {
  Func func("main", {}, {}, stmt);

  util::ScopedMap<Var,Var> definedVariables;
  std::vector<Var> globals;
  function<void(Var var)> addToGlobalsIfNotDefined = [&](Var var) {
    if (!definedVariables.contains(var)) {
      globals.push_back(var);
    }
  };

  // Find all undefined variables and add them to the context as globals.
  match<Func>(func, {
    {"VarDecl", [&](const IRNode* irNode) -> bool {
      iassert(dynamic_cast<const VarDecl*>(irNode));
      const VarDecl* op = static_cast<const VarDecl*>(irNode);
      definedVariables.insert(op->var,op->var);
      return true;
    }},
    {"ForRange", [&](const IRNode* irNode) -> bool {
      iassert(dynamic_cast<const ForRange*>(irNode));
      const ForRange* op = static_cast<const ForRange*>(irNode);
      definedVariables.insert(op->var,op->var);
      return true;
    }},
    {"For", [&](const IRNode* irNode) -> bool {
      iassert(dynamic_cast<const For*>(irNode));
      const For* op = static_cast<const For*>(irNode);
      definedVariables.insert(op->var,op->var);
      return true;
    }},
    {"AssignStmt", [&](const IRNode* irNode) -> bool {
      iassert(dynamic_cast<const AssignStmt*>(irNode));
      const AssignStmt* op = static_cast<const AssignStmt*>(irNode);
      addToGlobalsIfNotDefined(op->var);
      return true;
    }},
    {"VarExpr", [&](const IRNode* irNode) -> bool {
      iassert(dynamic_cast<const VarExpr*>(irNode));
      const VarExpr* op = static_cast<const VarExpr*>(irNode);
      addToGlobalsIfNotDefined(op->var);
      return true;
    }},
  });

  return compile(func, globals);
}

Function* Backend::compile(const Func& func) {
  return compile(func, {});
}

}}
