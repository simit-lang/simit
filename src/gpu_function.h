#ifndef SIMIT_GPU_FUNCTION_H
#define SIMIT_GPU_FUNCTION_H

#include <string>
#include <map>
#include <vector>

#include "function.h"

namespace simit {
namespace internal {

class GPUFunction : public simit::Function {
 public:
  GPUFunction(ir::Func simitFunc);
  ~GPUFunction();

  void print(std::ostream &os) const;

 private:
  FuncPtrType init(const std::vector<std::string> &formals,
                   std::map<std::string, Actual> &actuals);
};

}
}

#endif // SIMIT_GPU_FUNCTION_H
