#ifndef SIMIT_CODEGEN_LLVM_H
#define SIMIT_CODEGEN_LLVM_H

#include "backend.h"
#include "ir_visitor.h"
#include "scopedmap.h"

#include <ostream>
#include <memory>
#include <set>
#include <vector>

namespace llvm {
class LLVMContext;
class Module;
class ExecutionEngine;
class ConstantFolder;
template<bool> class IRBuilderDefaultInserter;
template<bool, typename, typename> class IRBuilder;
class Value;
class Instruction;
class Function;
}

namespace simit {
namespace internal {

/// Code generator that uses LLVM to compile Simit IR.
class LLVMBackend : public Backend, ir::IRVisitor {
public:
  LLVMBackend();
  ~LLVMBackend();

  simit::Function *compile(simit::ir::Func func);

private:
  static bool llvmInitialized;

  llvm::Module *module;
  llvm::IRBuilder<true, llvm::ConstantFolder,
                  llvm::IRBuilderDefaultInserter<true> > *builder;

  ScopedMap<std::string, llvm::Value*> symtable;

  /// used to return variables from Expr visit functions
  llvm::Value *val;

  llvm::Value *compile(const ir::Expr &expr);
  void compile(const ir::Stmt &stmt);

  virtual void visit(const ir::FieldRead *);
  virtual void visit(const ir::TensorRead *);
  virtual void visit(const ir::TupleRead *);
  virtual void visit(const ir::IndexRead *op);
  virtual void visit(const ir::Map *);
  virtual void visit(const ir::IndexedTensor *);
  virtual void visit(const ir::IndexExpr *op);
  virtual void visit(const ir::TensorWrite *);

  virtual void visit(const ir::Literal *);
  virtual void visit(const ir::VarExpr *);
  virtual void visit(const ir::Result *);
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
  virtual void visit(const ir::For *);
  virtual void visit(const ir::IfThenElse *);
  virtual void visit(const ir::Block *);
  virtual void visit(const ir::Pass *);

  /// Get a pointer to the given field
  llvm::Value *emitFieldRead(const ir::Expr &elemOrSet, std::string fieldName);

  /// Get the number of components in the tensor
  llvm::Value *emitComputeLen(const ir::TensorType *);

  /// Get the number of elements in the index sets
  llvm::Value *emitComputeLen(const ir::IndexSet &);

  /// Get the number of elements in the index domain
  llvm::Value *emitComputeLen(const ir::IndexDomain &);
};

}} // namespace simit::internal
#endif
