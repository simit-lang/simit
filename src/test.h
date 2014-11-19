#ifndef SIMIT_TEST_H
#define SIMIT_TEST_H

#include <string>
#include <vector>

#include "ir.h"
#include "program_context.h"
#include "function.h"

namespace simit {
namespace internal {

class Test {
public:
  Test() {}
  virtual ~Test() {}

  virtual std::string getCallee() const = 0;
  virtual bool evaluate(const ir::Func &func, simit::Function *compiledFunc,
                        Diagnostics *diags) const = 0;
};

/// A Simit test case. Simit test cases can be declared in language comments
/// and can subsequently be picked up by a test framework.
class FunctionTest : public Test {
public:
  FunctionTest(const std::string &callee,
       const std::vector<ir::Expr> &actuals,
       const std::vector<ir::Expr> &expected)
      : callee(callee), actuals(actuals), expected(expected) {}

  std::string getCallee() const { return callee; }

  std::vector<ir::Expr> getActuals() const {
    return actuals;
  }

  const std::vector<ir::Expr> &getExpectedResults() const {
    return expected;
  }

  bool evaluate(const ir::Func &func, simit::Function *compiledFunc,
                Diagnostics *diags) const {
    // run the function with test->call->arguments
    assert(getActuals().size() == func.getArguments().size());

    std::vector<ir::Var>  formalArgs = func.getArguments();
    std::vector<ir::Expr> actualArgs = getActuals();
    for (size_t i=0; i < actualArgs.size(); ++i) {
      compiledFunc->bind(formalArgs[i].getName(), &actualArgs[i]);
    }

    auto formalResults = func.getResults();
    std::vector<ir::Expr> actualResults;
    for (auto &formalResult : formalResults) {
      ir::Expr actualResult = ir::Literal::make(formalResult.getType());
      actualResults.push_back(actualResult);
      compiledFunc->bind(formalResult.getName(), &actualResult);
    }

    compiledFunc->runSafe();

    // compare function result with test->literal
    auto expectedResults = getExpectedResults();
    assert(expectedResults.size() == actualResults.size());
    auto rit = actualResults.begin();
    auto eit = expectedResults.begin();
    for (; rit != actualResults.end(); ++rit, ++eit) {
      if (*ir::to<ir::Literal>(*rit) != *ir::to<ir::Literal>(*eit)) {
        // TODO: Report with line number of test
        diags->report() << "Test failure (" << util::toString(*rit)
                        << " != " << util::toString(*eit) << ")";
        return false;
      }
    }

    return true;
  }

private:
  std::string callee;
  std::vector<ir::Expr> actuals;
  std::vector<ir::Expr> expected;
};

}} // namespace simit::internal

#endif
