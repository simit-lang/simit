#include "indexvariables.h"

#include <cassert>
#include <ostream>

namespace simit {
namespace internal {

/* class IndexVar */
std::ostream &operator<<(std::ostream &os, const IndexVar &var) {
  var.print(os);
  return os;
}


/* class ReductionIndexVar */
void ReductionIndexVar::print(std::ostream &os) const {
  switch (op) {
    case ReductionIndexVar::ADD:
      os << "+";
      break;
    case ReductionIndexVar::MUL:
      os << "*";
      break;
    default:
      assert(false);
      break;
  }

  os << getName();
}


/* class IndexVarFactory */
std::vector<std::shared_ptr<IndexVar>>
IndexVarFactory::makeFreeVars(unsigned int n) {
  auto freeIndexVars = std::vector<std::shared_ptr<IndexVar>>();
  for (unsigned int i=0; i<n; ++i) {
    auto freeIndexVar = new FreeIndexVar(makeName());
    freeIndexVars.push_back(std::shared_ptr<IndexVar>(freeIndexVar));
  }
  nameID += n;
  return freeIndexVars;
}

std::shared_ptr<IndexVar>
IndexVarFactory::makeReductionVar(ReductionIndexVar::Operator op) {
  auto reductionIndexVar = new ReductionIndexVar(op, makeName());
  return std::shared_ptr<IndexVar>(reductionIndexVar);
}

std::string IndexVarFactory::makeName() {
  char name[2];
  name[0] = 'i' + nameID;
  name[1] = '\0';
  nameID++;
  return std::string(name);
}

}}