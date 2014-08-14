#include "program.h"

#include <memory>

#include "ir.h"
#include "frontend.h"
#include "codegen.h"
#include "util.h"
#include "errors.h"

using namespace std;
using namespace simit;
using namespace simit::internal;

namespace simit {
namespace internal {
class ProgramContent {
 public:
  ProgramContent(const std::string &name)
      : name(name), frontend(new Frontend()), codegen(NULL) {}
  ~ProgramContent() {
    for (auto function : functions) {
      delete function;
    }
    delete frontend;
    delete codegen;
  }

  const std::string &name;
  std::vector<Function*> functions;
  std::vector<simit::Error> errors;
  std::vector<Test> tests;

  Frontend *getFrontend() { return frontend; }
  CodeGen *getCodeGen() {
    if (codegen == NULL) {
      codegen = new LLVMCodeGen();
    }
    return codegen;
  }

 private:
  Frontend *frontend;
  LLVMCodeGen *codegen;
};
}
}

/* Program */
Program::Program(const std::string &name) : impl(new ProgramContent(name)) {
}

Program::~Program() {
  delete impl;
}

std::string Program::getName() const {
  return impl->name;
}

int Program::loadString(const string &programString) {
  return impl->getFrontend()->parseString(programString, &impl->functions,
                                          &impl->errors, &impl->tests);
}

int Program::loadFile(const std::string &filename) {
  return impl->getFrontend()->parseFile(filename, &impl->functions,
                                        &impl->errors, &impl->tests);
}

int Program::compile() {
//  for (auto function : impl->functions) {
//    impl->getCodeGen()->compileToFunctionPointer(function);
//  }
  return 0;
}

int Program::run() {
  return 0;
}

string Program::getErrorString() {
  return util::join(impl->errors, "\n");
}

std::vector<Error> &Program::getErrors() {
  return impl->errors;
}

std::ostream &simit::operator<<(std::ostream &os, const Program &program) {
  os << util::join(program.impl->functions, "\n\n") << endl;
  return os;
}
