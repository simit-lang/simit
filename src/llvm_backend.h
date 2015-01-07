#ifndef SIMIT_CODEGEN_LLVM_H
#define SIMIT_CODEGEN_LLVM_H

#include "backend.h"
#include "var.h"
#include "ir_visitor.h"
#include "scopedmap.h"

#include <ostream>
#include <memory>
#include <set>
#include <vector>
#include <map>

namespace llvm {
class LLVMContext;
class Module;
class ExecutionEngine;
class ConstantFolder;
template<bool> class IRBuilderDefaultInserter;
template<bool, typename, typename> class IRBuilder;
class Type;
class Value;
class Instruction;
class Function;
}

typedef llvm::IRBuilder<true, llvm::ConstantFolder,
                        llvm::IRBuilderDefaultInserter<true>> LLVMIRBuilder;

namespace simit {
namespace internal {

/// Code generator that uses LLVM to compile Simit IR.
class LLVMBackend : public Backend, public ir::IRVisitor {
public:
  LLVMBackend();
  ~LLVMBackend();

  simit::Function *compile(simit::ir::Func func);

protected:
  ScopedMap<simit::ir::Var, llvm::Value*> symtable;

  ir::Storage storage;
  ir::TensorStorage fieldStorage;

  llvm::Module *module;
  std::unique_ptr<LLVMIRBuilder> builder;

  /// used to return variables from Expr visit functions
  llvm::Value *val;

  virtual llvm::Value *compile(const ir::Expr &expr);
  virtual void compile(const ir::Stmt &stmt);

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

  virtual void visit(const ir::Eq *);
  virtual void visit(const ir::Ne *);
  virtual void visit(const ir::Gt *);
  virtual void visit(const ir::Lt *);
  virtual void visit(const ir::Ge *);
  virtual void visit(const ir::Le *);
  virtual void visit(const ir::And *);
  virtual void visit(const ir::Or *);
  virtual void visit(const ir::Not *);
  virtual void visit(const ir::Xor *);

  virtual void visit(const ir::VarDecl *op);
  virtual void visit(const ir::AssignStmt *);
  virtual void visit(const ir::CallStmt *);
  virtual void visit(const ir::FieldWrite *);
  virtual void visit(const ir::Store *);
  virtual void visit(const ir::ForRange *);
  virtual void visit(const ir::For *);
  virtual void visit(const ir::While *);
  virtual void visit(const ir::IfThenElse *);
  virtual void visit(const ir::Block *);
  virtual void visit(const ir::Pass *);
  virtual void visit(const ir::Print *);

  /// Get a pointer to the given field
  llvm::Value *emitFieldRead(const ir::Expr &elemOrSet, std::string fieldName);

  /// Get the number of components in the tensor
  llvm::Value *emitComputeLen(const ir::TensorType*, const ir::TensorStorage &);

  /// Get the number of elements in the index domain
  llvm::Value *emitComputeLen(const ir::IndexDomain&);

  /// Get the number of elements in the index sets
  llvm::Value *emitComputeLen(const ir::IndexSet&);

  llvm::Value *loadFromArray(llvm::Value *array, llvm::Value *index);

  llvm::Value *emitCall(std::string name,
                        std::initializer_list<llvm::Value*> args,
                        llvm::Type *returnType);

  llvm::Value *emitCall(std::string name,
                        std::vector<llvm::Value*> args,
                        llvm::Type *returnType);

  /// Emit an empty function and set the builder cursor to its entry block. The
  /// function's arguments and result variables are added to the symbol table.
  llvm::Function *emitEmptyFunction(const std::string &name,
                                    const std::vector<ir::Var> &arguments,
                                    const std::vector<ir::Var> &results);

//  void emitPrintf(std::string format);
  void emitPrintf(std::string format, std::vector<llvm::Value*> args={});

  virtual void emitFirstAssign(const ir::Var& var,
                               const ir::Expr& value);
  void emitAssign(ir::Var var, const ir::Expr& value);

private:
  static bool llvmInitialized;
};

}} // namespace simit::internal
#endif
