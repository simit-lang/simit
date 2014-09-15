#include "backend.h"

#include "errors.h"
#include "function.h"
#include "ir.h"
#include "util.h"
#include "program_context.h"

using namespace std;
using namespace simit::ir;
using namespace simit::util;

namespace simit {
namespace internal {

int Backend::verify(ProgramContext &ctx, Diagnostics *diags) {
  // For each test look up the called function. Grab the actual arguments and
  // run the function with them as input.  Then compare the result to the
  // expected literal.
  std::map<ir::Function*, simit::Function*> compiled;

  for (auto &test : ctx.getTests()) {
    // get binary function with name test->call->callee from list of functions
    ir::Function *func = ctx.getFunction(test->getCallee());
    if (func == NULL) {
      diags->report() << "Error: attempting to test unknown function";
      return 1;
    }

    if (compiled.find(func) == compiled.end()) {
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
        diags->report() << "Test failure (" << toString(**rit) << " != "
                        << toString(**eit) << ")";
        return 2;
      }
    }
  }

  return 0;
}

}} // namespace simit::internal
