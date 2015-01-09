#ifndef SIMIT_GPU_FUNCTION_H
#define SIMIT_GPU_FUNCTION_H

#include <string>
#include <map>
#include <vector>
#include "cuda.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"

#include "function.h"
#include "gpu_backend.h"
#include "gpu_codegen.h"

namespace simit {
namespace internal {

class GPUFunction : public simit::Function {
 public:
  GPUFunction(
      ir::Func simitFunc, llvm::Module *module,
      std::vector< std::pair<llvm::Function *, GPUSharding> > kernels,
      class GPUSharding sharding, int cuDevMajor, int cuDevMinor);
  ~GPUFunction() {}

  void print(std::ostream &os) const;

 private:
  // Find the size of a domain
  int findShardSize(ir::IndexSet domain);

  FuncType init(const std::vector<std::string> &formals,
                std::map<std::string, Actual> &actuals);
  // Allocate the given argument as a device buffer
  CUdeviceptr allocArg(const ir::Type& var);
  // Get argument data as a Literal
  const ir::Literal& getArgData(Actual& actual);

  // Struct for tracking arguments being pushed and pulled to/from GPU
  struct DeviceDataHandle {
    CUdeviceptr *devBuffer;
    size_t size;
    bool shouldPull;

    DeviceDataHandle(CUdeviceptr *devBuffer, size_t size, bool shouldPull=true) :
        devBuffer(devBuffer), size(size), shouldPull(shouldPull) {}
  };

  // Copy argument memory into device and build an llvm value to point to it
  llvm::Value *pushArg(Actual& actual,
                       std::map<void*, DeviceDataHandle> &pushedBufs,
                       bool shouldPull = true);
  // Copy device buffer into host data block and free the device buffer
  void pullArgAndFree(void *hostPtr, DeviceDataHandle handle);
  // Create the harness function which sets up args for the main function
  llvm::Function *createHarness(const llvm::SmallVector<llvm::Value*, 8> &args,
                                llvm::Function *kernel, llvm::Module *module);

  std::unique_ptr<ir::Func> simitFunc;
  std::vector< std::pair<llvm::Function *, GPUSharding> > kernels;
  std::unique_ptr<llvm::Module> module;
  int cuDevMajor, cuDevMinor;
  class GPUSharding sharding;
};

}
}

#endif // SIMIT_GPU_FUNCTION_H
