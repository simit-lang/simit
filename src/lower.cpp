#include "lower.h"

#include <map>

#include "temps.h"
#include "flatten.h"
#include "storage.h"

using namespace std;

namespace simit {
namespace ir {

Func lower(Func func) {
  func = insertTemporaries(func);
  func = flattenIndexExpressions(func);

  Storage tensorStorages = getStorage(func);
//  for (auto &storage : tensorStorages) {
//    std::cout << storage.first << ": " << storage.second << std::endl;
//  }

  func = lowerAssemblies(func, tensorStorages);
  func = lowerIndexExpressions(func, tensorStorages);
  func = lowerTensorAccesses(func);
  return func;
}

}}
