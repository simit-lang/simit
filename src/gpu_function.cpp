#include "gpu_function.h"

namespace simit {
namespace internal {

GPUFunction::GPUFunction(simit::ir::Func simitFunc) : Function(simitFunc) {}
GPUFunction::~GPUFunction() {}

void GPUFunction::print(std::ostream &os) const {
  os << "Hello, world!" << std::endl;
}

simit::Function::FuncPtrType GPUFunction::init(
    const std::vector<std::string> &formals,
    std::map<std::string, Actual> &actuals) {
  return [](){};
}

}
}
