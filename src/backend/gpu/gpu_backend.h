#ifndef SIMIT_GPU_BACKEND_H
#define SIMIT_GPU_BACKEND_H

#include "backend/backend.h"
#include "backend/llvm/llvm_backend.h"

#include "ir_visitor.h"
#include "gpu_ir.h"

#define CUDA_GENERIC_ADDRSPACE 0
#define CUDA_GLOBAL_ADDRSPACE 1
#define CUDA_SHARED_ADDRSPACE 3

// transforms
#include "fuse_kernels.h"
#include "kernel_rw_analysis.h"
#include "rewrite_system_assign.h"
#include "shard_gpu_loops.h"
#include "var_decl_rewriter.h"

namespace llvm {
class Value;
}

namespace simit {
namespace internal {

class GPUBackend : public LLVMBackend {
public:
  GPUBackend() {}
  ~GPUBackend() {}

  virtual simit::internal::Function *compile(const simit::ir::Func &func);

protected:
  // CUDA variables
  int cuDevMajor, cuDevMinor;
  
  const int blockSize = 128;

  // Tracking whether we're in a kernel
  bool inKernel;

  // Currently compiling IR Func
  ir::Func irFunc;

  // Currently compiling LLVM function
  llvm::Function *func;

  virtual unsigned global_addrspace() { return CUDA_GLOBAL_ADDRSPACE; }

  using LLVMBackend::visit;

  virtual llvm::Value *compile(const ir::Expr &expr);
  virtual void visit(const ir::FieldRead *);
  virtual void visit(const ir::TensorRead *);
  virtual void visit(const ir::TupleRead *);
  virtual void visit(const ir::IndexRead *op);
  virtual void visit(const ir::Length *op);
  virtual void visit(const ir::Map *);
  virtual void visit(const ir::IndexedTensor *);
  virtual void visit(const ir::IndexExpr *op);
  virtual void visit(const ir::TensorWrite *);

  virtual void visit(const ir::Literal *);
  virtual void visit(const ir::VarExpr *);
  virtual void visit(const ir::Load *);
  virtual void visit(const ir::Call *);
  virtual void visit(const ir::Neg *);
  virtual void visit(const ir::Add *);
  virtual void visit(const ir::Sub *);
  virtual void visit(const ir::Mul *);
  virtual void visit(const ir::Div *);

  virtual void visit(const ir::VarDecl *);
  virtual void visit(const ir::AssignStmt *);
  virtual void visit(const ir::CallStmt *);
  virtual void visit(const ir::FieldWrite *);
  virtual void visit(const ir::Store *);
  virtual void visit(const ir::ForRange *);
  virtual void visit(const ir::For *);
  virtual void visit(const ir::IfThenElse *);
  virtual void visit(const ir::Block *);
  virtual void visit(const ir::Pass *);

  virtual void visit(const ir::GPUKernel *);

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

  virtual void makeGlobalTensor(ir::Var var);
};

}
}

#endif // SIMIT_GPU_BACKEND_H
