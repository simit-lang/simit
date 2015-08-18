#include "backend.h"

#include "ir.h"

using namespace simit::ir;

namespace simit {
namespace backend {

backend::Function* Backend::compile(const Stmt &stmt, const Environment &env) {
  Func func("main", {}, {}, stmt);
  func.setEnvironment(env);
  return compile(func);
}

}}
