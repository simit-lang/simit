#include "program.h"

#include <set>

#include "ir.h"
#include "frontend.h"
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
    auto simitFunc = ctx.getFunction(function);
    if (!simitFunc) {
      diags.report() << "Attempting to compile unknown function ("
                     << function << ")";
      return NULL;
    }
    return std::unique_ptr<Function>(compile(simitFunc));
  }

  int verify() {
    return 0; // TODO newir: Remove

    // For each test look up the called function. Grab the actual arguments and
    // run the function with them as input.  Then compare the result to the
    // expected literal.
    std::map<ir::Function*, simit::Function*> compiled;

    for (auto &test : ctx.getTests()) {
      // get binary function with name test->call->callee from list of functions
      ir::Function *func = ctx.getFunction(test->getCallee());
      if (func == NULL) {
        diags.report() << "Error: attempting to test unknown function";
        return 1;
      }

      if (compiled.find(func) == compiled.end()) {
        compiled[func] = compile(func);
      }
      simit::Function *compiledFunc = compiled[func];

      // run the function with test->call->arguments
      assert(test->getActuals().size() == func->getArguments().size());

      auto formalArgs = func->getArguments();
      auto actualArgs = test->getActuals();
      for (size_t i=0; i < actualArgs.size(); ++i) {
        auto formal = toVariable(formalArgs[i])->name;
        auto actual = toLiteral(actualArgs[i]);
        compiledFunc->bind(formal, actual);
      }

      auto formalResults = func->getResults();
      std::vector<ir::Expr> actualResults;
      for (auto &formalResult : formalResults) {
        ir::Expr actualResult = ir::Literal::make(formalResult.type());
        actualResults.push_back(actualResult);
        auto formal = toVariable(formalResult)->name;
        auto actual = toLiteral(actualResult);
        compiledFunc->bind(formal, actual);
      }

      compiledFunc->run();

      // compare function result with test->literal
      auto expectedResults = test->getExpectedResults();
      assert(expectedResults.size() == actualResults.size());
      auto rit = actualResults.begin();
      auto eit = expectedResults.begin();
      for (; rit != actualResults.end(); ++rit, ++eit) {
        if (*static_cast<ir::Literal*>(rit->expr()) !=
            *static_cast<ir::Literal*>(eit->expr())) {
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

  Function *compile(ir::Function *simitFunc) {
    return getBackend()->compile(simitFunc);
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
