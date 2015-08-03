#ifndef SIMIT_GPU_FUNCTION_H
#define SIMIT_GPU_FUNCTION_H

#include <string>
#include <map>
#include <vector>
#include "cuda.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"

#include "backend/backend_function.h"
#include "gpu_backend.h"
#include "gpu_codegen.h"

namespace simit {
namespace backend {

class GPUFunction : public simit::backend::Function {
 public:
  GPUFunction(ir::Func simitFunc, llvm::Function *llvmFunc,
              llvm::Module *module,
              std::map<ir::Var, llvm::Value*> globalBufs,
              ir::Storage storage);
  ~GPUFunction();

  void print(std::ostream &os) const;
  void printMachine(std::ostream &os) const {}

  virtual void mapArgs();
  virtual void unmapArgs(bool updated);

 private:
  // Find the size of a domain
  int findShardSize(ir::IndexSet domain);

  FuncType init(const std::vector<std::string> &formals,
                std::map<std::string, Actual> &actuals);
  // Allocate the given argument as a device buffer
  CUdeviceptr allocArg(const ir::Type& var);
  // Get argument data as a Literal
  const ir::Literal& getArgData(Actual& actual);
  // Compute tensor size given the actual bound sets
  size_t computeTensorSize(const ir::Var& var);

  // Struct for tracking arguments being pushed and pulled to/from GPU
  // TODO: Split tracking current function args from any data we own on the GPU
  struct DeviceDataHandle {
    CUdeviceptr *devBuffer;
    void *hostBuffer;
    size_t size;
    bool devDirty;
    // TODO: Potentially support dirtying specific host buffers and
    // pushing only those
    // bool hostDirty;

    static size_t total_allocations;

    DeviceDataHandle(void *hostBuffer, CUdeviceptr *devBuffer, size_t size)
        : devBuffer(devBuffer), hostBuffer(hostBuffer), size(size),
          devDirty(false) {
      total_allocations += size;
    }

    ~DeviceDataHandle() { total_allocations -= size; }
  };

  // Copy argument memory into device and build an llvm value to point to it
  llvm::Value *pushArg(std::string formal, Actual& actual);
  // Copy device buffer into host data block
  void pullArg(DeviceDataHandle* handle);
  // Free the device buffer
  void freeArg(DeviceDataHandle* handle);
  // Create the harness function which sets up args for the main function
  llvm::Function *createHarness(const llvm::SmallVector<llvm::Value*, 8> &args,
                                llvm::Function *kernel, llvm::Module *module);

  std::vector<DeviceDataHandle*> pushedBufs;
  std::map<std::string, std::vector<DeviceDataHandle*> > argBufMap;
  std::unique_ptr<ir::Func> simitFunc;
  std::unique_ptr<llvm::Function> llvmFunc;
  std::unique_ptr<llvm::Module> module;
  std::map<ir::Var, llvm::Value*> globalBufs;
  ir::Storage storage;
  CUcontext *cudaContext;
  CUmodule *cudaModule;
  int cuDevMajor, cuDevMinor;
};

}}
#endif // SIMIT_GPU_FUNCTION_H
