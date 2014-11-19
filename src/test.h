#ifndef SIMIT_TEST_H
#define SIMIT_TEST_H

#include <string>
#include <vector>
#include <map>

#include "ir.h"
#include "program_context.h"
#include "function.h"
#include "graph.h"

namespace simit {
namespace internal {


/// A Simit test case. Simit test cases can be declared in language comments
/// and can subsequently be picked up by a test framework.
class Test {
public:
  Test() {}
  virtual ~Test() {}

  virtual std::string getCallee() const = 0;
  virtual bool evaluate(const ir::Func &func, simit::Function *compiledFunc,
                        Diagnostics *diags) const = 0;
};


/// A Simit function test. Test functions are invoked in language comments and
/// their results are compared against expected values:
///
/// %! add([0.0, 1.0, 2.0], [3.0, 4.0, 5.0]) == [3.0, 5.0, 7.0];
/// func add(a : tensor[3](float), b : tensor[3](float)) -> (c : tensor[3](float))
///   c = a + b;
/// end
class FunctionTest : public Test {
public:
  FunctionTest(const std::string &callee,
       const std::vector<ir::Expr> &actuals,
       const std::vector<ir::Expr> &expected)
      : callee(callee), actuals(actuals), expected(expected) {}

  std::string getCallee() const { return callee; }

  bool evaluate(const ir::Func &func, simit::Function *compiledFunc,
                Diagnostics *diags) const {
    // run the function with test->call->arguments
    assert(actuals.size() == func.getArguments().size());

    std::vector<ir::Var>  formalArgs = func.getArguments();
    std::vector<ir::Expr> actualArgs = actuals;
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
    auto expectedResults = expected;
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


/// A Simit procedure test. Test procedures are invoked in language comments.
/// The initial value of externs are set and the value is compared to expected
/// results after procedure invocation:
///
/// %%% field_add
/// %! points=3: points.x=[1.0, 2.0, 3.0] -> [-1.0, -2.0, -3.0]
/// element Point
///   x : float;
/// end
/// extern points : set{Point};
/// proc main
///   points.x = -points.x;
/// end
class ProcedureTest : public Test {
public:
  struct ExternField {
    FieldRefBase *field;
  };

  ProcedureTest(const std::string &callee,
                std::map<std::string, simit::SetBase*> externs)
      : callee(callee), externs(externs) {}

  std::string getCallee() const {return callee;}

  bool evaluate(const ir::Func &func, simit::Function *compiledFunc,
                Diagnostics *diags) const {
    for (auto &ext : externs) {
      std::cout << ext.first << std::endl;
      compiledFunc->bind(ext.first, ext.second);
    }

    return false;
  }

private:
  std::string callee;
  std::map<std::string, simit::SetBase*> externs;
};

}} // namespace simit::internal

#endif
