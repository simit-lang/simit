#include "storage.h"

using namespace std;
using namespace simit::ir;

namespace simit {
namespace internal {

std::map<IRNode*, void*>
TemporaryAllocator::allocateTemporaries(Function *f) {
  temps.clear();

  return temps;
}

}} // namespace simit::internal
