#include "function.h"

#include "backend/backend_function.h"
#include "types_convert.h"

using namespace std;

namespace simit {

// class Function
Function::Function() : Function(nullptr) {
}

Function::Function(backend::Function *func) : impl(func), funcPtr(nullptr) {
}

void Function::bind(string argumentName, const TensorType& ttype, void* data) {
  impl->bindTensorData(argumentName, ir::convert(ttype), data);
}

void Function::bind(string argumentName, void* data) {
  impl->bindTensorData(argumentName, data);
}

void Function::bind(string argumentName, simit::Set *set) {
  uassert(defined()) << "undefined function";
  impl->bind(argumentName, set);
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
