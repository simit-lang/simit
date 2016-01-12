#ifndef SIMIT_CONST_CHECKER_H
#define SIMIT_CONST_CHECKER_H

#include <vector>
#include <string>

#include "hir.h"
#include "error.h"

namespace simit {
namespace hir {

// Searches for non-literals in places where literals values are expected (i.e. 
// global constant initialization and tests). Most if not all of the 
// constraints enforced by this pass should be removed or relaxed eventually.
class ConstChecker {
public:
  ConstChecker(std::vector<ParseError> *errors) : errors(errors) {} 
  
  void check(Program::Ptr);

private:
  void reportError(std::string, HIRNode::Ptr);

private: 
  std::vector<ParseError> *errors;
};

}
}

#endif

