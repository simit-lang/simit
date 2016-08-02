#ifndef SIMIT_GPU_BACKEND_H
#define SIMIT_GPU_BACKEND_H

#include "backend/backend.h"
#include "backend/llvm/llvm_backend.h"

#include "ir_visitor.h"
#include "gpu_ir.h"

#define CUDA_GENERIC_ADDRSPACE 0
#define CUDA_GLOBAL_ADDRSPACE 1
#define CUDA_SHARED_ADDRSPACE 3

// Pointers with global addrspace
#define CUDA_INT_PTR_GLOBAL    llvm::Type::getInt32PtrTy(LLVM_CTX, 1)
#define CUDA_FLOAT_PTR_GLOBAL  llvm::Type::getFloatPtrTy(LLVM_CTX, 1)
#define CUDA_DOUBLE_PTR_GLOBAL llvm::Type::getDoublePtrTy(LLVM_CTX, 1)
#define CUDA_BOOL_PTR_GLOBAL   llvm::Type::getInt1PtrTy(LLVM_CTX, 1)
#define CUDA_INT8_PTR_GLOBAL   llvm::Type::getInt8PtrTy(LLVM_CTX, 1)

// transforms
#include "fuse_kernels.h"
#include "kernel_rw_analysis.h"
#include "localize_temps.h"
#include "rewrite_system_assign.h"
#include "shard_gpu_loops.h"
#include "var_decl_rewriter.h"

namespace llvm {
class Value;
}

namespace simit {
namespace backend {

class GPUBackend : public LLVMBackend {
public:
  GPUBackend();
  ~GPUBackend() {}

protected:
  // CUDA variables
  int cuDevMajor, cuDevMinor;
  
  const int blockSize = 128;

  // Tracking whether we're in a kernel
  bool inKernel;

  // Currently compiling IR Func
  ir::Func irFunc;

  virtual unsigned globalAddrspace() { return CUDA_GLOBAL_ADDRSPACE; }

  using LLVMBackend::compile;
  virtual Function* compile(ir::Func func, const ir::Storage& storage);

  virtual void compile(const ir::Literal&);
  virtual void compile(const ir::VarExpr&);
  virtual void compile(const ir::Load&);
  virtual void compile(const ir::FieldRead&);
  virtual void compile(const ir::Length&);
  virtual void compile(const ir::IndexRead&);

  // Binary ops delegated to generic LLVM backend:
  // virtual void compile(const ir::Neg&);
  // virtual void compile(const ir::Add&);
  // virtual void compile(const ir::Sub&);
  // virtual void compile(const ir::Mul&);
  // virtual void compile(const ir::Div&);
  // virtual void compile(const ir::Rem&);

  // virtual void compile(const ir::Not&);
  // virtual void compile(const ir::Eq&);
  // virtual void compile(const ir::Ne&);
  // virtual void compile(const ir::Gt&);
  // virtual void compile(const ir::Lt&);
  // virtual void compile(const ir::Ge&);
  // virtual void compile(const ir::Le&);
  // virtual void compile(const ir::And&);
  // virtual void compile(const ir::Or&);
  // virtual void compile(const ir::Xor&);

  virtual void compile(const ir::VarDecl&);
  virtual void compile(const ir::AssignStmt&);
  virtual void compile(const ir::CallStmt&);
  virtual void compile(const ir::Store&);
  virtual void compile(const ir::FieldWrite&);
  virtual void compile(const ir::Scope&);
  virtual void compile(const ir::IfThenElse&);
  virtual void compile(const ir::ForRange&);
  virtual void compile(const ir::For&);
  virtual void compile(const ir::While&);
  virtual void compile(const ir::Print&);

  virtual void compile(const ir::GPUKernel&);

  // Emits calls to nvvm intrinsics
  llvm::Value *emitBarrier();
  llvm::Value *emitCheckRoot();
  llvm::Value *getTidX();
  llvm::Value *getTidY();
  llvm::Value *getTidZ();
  llvm::Value *emitCastGlobalToGen(llvm::Value *src);

  void emitThreadBarrier();
  void emitDeviceSync();
  void emitAtomicLoadAdd(llvm::Value *ptr, llvm::Value *value);
  void emitAtomicFLoadAdd(llvm::Value *ptr, llvm::Value *value);
  void emitKernelLaunch(llvm::Function *kernel,
                        std::vector<llvm::Value*> args,
                        GPUSharding sharding);
  void emitKernelLaunch(llvm::Function *kernel,
                        std::vector<llvm::Value*> args,
                        llvm::Value *xSize,
                        llvm::Value *ySize,
                        llvm::Value *zSize);

  virtual void emitGlobals(const ir::Environment& env);

  virtual void emitPrintf(std::string format,
                          std::vector<llvm::Value*> args={});

  virtual void emitMemCpy(llvm::Value *dst, llvm::Value *src,
                          llvm::Value *size, unsigned align);

  virtual void emitMemSet(llvm::Value *dst, llvm::Value *val,
                          llvm::Value *size, unsigned align);

  void emitShardedMemSet(ir::Type targetType, llvm::Value *target,
                         llvm::Value *size);

  void emitShardedDot(ir::Type vec1Type, ir::Type vec2Type, ir::Type resType,
                      llvm::Value *vec1, llvm::Value *vec2,
                      llvm::Value *size, llvm::Value *result);

  void emitFillBuf(llvm::Value *buffer,
                   std::vector<llvm::Value*> vals,
                   unsigned align,
                   bool alignToArgSize);

  virtual llvm::Value *makeGlobalTensor(ir::Var var);
};

}
}

#endif // SIMIT_GPU_BACKEND_H
