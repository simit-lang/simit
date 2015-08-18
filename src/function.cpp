#include "function.h"

#include "backend/backend_function.h"

namespace simit {

// class Function
Function::Function() : Function(nullptr) {
}

Function::Function(backend::Function *func) : impl(func), funcPtr(nullptr) {
}

void Function::bind(const std::string& argName,
                    const TensorType& ttype, void* data) {
  not_supported_yet;
}

void Function::bind(const std::string &argName, simit::Tensor *tensor) {
  uassert(defined()) << "undefined function";
  impl->bind(argName, tensor);
}

void Function::bind(const std::string &argName, simit::Set *set) {
  uassert(defined()) << "undefined function";
  impl->bind(argName, set);
}

void Function::init() {
  uassert(defined()) << "undefined function";
  impl->init();
  funcPtr = impl->getFunctionHandle();
}

bool Function::isInit() {
  uassert(defined()) << "undefined function";
  return impl->isInit();
}

void Function::runSafe() {
  uassert(defined()) << "undefined function";
  impl->runSafe();
}

void Function::mapArgs() {
  uassert(defined()) << "undefined function";
  impl->mapArgs();
}

void Function::unmapArgs(bool updated) {
  uassert(defined()) << "undefined function";
  impl->unmapArgs(updated);
}

void Function::printMachine(std::ostream &os) {
  impl->printMachine(os);
}

std::ostream &operator<<(std::ostream &os, const Function &f) {
  return os << *f.impl;
}

}
