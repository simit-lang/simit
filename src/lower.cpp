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

  map<Var,TensorStorage> storageDescriptors = getTensorStorage(func);
//  for (auto &storage : storageDescriptors) {
//    std::cout << storage.first << ": " << storage.second << std::endl;
//  }

  func = lowerIndexExpressions(func);
  func = lowerAssemblies(func);
  func = lowerTensorAccesses(func);
  return func;
}

}}
