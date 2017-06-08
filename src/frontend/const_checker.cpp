#include <string>

#include "const_checker.h"
#include "fir.h"
#include "error.h"
  
namespace simit {
namespace fir {

void ConstChecker::check(Program::Ptr program) {
  for (auto elem : program->elems) {
    if (isa<ConstDecl>(elem)) {
      const auto decl = to<ConstDecl>(elem);

      // Verify that global constant is not inialized to a non-literal value, 
      // which is currently unsupported.
      simit_iassert((bool)decl->initVal);
      if (!isa<TensorLiteral>(decl->initVal)) {
        reportError("global constant must be initialized to a literal", decl);
      }
    } else if (isa<Test>(elem)) {
      const auto test = to<Test>(elem);
      
      // Check for non-literal arguments, which are unsupported for tests.
      for (auto arg : test->args) {
        if (!isa<TensorLiteral>(arg)) {
          reportError("input to test must be a literal", arg);
        }
      }
    
      // Check for non-literal expected value, which are unsupported for tests.
      if (!isa<TensorLiteral>(test->expected)) {
        const auto msg = "expected value for test must be a literal";
        reportError(msg, test->expected);
      }
    }
  }
}

void ConstChecker::reportError(std::string msg, FIRNode::Ptr loc) {
  const auto err = ParseError(loc->getLineBegin(), loc->getColBegin(), 
                              loc->getLineEnd(), loc->getColEnd(), msg);
  errors->push_back(err);
}

}
}

