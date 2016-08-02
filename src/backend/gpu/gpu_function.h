#ifndef SIMIT_GPU_FUNCTION_H
#define SIMIT_GPU_FUNCTION_H

#include <string>
#include <map>
#include <memory>
#include <vector>
#include "cuda.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"

#include "backend/llvm/llvm_function.h"
#include "backend/backend_function.h"
#include "gpu_backend.h"
#include "gpu_codegen.h"

namespace simit {
namespace backend {
class Actual;

class GPUFunction : public LLVMFunction {
 public:
  GPUFunction(ir::Func simitFunc, llvm::Function *llvmFunc,
              llvm::Module *module,
              std::shared_ptr<llvm::EngineBuilder> engineBuilder,
              const ir::Storage& storage);
  ~GPUFunction();

  void print(std::ostream &os) const;
  void printMachine(std::ostream &os) const {}

  virtual void bind(const std::string& name, simit::Set* set);
  virtual void bind(const std::string& name, void* data);
  virtual void bind(const std::string& name, TensorData& data);
  virtual void mapArgs();
  virtual void unmapArgs(bool updated);

  virtual FuncType init();

 private:
  // Struct for tracking arguments being pushed and pulled to/from GPU
  // TODO: Split tracking current function args from any data we own on the GPU
  class DeviceDataHandle {
   public:
    CUdeviceptr *devBuffer;
    void *hostBuffer;
    const void *hostBufferConst;
    size_t size;
    bool devDirty;
    // TODO: Potentially support dirtying specific host buffers and
    // pushing only those
    // bool hostDirty;

    static size_t total_allocations;

    DeviceDataHandle(void *hostBuffer, CUdeviceptr *devBuffer, size_t size)
        : devBuffer(devBuffer), hostBuffer(hostBuffer),
          hostBufferConst((const void*)nullptr), size(size),
          devDirty(false) {
      total_allocations += size;
    }

    DeviceDataHandle(const void *hostBuffer, CUdeviceptr *devBuffer, size_t size)
        : devBuffer(devBuffer), hostBuffer(nullptr),
          hostBufferConst(hostBuffer), size(size),
          devDirty(false) {
      total_allocations += size;
    }

    ~DeviceDataHandle() { total_allocations -= size; }

   private:
    friend std::ostream& operator<<(std::ostream& os,
                                    const DeviceDataHandle& handle) {
      return os << ((handle.hostBuffer != nullptr) ?
                    handle.hostBuffer : handle.hostBufferConst)
                << " <-> " << (void*)(*handle.devBuffer)
                << " (" << handle.size << ")";
    }
  };
  // Internal collection to hold pushed set data
  // NOTE: None of the DeviceDataHandle pointers are owned by this struct,
  // they are instead memory-managed by the pushedBufs array.
  struct SetData {
    union {
      int setSize;
      DeviceDataHandle *setSizes;
    };

    // Only included if this is an edge set
    DeviceDataHandle *endpoints;
    DeviceDataHandle *startIndex; // row starts
    DeviceDataHandle *nbrIndex; // col indexes

    // Fields
    std::vector<DeviceDataHandle*> fields;
  };

  // Internal collection to hold pushed sparse tensor data
  // NOTE: None of the DeviceDataHandle pointers are owned by this struct,
  // they are instead memory-managed by the pushedBufs array.
  struct SparseTensorData {
    DeviceDataHandle *data;
    DeviceDataHandle *rowPtr;
    DeviceDataHandle *colInd;
  };

  // Push all data for a set, and return references to pushed buffers in a
  // SetData struct.
  SetData pushSetData(Set* set, const ir::SetType* setType);
  // Push data for a global tensor
  DeviceDataHandle *pushGlobalTensor(const ir::Environment& env,
                                     const ir::Storage& storage,
                                     const ir::Var& bufVar,
                                     const ir::TensorType* ttype);
  // Push data for an extern sparse tensor
  SparseTensorData pushExternSparseTensor(const ir::Environment& env,
                                          const ir::Var& bufVar,
                                          const ir::TensorType* ttype);

  // Copy argument memory into device and build an llvm value to point to it
  llvm::Value *pushArg(std::string name, ir::Type& argType, Actual* actual);
  
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
  std::map<std::string, TensorData*> tensorData;
  CUcontext *cudaContext;
  CUmodule *cudaModule;
  int cuDevMajor, cuDevMinor;
};

}}
#endif // SIMIT_GPU_FUNCTION_H
