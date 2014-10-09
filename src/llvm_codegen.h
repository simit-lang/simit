#ifndef SIMIT_LLVM_CODEGEN_H
#define SIMIT_LLVM_CODEGEN_H

#include <memory>
#include <string>
#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"

#include "ir.h"

#define LLVM_CONTEXT   llvm::getGlobalContext()

#define LLVM_VOID      llvm::Type::getVoidTy(LLVM_CONTEXT)

#define LLVM_INT       llvm::Type::getInt32Ty(LLVM_CONTEXT)
#define LLVM_INTPTR    llvm::Type::getInt32PtrTy(LLVM_CONTEXT)
#define LLVM_DOUBLE    llvm::Type::getDoubleTy(LLVM_CONTEXT)
#define LLVM_DOUBLEPTR llvm::Type::getDoublePtrTy(LLVM_CONTEXT)

#define LLVM_INT8      llvm::Type::getInt8Ty(LLVM_CONTEXT)
#define LLVM_INT32     llvm::Type::getInt32Ty(LLVM_CONTEXT)
#define LLVM_INT64     llvm::Type::getInt64Ty(LLVM_CONTEXT)

namespace simit {
namespace internal {

llvm::ConstantInt* getInt32(int val);
llvm::ConstantInt* getUInt32(unsigned val);

llvm::Type *llvmType(const ir::ScalarType *stype);

llvm::Type *llvmType(const ir::TensorType *ttype);

llvm::Type *llvmType(const ir::Type &type);

llvm::Constant *llvmPtr(const ir::Type &type, void *data);

llvm::Constant *llvmPtr(ir::Literal *literal);

ir::Type simitType(const llvm::Type *type);

/// Creates an empy llvm function.
llvm::Function *
createFunction(const std::string &name,
               const std::vector<std::shared_ptr<ir::Argument>> &arguments,
               const std::vector<std::shared_ptr<ir::Result>> &results,
               llvm::GlobalValue::LinkageTypes linkage,
               llvm::Module *module);

}} // namespace simit::internal


#endif
