#include "storage.h"

using namespace simit::internal;
using namespace std;

std::map<IRNode*, void*> TemporaryAllocator::allocateTemporaries(Function *f) {
  temps.clear();

  return temps;
}
