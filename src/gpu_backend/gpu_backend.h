#ifndef SIMIT_GPU_BACKEND_H
#define SIMIT_GPU_BACKEND_H

#include "backend.h"
#include "ir_visitor.h"
#include "llvm_backend.h"
#include "gpu_ir.h"

namespace llvm {
class Value;
}

namespace simit {
namespace internal {

class GPUBackend : public LLVMBackend {
public:
  GPUBackend();
  ~GPUBackend();

  simit::Function *compile(simit::ir::Func func);

private:
  // Used to track which dimensions of the GPU computation have been
  // parallelized across blocks
  GPUSharding sharding;

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
  virtual void visit(const ir::AssignStmt *);
  virtual void visit(const ir::FieldWrite *);
  virtual void visit(const ir::Store *);
  virtual void visit(const ir::ForRange *);
  virtual void visit(const ir::For *);
  virtual void visit(const ir::IfThenElse *);
  virtual void visit(const ir::Block *);
  virtual void visit(const ir::Pass *);

  virtual void visit(const ir::GPUFor *);

  // Emits calls to nvvm intrinsics to read thread ids
  llvm::Value *emitBarrier();
  llvm::Value *emitCheckRoot();
  llvm::Value *getTidX();
  llvm::Value *getTidY();
  llvm::Value *getTidZ();

  void emitThreadBarrier();
  virtual void emitFirstAssign(const ir::Var& var,
                               const ir::Expr& value);
};

}
}

#endif // SIMIT_GPU_BACKEND_H
