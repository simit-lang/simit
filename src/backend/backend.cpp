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
  match(func,
    function<void(const VarDecl*)>([&](const VarDecl* op) {
      definedVariables.insert(op->var, op->var);
    })
    ,
    function<void(const ForRange*)>([&](const ForRange* op) {
      definedVariables.insert(op->var,op->var);
    })
    ,
    function<void(const For*)>([&](const For* op) {
      definedVariables.insert(op->var,op->var);
    })
    ,
    function<void(const AssignStmt*)>([&](const AssignStmt* op) {
      addToGlobalsIfNotDefined(op->var);
    })
    ,
    function<void(const VarExpr*)>([&](const VarExpr* op) {
      addToGlobalsIfNotDefined(op->var);
    })
    ,
    function<void(const Block*)>([&](const Block* op) {
      // TODO: Figure out how to scope/unscope symtable. Either add it as a
      //       match parameter, and let it scope/unscope it, or add support
      //       for returning a void lambda that executes after children have
      //       been visited
    })
  );

  return pimpl->compile(func, globals);
}

Function* Backend::compile(const Func& func) {
  return pimpl->compile(func, {});
}

}}
