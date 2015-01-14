#ifndef SIMIT_GPU_BACKEND_H
#define SIMIT_GPU_BACKEND_H

#include "backend.h"
#include "ir_visitor.h"
#include "llvm_backend.h"
#include "gpu_ir.h"

#define CUDA_GENERIC_ADDRSPACE 0
#define CUDA_GLOBAL_ADDRSPACE 1
#define CUDA_SHARED_ADDRSPACE 3

// transforms
#include "shard_gpu_loops.h"
#include "lift_gpu_vars.h"

namespace llvm {
class Value;
}

namespace simit {
namespace internal {

class GPUBackend : public LLVMBackend {
public:
  GPUBackend() {}
  ~GPUBackend() {}

  simit::Function *compile(simit::ir::Func func);

protected:
  // CUDA variables
  int cuDevMajor, cuDevMinor;

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

  // Emits calls to nvvm intrinsics to read thread ids
  llvm::Value *emitBarrier();
  llvm::Value *emitCheckRoot();
  llvm::Value *getTidX();
  llvm::Value *getTidY();
  llvm::Value *getTidZ();

  void emitThreadBarrier();
  void emitAtomicLoadAdd(llvm::Value *ptr, llvm::Value *value);
  void emitAtomicFLoadAdd(llvm::Value *ptr, llvm::Value *value);
  void emitTid0Code(const ir::Stmt& body);
  void emitKernelLaunch(llvm::Function *kernel,
                        std::vector<llvm::Value*> args,
                        GPUSharding sharding);

  virtual void emitPrintf(std::string format,
                          std::vector<llvm::Value*> args={});

  virtual void emitMemCpy(llvm::Value *dst, llvm::Value *src,
                          llvm::Value *size, unsigned align);

  virtual void emitMemSet(llvm::Value *dst, llvm::Value *val,
                          llvm::Value *size, unsigned align);

  void emitFillBuf(llvm::Value *buffer,
                   std::vector<llvm::Value*> vals);

  virtual void makeGlobalTensor(ir::Var var);
};

}
}

#endif // SIMIT_GPU_BACKEND_H
