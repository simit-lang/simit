#ifndef SIMIT_LLVM_BACKEND_H
#define SIMIT_LLVM_BACKEND_H

#include <ostream>
#include <memory>
#include <set>
#include <vector>
#include <map>

#include "backend/backend.h"

#include "storage.h"
#include "var.h"
#include "ir_visitor.h"
#include "util/scopedmap.h"

namespace llvm {
class LLVMContext;
class Module;
class ExecutionEngine;
class ConstantFolder;
template<bool> class IRBuilderDefaultInserter;
template<bool, typename, typename> class IRBuilder;
class Constant;
class Type;
class Value;
class Instruction;
class Function;
class DataLayout;
}

typedef llvm::IRBuilder<true, llvm::ConstantFolder,
                        llvm::IRBuilderDefaultInserter<true>> LLVMIRBuilder;

namespace simit {
namespace backend {

extern const std::string VAL_SUFFIX;
extern const std::string PTR_SUFFIX;
extern const std::string LEN_SUFFIX;

/// Code generator that uses LLVM to compile Simit IR.
class LLVMBackend : public Backend, public ir::IRVisitor {
public:
  LLVMBackend();
  ~LLVMBackend();

  virtual Function* compile(const ir::Func &func);

protected:
  virtual unsigned global_addrspace()  { return 0; } // LLVM generic addrspace
  virtual unsigned generic_addrspace() { return 0; } // LLVM generic addrspace
  
  util::ScopedMap<simit::ir::Var, llvm::Value*> symtable;

  // Globally allocated buffers
  std::map<ir::Var, llvm::Value*> buffers;
  ir::Storage storage;

  llvm::Module *module;
  std::unique_ptr<llvm::DataLayout> dataLayout;
  std::unique_ptr<LLVMIRBuilder> builder;

  /// used to return variables from Expr visit functions
  llvm::Value *val;
  
  using Backend::compile;
  using ir::IRVisitor::visit;

  virtual llvm::Value *compile(const ir::Expr &expr);
  virtual void compile(const ir::Stmt &stmt);

  virtual void visit(const ir::FieldRead *);
  virtual void visit(const ir::IndexRead *op);
  virtual void visit(const ir::Length *op);

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

  /// IRNodes that should never reach the backend (should have been lowered)
  virtual void visit(const ir::Map *);
  virtual void visit(const ir::IndexedTensor *);
  virtual void visit(const ir::IndexExpr *op);
  virtual void visit(const ir::TupleRead *);
  virtual void visit(const ir::TensorWrite *);
  virtual void visit(const ir::TensorRead *);

  /// Get a pointer to the given field
  llvm::Value *emitFieldRead(const ir::Expr &elemOrSet, std::string fieldName);

  /// Get the number of components in the tensor
  llvm::Value *emitComputeLen(const ir::TensorType*, const ir::TensorStorage &);

  /// Get the number of elements in the index domain
  llvm::Value *emitComputeLen(const ir::IndexDomain&);

  /// Get the number of elements in the index sets
  llvm::Value *emitComputeLen(const ir::IndexSet&);

  llvm::Value *loadFromArray(llvm::Value *array, llvm::Value *index);

  llvm::Value *emitCall(std::string name, std::vector<llvm::Value*> args);

  llvm::Value *emitCall(std::string name, std::vector<llvm::Value*> args,
                        llvm::Type *returnType);

  /// Build a global string and return a constant pointer to it
  llvm::Constant *emitGlobalString(const std::string& str);

  /// Gets a reference to a named built-in
  llvm::Function* getBuiltIn(std::string name,
                             llvm::Type *retTy,
                             std::vector<llvm::Type*> argTys);

  /// Emit an empty function and set the builder cursor to its entry block. The
  /// function's arguments and result variables are added to the symbol table.
  llvm::Function *emitEmptyFunction(const std::string &name,
                                    const std::vector<ir::Var> &arguments,
                                    const std::vector<ir::Var> &results,
                                    bool externalLinkage=false,
                                    bool doesNotThrow=true,
                                    bool scalarsByValue=true);

  virtual void emitPrintf(std::string format, std::vector<llvm::Value*> args={});

  void emitAssign(ir::Var var, const ir::Expr& value);

  /// Emit a memcpy instruction
  virtual void emitMemCpy(llvm::Value *dst, llvm::Value *src,
                          llvm::Value *size, unsigned align);

  /// Emit a memset instruction
  virtual void emitMemSet(llvm::Value *dst, llvm::Value *val,
                          llvm::Value *size, unsigned align);

  /// Allocate a global pointer for a tensor, and add to the symtable
  /// and list of global buffers
  virtual llvm::Value *makeGlobalTensor(ir::Var var);

private:
  static bool llvmInitialized;
};

}}
#endif
