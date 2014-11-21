#ifndef SIMIT_GPU_BACKEND_H
#define SIMIT_GPU_BACKEND_H

#include "backend.h"
#include "ir_visitor.h"

namespace llvm {
class IRBuilderBase;
}

namespace simit {
namespace internal {

class GPUBackend : public Backend, ir::IRVisitor {
public:
  GPUBackend();
  ~GPUBackend();

  simit::Function *compile(simit::ir::Func func);

private:
  // virtual void visit(const ir::Literal *);
  // virtual void visit(const ir::VarExpr *);
  // virtual void visit(const ir::Result *);
  // virtual void visit(const ir::FieldRead *);
  // virtual void visit(const ir::TensorRead *);
  // virtual void visit(const ir::TupleRead *);
  // virtual void visit(const ir::Map *);
  // virtual void visit(const ir::IndexedTensor *);
  // virtual void visit(const ir::Call *);
  // virtual void visit(const ir::Neg *);
  // virtual void visit(const ir::Add *);
  // virtual void visit(const ir::Sub *);
  // virtual void visit(const ir::Mul *);
  // virtual void visit(const ir::Div *);

  // virtual void visit(const ir::AssignStmt *);
  // virtual void visit(const ir::FieldWrite *);
  // virtual void visit(const ir::TensorWrite *);
  // virtual void visit(const ir::For *);
  // virtual void visit(const ir::IfThenElse *);
  // virtual void visit(const ir::Block *);
  // virtual void visit(const ir::Pass *);

  std::unique_ptr<llvm::IRBuilderBase> builder;
};

}
}

#endif // SIMIT_GPU_BACKEND_H
