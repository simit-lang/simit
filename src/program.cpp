#include "program.h"

#include <set>

#include "ir.h"
#include "frontend.h"
#include "llvm_codegen.h"
#include "function.h"
#include "util.h"
#include "errors.h"
#include "program_context.h"

using namespace std;

namespace simit {

// class ProgramContent
class Program::ProgramContent {
 public:
  ProgramContent(const std::string &name)
      : name(name), frontend(new internal::Frontend()), codegen(NULL) {}
  ~ProgramContent() {
//    for (auto &function : functions) {
//      delete function.second;
//    }
//    for (auto test : tests) {
//      delete test;
//    }
    delete frontend;
    delete codegen;
  }

  const std::string &name;

  internal::ProgramContext ctx;

//  std::map<std::string, internal::Function*> functions;
//  std::map<std::string, internal::ElementType*> elementTypes;
//  std::vector<internal::Test*> tests;

  Diagnostics diagnostics;

  internal::Frontend *getFrontend() { return frontend; }
  internal::CodeGen *getCodeGen() {
    if (codegen == NULL) {
      codegen = new internal::LLVMCodeGen();
    }
    return codegen;
  }

 private:
  internal::Frontend *frontend;
  internal::LLVMCodeGen *codegen;
};


// class Program
Program::Program(const std::string &name)
    : impl(new ProgramContent(name)) {
}

Program::~Program() {
  delete impl;
}

std::string Program::getName() const {
  return impl->name;
}

int Program::loadString(const string &programString) {
  std::vector<Error> errors;
  int status = impl->getFrontend()->parseString(programString, &impl->ctx,
                                                &errors);
  for (auto &error : errors) {
    impl->diagnostics.report() << error.toString();
  }
  return status;
}

int Program::loadFile(const std::string &filename) {
  std::vector<Error> errors;
  int status = impl->getFrontend()->parseFile(filename, &impl->ctx, &errors);
  for (auto &error : errors) {
    impl->diagnostics.report() << error.toString();
  }
  return status;
}

std::unique_ptr<Function> Program::compile(const std::string &function) {
  internal::Function *func = impl->ctx.getFunction(function);
  if (!func) {
    impl->diagnostics.report() << "Attempting to compile unknown function ("
                               << function << ")";
    return NULL;
  }

  return std::unique_ptr<Function>(impl->getCodeGen()->compile(func));
}

int Program::verify() {
  internal::LLVMCodeGen codegen;
  return codegen.verify(impl->ctx, &impl->diagnostics) != 0;
}

bool Program::hasErrors() const {
  return impl->diagnostics.hasErrors();
}

const Diagnostics &Program::getDiagnostics() const {
  return impl->diagnostics;
}

std::ostream &operator<<(std::ostream &os, const Program &program) {
  auto it = program.impl->ctx.getFunctions().begin();
  auto end = program.impl->ctx.getFunctions().end();
  if (it != end) {
    os << *it++;
  }
  while (it != end) {
    os << ", " << *it++;
  }
  return os << endl;
}

} // namespace simit
