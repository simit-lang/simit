#include "function.h"

#include <iostream>
using namespace std;

#include "graph.h"
#include "types.h"
#include "ir.h"

namespace simit {

Function::Function(const simit::ir::Function &simitFunc)
    : funcPtr(NULL), initRequired(true) {
  for (auto &argument : simitFunc.getArguments()) {
    actuals[argument->getName()] = Actual(argument->getType());
  }
  for (auto &result : simitFunc.getResults()) {
    actuals[result->getName()] = Actual(result->getType());
  }
}

Function::~Function() {
}

void Function::bind(const std::string &argName, Tensor *tensor) {
  assert(actuals.find(argName) != actuals.end() &&
         "no argument of this name in function");
  assert(*tensor->getType() == *actuals[argName].getType() &&
         "tensor type does not match function argument type");
  actuals[argName].bind(tensor);
  initRequired = true;
}

void Function::bind(const std::string &argName, SetBase *set) {
  NOT_SUPPORTED_YET;
  initRequired = true;
}

} // namespace simit
