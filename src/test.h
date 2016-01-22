#ifndef SIMIT_TEST_H
#define SIMIT_TEST_H

#include <string>
#include <vector>
#include <map>

#include "tensor.h"

#include "ir.h"
#include "types.h"
#include "program_context.h"
#include "backend/backend_function.h"
#include "program.h"
#include "graph.h"
#include "util/collections.h"
#include "util/arrays.h"

namespace simit {
namespace internal {

/// A Simit test case. Simit test cases can be declared in language comments
/// and can subsequently be picked up by a test framework.
class Test {
public:
  Test() {}
  virtual ~Test() {}

  virtual std::string getCallee() const = 0;
  virtual bool evaluate(const ir::Func &func, simit::Function compiledFunc,
                        Diagnostics *diags) const = 0;
};


/// A Simit function test. Test functions are called from language comments and
/// their results are compared against expected values:
///
/// %! add([0.0, 1.0, 2.0], [3.0, 4.0, 5.0]) == [3.0, 5.0, 7.0];
/// func add(a : tensor[3](float), b : tensor[3](float))->(c : tensor[3](float))
///   c = a + b;
/// end
class FunctionTest : public Test {
public:
  FunctionTest(const std::string &callee,
               const std::vector<ir::Expr> &actualLiterals,
               const std::vector<ir::Expr> &expectLiterals)
      : callee(callee), actuals(actualLiterals), expects(expectLiterals) {
  }

  ~FunctionTest() {}

  std::string getCallee() const { return callee; }

  bool evaluate(const ir::Func &func, simit::Function compiledFunc,
                Diagnostics *diags) const {

    // Check that actual types match function formal types
    if (actuals.size() != func.getArguments().size()) {
      diags->report() << "The number of actuals do not match the number of "
                         "formals.";
      return false;
    }
    for (auto pair : util::zip(actuals, func.getArguments())) {
      iassert(ir::isa<ir::Literal>(pair.first));
      const auto actualType = pair.first.type();
      const auto argType = pair.second.getType();
      if (actualType != argType || (actualType.isTensor() &&
          actualType.toTensor()->isColumnVector != 
          argType.toTensor()->isColumnVector)) {
        diags->report() << "The actual types do not match the formal types.";
        return false;
      }
    }

    // Check that expected types match function result types
    if (expects.size() != func.getResults().size()) {
      diags->report() << "The number of results do not match the expected "
                         "number of results.";
      return false;
    }
    for (auto pair : util::zip(expects, func.getResults())) {
      iassert(ir::isa<ir::Literal>(pair.first));
      if (pair.first.type() != pair.second.getType()) {
        diags->report() << "The result types do not match the expected result "
                           "types.";
        return false;
      }
    }

    // Bind the actuals
    for (auto pair : util::zip(func.getArguments(), actuals)) {
      auto& formal = pair.first;
      auto& actual = pair.second;
      compiledFunc.bind(formal.getName(), ir::to<ir::Literal>(actual)->data);
    }

    // Allocate space for and bind results
    std::vector<ir::Expr> results;
    for (auto &resultFormal : func.getResults()) {
      ir::Type resultType = resultFormal.getType();
      ir::Expr result = ir::Literal::make(resultType);
      results.push_back(result);
      compiledFunc.bind(resultFormal.getName(),
                        ir::to<ir::Literal>(result)->data);
    }

    compiledFunc.runSafe();

    // Compare function result with the expected result
    for (auto pair : util::zip(results, expects)) {
      auto& result = pair.first;
      auto& expect = pair.second;

      if (*ir::to<ir::Literal>(result) != *ir::to<ir::Literal>(expect)) {
        diags->report() << util::toString(result) << " != " <<
                           util::toString(expect);
        return false;
      }
    }

    return true;
  }

private:
  std::string callee;
  std::vector<ir::Expr> actuals;
  std::vector<ir::Expr> expects;
};


/// A Simit procedure test. Test procedures are called from language comments.
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
                std::map<std::string,simit::Set*> externs)
      : callee(callee), externs(externs) {}

  std::string getCallee() const {return callee;}

  bool evaluate(const ir::Func &func, simit::Function compiledFunc,
                Diagnostics *diags) const {
    for (auto &ext : externs) {
      compiledFunc.bind(ext.first, ext.second);
    }

    return false;
  }

private:
  std::string callee;
  std::map<std::string,simit::Set*> externs;
};

}}

#endif
