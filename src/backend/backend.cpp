#include "backend.h"

#include "backend_impl.h"
#include "backend/llvm/llvm_backend.h"
#ifdef GPU
#include "backend/gpu/gpu_backend.h"
#endif

#include "ir.h"
#include "ir_visitor.h"
#include "util/scopedmap.h"

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

Function* Backend::compile(const Stmt& stmt, std::set<ir::Var> output) {
  Func func("main", {}, {}, stmt);
  std::set<Var> globals = output;

  // Find all undefined variables and add them to the context as globals.
  util::ScopedMap<Var,void*> definedVariables;
  function<void(Var var)> addToGlobalsIfNotDefined = [&](Var var) {
    if (!definedVariables.contains(var)) {
      globals.insert(var);
    }
  };
  match(func,
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
      definedVariables.scope();
      ctx->match(op->start);
      ctx->match(op->end);
      definedVariables.insert(op->var, nullptr);
      ctx->match(op->body);
      definedVariables.unscope();
    })
    ,
    function<void(const For*,Matcher*)>([&](const For* op, Matcher* ctx) {
      definedVariables.scope();
      definedVariables.insert(op->var, nullptr);
      ctx->match(op->body);
      definedVariables.unscope();
    })
    ,
    function<void(const Block*,Matcher*)>([&](const Block* op, Matcher* ctx) {
      definedVariables.scope();
      ctx->match(op->first);
      if (op->rest.defined()) {
        ctx->match(op->rest);
      }
      definedVariables.unscope();
    })
  );

  return pimpl->compile(func, globals);
}

Function* Backend::compile(const Func& func) {
  return pimpl->compile(func, {});
}

}}
