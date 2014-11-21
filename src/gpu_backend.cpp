#include "gpu_backend.h"

#include "gpu_function.h"

namespace simit {
namespace internal {

GPUBackend::GPUBackend() {}
GPUBackend::~GPUBackend() {}

simit::Function *GPUBackend::compile(simit::ir::Func func) {
  // NOTE(gkanwar): What does this actually do?
  func.getBody().accept(this);

  return new GPUFunction(func);
}

}
}
