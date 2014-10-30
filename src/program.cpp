#include "program.h"

#include <set>

#include "ir.h"
#include "frontend.h"
#include "lower.h"
#include "llvm_backend.h"
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
      backend = new internal::LLVMBackend();
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
    std::map<ir::Func, simit::Function*> compiled;

    for (auto &test : ctx.getTests()) {
      // get binary function with name test->call->callee from list of functions
      ir::Func func = ctx.getFunction(test->getCallee());
      if (!func.defined()) {
        diags.report() << "Error: attempting to test unknown function";
        return 1;
      }

      if (compiled.find(func) == compiled.end()) {
        compiled[func] = compile(func);
      }
      Function *compiledFunc = compiled[func];

      // run the function with test->call->arguments
      assert(test->getActuals().size() == func.getArguments().size());

      std::vector<ir::Var>  formalArgs = func.getArguments();
      std::vector<ir::Expr> actualArgs = test->getActuals();
      for (size_t i=0; i < actualArgs.size(); ++i) {
        compiledFunc->bind(formalArgs[i].name, &actualArgs[i]);
      }

      auto formalResults = func.getResults();
      std::vector<ir::Expr> actualResults;
      for (auto &formalResult : formalResults) {
        ir::Expr actualResult = ir::Literal::make(formalResult.type);
        actualResults.push_back(actualResult);
        compiledFunc->bind(formalResult.name, &actualResult);
      }

      compiledFunc->runSafe();

      // compare function result with test->literal
      auto expectedResults = test->getExpectedResults();
      assert(expectedResults.size() == actualResults.size());
      auto rit = actualResults.begin();
      auto eit = expectedResults.begin();
      for (; rit != actualResults.end(); ++rit, ++eit) {
        if (*ir::to<ir::Literal>(*rit) != *ir::to<ir::Literal>(*eit)) {
          // TODO: Report with line number of test
          diags.report() << "Test failure (" << util::toString(*rit)
                         << " != " << util::toString(*eit) << ")";
          return 2;
        }
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
  std::vector<Error> errors;
  int status = impl->getFrontend()->parseString(programString, &impl->ctx,
                                                &errors);
  for (auto &error : errors) {
    impl->diags.report() << error.toString();
  }
  return status;
}

int Program::loadFile(const std::string &filename) {
  std::vector<Error> errors;
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
