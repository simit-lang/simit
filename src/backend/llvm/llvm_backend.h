#ifndef SIMIT_LLVM_BACKEND_H
#define SIMIT_LLVM_BACKEND_H

#include <ostream>
#include <memory>
#include <set>
#include <vector>
#include <map>

#include "backend/backend_impl.h"

#include "storage.h"
#include "var.h"
#include "backend/backend_visitor.h"
#include "util/scopedmap.h"

namespace llvm {
class LLVMContext;
class Module;
class EngineBuilder;
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


namespace simit {

namespace ir {
  class Environment;
}

namespace backend {

class SimitIRBuilder;

extern const std::string VAL_SUFFIX;
extern const std::string PTR_SUFFIX;
extern const std::string LEN_SUFFIX;

std::shared_ptr<llvm::EngineBuilder> createEngineBuilder(llvm::Module *module);

/// Code generator that uses LLVM to compile Simit IR.
class LLVMBackend : public BackendImpl, protected BackendVisitor<llvm::Value*> {
public:
  LLVMBackend();
  virtual ~LLVMBackend();

protected:
  virtual unsigned globalAddrspace() {return 0;}

  util::ScopedMap<simit::ir::Var, llvm::Value*> symtable;

  // Globally allocated buffers
  std::map<ir::Var, llvm::Value*> buffers;

  std::set<ir::Var> globals;
  ir::Storage storage;
  const ir::Environment* environment;

  llvm::Module *module;
  std::unique_ptr<llvm::DataLayout> dataLayout;
  std::unique_ptr<SimitIRBuilder> builder;

  using BackendImpl::compile;
  virtual Function* compile(ir::Func func, const ir::Storage& storage);

  using BackendVisitor::compile;
  virtual void compile(const ir::Literal&);
  virtual void compile(const ir::VarExpr&);
  virtual void compile(const ir::Load&);
  virtual void compile(const ir::FieldRead&);
  virtual void compile(const ir::Length&);
  virtual void compile(const ir::IndexRead&);

  virtual void compile(const ir::Neg&);
  virtual void compile(const ir::Add&);
  virtual void compile(const ir::Sub&);
  virtual void compile(const ir::Mul&);
  virtual void compile(const ir::Div&);
  virtual void compile(const ir::Rem&);

  virtual void compile(const ir::Not&);
  virtual void compile(const ir::Eq&);
  virtual void compile(const ir::Ne&);
  virtual void compile(const ir::Gt&);
  virtual void compile(const ir::Lt&);
  virtual void compile(const ir::Ge&);
  virtual void compile(const ir::Le&);
  virtual void compile(const ir::And&);
  virtual void compile(const ir::Or&);
  virtual void compile(const ir::Xor&);
  
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

  void emitAssign(ir::Var var, const ir::Expr& value);

  /// Produce LLVM globals for everything in `env` and store in `globals`
  /// and in `symtable` appropriately.
  virtual void emitGlobals(const ir::Environment& env, bool packed=true);

  virtual void emitPrintf(llvm::Value *str, 
                          std::vector<llvm::Value*> args={});
  virtual void emitPrintf(std::string format, 
                          std::vector<llvm::Value*> args={});

  /// Emit a memcpy instruction
  virtual void emitMemCpy(llvm::Value *dst, llvm::Value *src,
                          llvm::Value *size, unsigned align);

  /// Emit a memset instruction
  virtual void emitMemSet(llvm::Value *dst, llvm::Value *val,
                          llvm::Value *size, unsigned align);

  /// Allocate a global pointer for a tensor, and add to the symtable
  /// and list of global buffers
  virtual llvm::Value *makeGlobalTensor(ir::Var var);
  
  /// Compile a single argument and return its llvm values
  std::vector<llvm::Value*> emitArgument(ir::Expr argument,
                                         bool excludeStaticTypes);

  /// Compile several arguments and return their llvm values
  std::vector<llvm::Value*> emitArguments(std::vector<ir::Expr> arguments,
                                          bool excludeStaticTypes);

  /// Compile a single result and return its llvm values
  std::vector<llvm::Value*>
  emitResult(ir::Var result, bool excludeStaticTypes,
             std::vector<std::pair<ir::Var, llvm::Value*>>* resVals);

  /// Compile several results and return their llvm values
  std::vector<llvm::Value*>
  emitResults(std::vector<ir::Var> results, bool excludeStaticTypes,
              std::vector<std::pair<ir::Var, llvm::Value*>>* resVals);

  /// Emit a call to a function defined in Simit
  void emitInternalCall(const ir::CallStmt& callStmt);

  /// Emit a call to a function defined externally (e.g. C)
  void emitExternCall(const ir::CallStmt& callStmt);

  /// Emit a call to an intrinsic
  void emitIntrinsicCall(const ir::CallStmt& callStmt);

  // TODO: Remove this function, once the old init system has been removed
  ir::Func makeSystemTensorsGlobal(ir::Func func);

private:
  static bool llvmInitialized;
};

}}
#endif
