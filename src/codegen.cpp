#include "codegen.h"

#include "errors.h"
#include "function.h"
#include "ir.h"
#include "util.h"
#include "program_context.h"

using namespace std;
using namespace simit::util;

namespace simit {
namespace internal {

int CodeGen::verify(ProgramContext &ctx, Diagnostics *diags) {
  // For each test look up the called function. Grab the actual arguments and
  // run the function with them as input.  Then compare the result to the
  // expected literal.
  std::map<Function*, simit::Function*> compiled;

  for (auto &test : ctx.getTests()) {
    // get binary function with name test->call->callee from list of functions
    Function *func = ctx.getFunction(test->getCallee());
    if (func == NULL) {
      diags->report() << "Error: attempting to test unknown function";
      return 1;
    }

    if (compiled.find(func) == compiled.end()) {
      compiled[func] = compile(func);
    }
    simit::Function *compiledFunc = compiled[func];

    // run the function with test->call->arguments
    auto arguments = test->getArguments();
    assert(arguments.size() == func->getArguments().size());

    std::vector<std::shared_ptr<internal::Literal>> results;
    for (auto &result : func->getResults()) {
      internal::Literal *resultLit = new internal::Literal(result->getType());
      resultLit->clear();
      results.push_back(shared_ptr<internal::Literal>(resultLit));
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
