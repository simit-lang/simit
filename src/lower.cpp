#include "lower.h"

#include <map>

#include "temps.h"
#include "flatten.h"
#include "tensor_storage.h"

using namespace std;

namespace simit {
namespace ir {

Func lower(Func func) {
  func = insertTemporaries(func);
  func = flattenIndexExpressions(func);

  TensorStorages tensorStorages = getTensorStorages(func);
//  for (auto &storage : tensorStorages) {
//    std::cout << storage.first << ": " << storage.second << std::endl;
//  }

  func = lowerAssemblies(func, tensorStorages);
  func = lowerIndexExpressions(func, tensorStorages);
  func = lowerTensorAccesses(func);
  return func;
}

}}
