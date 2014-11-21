#include "program.h"

#include <set>

#include "ir.h"
#include "frontend.h"
#include "lower.h"
#include "gpu_backend.h"
#include "llvm_backend.h"
#include "function.h"
#include "util.h"
#include "error.h"
#include "program_context.h"

using namespace std;

namespace simit {

// class ProgramContent
class Program::ProgramContent {
 public:
  ProgramContent(const std::string &name)
      : name(name), frontend(new internal::Frontend()), backend(NULL) {}

  ~ProgramContent() {
    delete frontend;
    delete backend;
  }

  const std::string &name;

  internal::ProgramContext ctx;
  Diagnostics diags;

  internal::Frontend *getFrontend() { return frontend; }

  internal::Backend *getBackend() {
    if (backend == NULL) {
      // backend = new internal::LLVMBackend();
      backend = new internal::GPUBackend();
    }
    return backend;
  }

  std::unique_ptr<Function> compile(const std::string &function) {
    ir::Func simitFunc = ctx.getFunction(function);
    if (!simitFunc.defined()) {
      diags.report() << "Attempting to compile unknown function ("
                     << function << ")";
      return NULL;
    }
    return std::unique_ptr<Function>(compile(simitFunc));
  }

  int verify() {
    // For each test look up the called function. Grab the actual arguments and
    // run the function with them as input.  Then compare the result to the
    // expected literal.
    const std::map<std::string, ir::Func> &functions = ctx.getFunctions();
    std::map<ir::Func, simit::Function*> compiled;

    for (auto &test : ctx.getTests()) {
      if (functions.find(test->getCallee()) == functions.end()) {
        diags.report() << "Error: attempting to test unknown function "
                       << "'" << test->getCallee() << "'";
        return 1;
      }
      ir::Func func = functions.at(test->getCallee());

      if (compiled.find(func) == compiled.end()) {
        compiled[func] = compile(func);
      }
    }

    for (auto &test : ctx.getTests()) {
      iassert(functions.find(test->getCallee()) != functions.end());
      ir::Func func = functions.at(test->getCallee());

      iassert(compiled.find(func) != compiled.end());
      Function *compiledFunc = compiled.at(func);

      bool evaluates = test->evaluate(func, compiledFunc, &diags);
      if (!evaluates) {
        return 2;
      }
    }
    
    return 0;
  }

 private:
  internal::Frontend *frontend;
  internal::Backend *backend;

  Function *compile(ir::Func func) {
    func = lower(func);
    return getBackend()->compile(func);
  }
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
  std::vector<ParseError> errors;
  int status = impl->getFrontend()->parseString(programString, &impl->ctx,
                                                &errors);
  for (auto &error : errors) {
    impl->diags.report() << error.toString();
  }
  return status;
}

int Program::loadFile(const std::string &filename) {
  std::vector<ParseError> errors;
  int status = impl->getFrontend()->parseFile(filename, &impl->ctx, &errors);
  for (auto &error : errors) {
    impl->diags.report() << error.toString();
  }
  return status;
}

std::unique_ptr<Function> Program::compile(const std::string &function) {
  return impl->compile(function);
}

int Program::verify() {
  return impl->verify();
}

bool Program::hasErrors() const {
  return impl->diags.hasErrors();
}

const Diagnostics &Program::getDiagnostics() const {
  return impl->diags;
}

std::ostream &operator<<(std::ostream &os, const Program &program) {
  auto it = program.impl->ctx.getFunctions().begin();
  auto end = program.impl->ctx.getFunctions().end();
  if (it != end) {
    os << it->second << endl;
    ++it;
  }
  while (it != end) {
    os << endl << it->second << endl;
    ++it;
  }
  return os;
}

} // namespace simit
