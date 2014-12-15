#include "lower.h"

#include <map>
#include "storage.h"

using namespace std;

namespace simit {
namespace ir {

Func lower(Func func) {
  func = lowerAssemblies(func);
  func = lowerIndexExpressions(func);
  func = lowerTensorAccesses(func);
  return func;
}

}}
