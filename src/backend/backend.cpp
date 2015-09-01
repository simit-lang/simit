#include "backend.h"

#include "backend_impl.h"
#include "backend/llvm/llvm_backend.h"
#ifdef GPU
#include "backend/gpu/gpu_backend.h"
#endif

#include "ir.h"
#include "ir_visitor.h"
#include "util/scopedmap.h"
#include "util/collections.h"

using namespace std;
using namespace simit::ir;

namespace simit {
namespace backend {

BackendImpl* getBackendImpl(const std::string &type) {
  if (type == "llvm") {
    return new backend::LLVMBackend();
  }
#ifdef GPU
  if (type == "gpu") {
    return new backend::GPUBackend();
  }
#endif

  ierror << "Invalid backend choice: " << type << ". "
         << "Did you forget to call simit::init()?";
  return nullptr;
}

// class Backend
Backend::Backend(const std::string &type) : pimpl(getBackendImpl(type)) {
}

Backend::~Backend() {
  delete pimpl;
}

Function* Backend::compile(const Func& func) {
  return pimpl->compile(func);
}

backend::Function* Backend::compile(const Stmt& stmt) {
  return compile(stmt, {});
}

Function* Backend::compile(const Stmt& stmt, vector<ir::Var> output){
  Environment env;

  // Add output and undefined variable in the stmt to the environment as externs
  std::set<Var> globalsSet;
  util::ScopedMap<Var,void*> definedVariables;
  function<void(Var var)> addToGlobalsIfNotDefined = [&](Var var) {
    if (!definedVariables.contains(var) && !util::contains(globalsSet, var)) {
      env.addExtern(var);
      globalsSet.insert(var);
      definedVariables.insert(var, nullptr);
    }
  };

  // Remove duplicates in output
  for (const Var& var : output) {
    addToGlobalsIfNotDefined(var);
  }

  // Find all undefined variables and add them to the context as globals.
  match(stmt,
    function<void(const VarExpr*)>([&](const VarExpr* op) {
      addToGlobalsIfNotDefined(op->var);
    })
    ,
    function<void(const AssignStmt*)>([&](const AssignStmt* op) {
      definedVariables.insert(op->var, nullptr);
    })
    ,
    function<void(const VarDecl*)>([&](const VarDecl* op) {
      definedVariables.insert(op->var, nullptr);
    })
    ,
    function<void(const ForRange*,Matcher*)>([&](const ForRange* op, Matcher* ctx) {
      ctx->match(op->start);
      ctx->match(op->end);
      definedVariables.insert(op->var, nullptr);
      ctx->match(op->body);
    })
    ,
    function<void(const For*,Matcher*)>([&](const For* op, Matcher* ctx) {
      definedVariables.insert(op->var, nullptr);
      ctx->match(op->body);
    })
    ,
    function<void(const Scope*,Matcher*)>([&](const Scope* op, Matcher* ctx) {
      definedVariables.scope();
      ctx->match(op->scopedStmt);
      definedVariables.unscope();
    })
  );

  Func func("main", {}, {}, stmt, env);
  return pimpl->compile(func);
}

}}
