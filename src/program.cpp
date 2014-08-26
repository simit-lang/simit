#include "program.h"

#include <memory>
#include <set>

#include "ir.h"
#include "frontend.h"
#include "llvm_codegen.h"
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
    for (auto &function : functions) {
      delete function.second;
    }
    for (auto test : tests) {
      delete test;
    }
    delete frontend;
    delete codegen;
  }

  const std::string &name;

  std::map<std::string, Function*> functions;

  std::vector<simit::Error> errors;
  std::vector<Test*> tests;

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
}} // simit::internal


// Program
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

int Program::verify() {
  // For each test look up the called function. Grab the actual arguments and
  // run the function with them as input.  Then compare the result to the
  // expected literal.

  std::map<Function*, CompiledFunction*> compiled;

  for (auto &test : impl->tests) {
    LLVMCodeGen codegen;
    // get binary function with name test->call->callee from list of functions
    Function *func = impl->functions[test->getCallee()];
    if (func == NULL) {
      // TODO: Report error, attempting to call unknown function
      cerr << "Error: attempting to call unknown function" << endl;
      return 1;
    }

    if (compiled.find(func) == compiled.end()) {
      compiled[func] = codegen.compile(func);
//      cout << *func << endl << *compiled[func];
    }
    CompiledFunction *compiledFunc = compiled[func];

    // run the function with test->call->arguments
    auto arguments = test->getArguments();
    assert(arguments.size() == func->getArguments().size());

    std::vector<std::shared_ptr<Literal>> results;
    for (auto &result : func->getResults()) {
      Literal *resultLit = new Literal(new TensorType(*result->getType()));
      resultLit->clear();
      results.push_back(shared_ptr<Literal>(resultLit));
    }

    compiledFunc->bind(arguments, results);
    compiledFunc->run();

    // compare function result with test->literal
    auto expectedResults = test->getExpectedResults();
    assert(expectedResults.size() == results.size());
    auto rit = results.begin();
    auto eit = expectedResults.begin();
    for (; rit != results.end(); ++rit, ++eit) {
      if (**rit != **eit) {
        // TODO: Report error
        cerr << **rit << " != " << endl << **eit << endl;
        return 1;
      }
    }
  }
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
  auto begin = program.impl->functions.begin();
  auto end = program.impl->functions.end();
  if (begin != end) {
    os << begin->second;
    ++begin;
  }
  while (begin != end) {
    os << ", " << begin->second;
    ++begin;
  }
  return os << endl;
}
