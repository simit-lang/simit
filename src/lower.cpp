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

  TensorStorages storageDescriptors = getTensorStorages(func);
//  for (auto &storage : storageDescriptors) {
//    std::cout << storage.first << ": " << storage.second << std::endl;
//  }

//  func = lowerAssemblies(func);
  func = lowerIndexExpressions(func, storageDescriptors);
  func = lowerTensorAccesses(func);
  return func;
}

}}
