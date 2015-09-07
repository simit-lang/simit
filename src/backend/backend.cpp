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

backend::Function* Backend::compile(const ir::Func& func) {
  return compile(func, Storage());
}

Function* Backend::compile(const Func& func, const Storage& storage) {
  return pimpl->compile(func, storage);
}

backend::Function* Backend::compile(const Stmt& stmt, const Environment& env) {
  return compile(stmt, env, Storage());
}

backend::Function* Backend::compile(const Stmt& stmt, const Environment& env,
                                    const Storage& storage) {
  Environment environment = env; // copy

  // Add output and undefined variable in the stmt to the environment as externs
  std::set<Var> globalsSet;
  util::ScopedMap<Var,void*> definedVariables;

  // Add all environment variables to defined variables
  for (auto& var : environment.getConstants()) {
    globalsSet.insert(var.first);
    definedVariables.insert(var.first, nullptr);
  }
  for (auto& ext : environment.getExterns()) {
    const Var& var = ext.getVar();
    globalsSet.insert(var);
    definedVariables.insert(var, nullptr);
  }
  for (auto& tmp : environment.getTemporaries()) {
    const Var& var = tmp.getVar();
    globalsSet.insert(var);
    definedVariables.insert(var, nullptr);
  }

  // Find all undefined variables and add them to the context as globals.
  function<void(Var var)> addToGlobalsIfNotDefined = [&](Var var) {
    if (!definedVariables.contains(var) && !util::contains(globalsSet, var)) {
      environment.addExtern(var);
      globalsSet.insert(var);
      definedVariables.insert(var, nullptr);
    }
  };

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

  Func func("main", {}, {}, stmt, environment);
  return pimpl->compile(func, storage);
}

Function* Backend::compile(const Stmt& stmt, vector<ir::Var> output){
  Environment env;

  std::set<Var> externSet;
  for (const Var& var : output) {
    if (!util::contains(externSet, var)) {  // Remove duplicates
      externSet.insert(var);
      env.addExtern(var);
    }
  }

  return compile(stmt, env, Storage());
}

}}
