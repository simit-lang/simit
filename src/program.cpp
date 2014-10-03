#include "program.h"

#include <set>

#include "ir.h"
#include "sir.h"
#include "frontend.h"
#include "sir_codegen.h"
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
    //  return impl->getBackend()->verify(impl->ctx, &impl->diagnostics) != 0;

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
//        compiled[func] = getBackend()->compile(func);
        compiled[func] = compile(func);
      }
      simit::Function *compiledFunc = compiled[func];

      // run the function with test->call->arguments
      assert(test->getActuals().size() == func->getArguments().size());

      auto &formalArguments = func->getArguments();
      auto &actualArguments = test->getActuals();
      for (size_t i=0; i < actualArguments.size(); ++i) {
        compiledFunc->bind(formalArguments[i]->getName(),
                           actualArguments[i].get());
      }

      auto &formalResults = func->getResults();
      std::vector<std::shared_ptr<ir::Literal>> actualResults;
      for (auto &formalResult : formalResults) {
        auto actualResultPtr = new ir::Literal(formalResult->getType());
        auto actualResult = std::shared_ptr<ir::Literal>(actualResultPtr);
        actualResult->clear();
        actualResults.push_back(actualResult);
        compiledFunc->bind(formalResult->getName(), actualResult.get());
      }

      compiledFunc->run();

      // compare function result with test->literal
      auto expectedResults = test->getExpectedResults();
      assert(expectedResults.size() == actualResults.size());
      auto rit = actualResults.begin();
      auto eit = expectedResults.begin();
      for (; rit != actualResults.end(); ++rit, ++eit) {
        if (**rit != **eit) {
          // TODO: Report with line number of test
          diags.report() << "Test failure (" << util::toString(**rit)
          << " != " << util::toString(**eit) << ")";
          return 2;
        }
      }
    }
    
    return 0;
  }

 private:
  internal::Frontend *frontend;
  ir::SetIRCodeGen setIRCodeGen;
  internal::LLVMBackend *backend;

  Function *compile(ir::Function *simitFunc) {
//    std::unique_ptr<ir::SetIRNode> setIR = setIRCodeGen.codegen(simitFunc);
//    cout << *simitFunc << endl;
//    cout << *setIR << endl;
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
    os << *it++;
  }
  while (it != end) {
    os << ", " << *it++;
  }
  return os << endl;
}

} // namespace simit
