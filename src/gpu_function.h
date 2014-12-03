#ifndef SIMIT_GPU_FUNCTION_H
#define SIMIT_GPU_FUNCTION_H

#include <string>
#include <map>
#include <vector>
#include "cuda.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"

#include "function.h"

namespace simit {
namespace internal {

class GPUFunction : public simit::Function {
 public:
  GPUFunction(simit::ir::Func simitFunc,
              llvm::Function *llvmFunc, llvm::Module *llvmModule);
  ~GPUFunction();

  void print(std::ostream &os) const;

 private:
  FuncType init(const std::vector<std::string> &formals,
                std::map<std::string, Actual> &actuals);
  // Allocate the given argument as a device buffer
  CUdeviceptr allocArg(const ir::Type& var);
  // Get argument data as a Literal
  const ir::Literal& getArgData(Actual& actual);
  // Copy literal memory to device buffer
  void pushArg(const ir::Literal& literal, CUdeviceptr &devBuffer);
  // Copy device buffer into literal data block
  void pullArg(const ir::Literal& literal, CUdeviceptr &devBuffer);

  std::unique_ptr<ir::Func> simitFunc;
  std::unique_ptr<llvm::Function> llvmFunc;
  std::unique_ptr<llvm::Module> llvmModule;
};

}
}

#endif // SIMIT_GPU_FUNCTION_H
