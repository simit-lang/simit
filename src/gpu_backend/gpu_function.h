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

namespace simit {
namespace internal {

class GPUFunction : public simit::Function {
 public:
  GPUFunction(ir::Func simitFunc, llvm::Function *llvmFunc,
              llvm::Module *llvmModule, struct GPUSharding sharding);
  ~GPUFunction();

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

    DeviceDataHandle(CUdeviceptr *devBuffer, size_t size) :
        devBuffer(devBuffer), size(size), shouldPull(true) {}
    DeviceDataHandle(CUdeviceptr *devBuffer, size_t size, bool shouldPull) :
        devBuffer(devBuffer), size(size), shouldPull(shouldPull) {}
  };

  // Copy argument memory into device and build an llvm value to point to it
  llvm::Value *pushArg(Actual& actual,
                       std::map<void*, DeviceDataHandle> &pushedBufs);
  // Copy device buffer into host data block and free the device buffer
  void pullArgAndFree(void *hostPtr, DeviceDataHandle handle);
  // Create the harness function which sets up args for the main function
  llvm::Function *createHarness(const llvm::SmallVector<llvm::Value*, 8> &args);

  std::unique_ptr<ir::Func> simitFunc;
  std::unique_ptr<llvm::Function> llvmFunc;
  std::unique_ptr<llvm::Module> llvmModule;
  struct GPUSharding sharding;
};

}
}

#endif // SIMIT_GPU_FUNCTION_H
