#include "indexvariables.h"

#include <cassert>
#include <ostream>

namespace simit {
namespace internal {

/* class IndexVariable */
std::ostream &operator<<(std::ostream &os,
                         const simit::internal::IndexVariable &var) {
  var.print(os);
  return os;
}


/* class ReductionIndexVariable */
void ReductionIndexVariable::print(std::ostream &os) const {
  switch (op) {
    case ReductionIndexVariable::ADD:
      os << "+";
      break;
    case ReductionIndexVariable::MUL:
      os << "*";
      break;
    default:
      assert(false);
      break;
  }

  os << getName();
}


/* class IndexVariableFactory */
std::list<std::shared_ptr<IndexVariable>>
IndexVariableFactory::makeFreeVariables(unsigned int n) {
  auto freeIndexVars = std::list<std::shared_ptr<IndexVariable>>();
  for (int i=0; i<n; ++i) {
    auto freeIndexVar = new FreeIndexVariable(makeName());
    freeIndexVars.push_back(std::shared_ptr<IndexVariable>(freeIndexVar));
  }
  nameID += n;
  return freeIndexVars;
}

std::shared_ptr<IndexVariable>
IndexVariableFactory::makeReductionVariable(ReductionIndexVariable::Operator op) {
  auto reductionIndexVar = new ReductionIndexVariable(op, makeName());
  return std::shared_ptr<IndexVariable>(reductionIndexVar);
}

std::string IndexVariableFactory::makeName() {
  char name[2];
  name[0] = 'i' + nameID;
  name[1] = '\0';
  nameID++;
  return std::string(name);
}

}}