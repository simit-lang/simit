#ifndef SIMIT_LLVM_CODEGEN_H
#define SIMIT_LLVM_CODEGEN_H

#include <memory>
#include <string>
#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"

#include "ir.h"

#define LLVM_CONTEXT   llvm::getGlobalContext()

#define LLVM_VOID      llvm::Type::getVoidTy(LLVM_CONTEXT)

#define LLVM_INT       llvm::Type::getInt32Ty(LLVM_CONTEXT)
#define LLVM_INTPTR    llvm::Type::getInt32PtrTy(LLVM_CONTEXT)
#define LLVM_DOUBLE    llvm::Type::getDoubleTy(LLVM_CONTEXT)
#define LLVM_DOUBLEPTR llvm::Type::getDoublePtrTy(LLVM_CONTEXT)
#define LLVM_BOOL      llvm::Type::getInt1Ty(LLVM_CONTEXT)
#define LLVM_BOOLPTR   llvm::Type::getInt1PtrTy(LLVM_CONTEXT)

#define LLVM_INT8      llvm::Type::getInt8Ty(LLVM_CONTEXT)
#define LLVM_INT8PTR   llvm::Type::getInt8PtrTy(LLVM_CONTEXT)
#define LLVM_INT32     llvm::Type::getInt32Ty(LLVM_CONTEXT)
#define LLVM_INT64     llvm::Type::getInt64Ty(LLVM_CONTEXT)

typedef llvm::IRBuilder<true, llvm::ConstantFolder,
                        llvm::IRBuilderDefaultInserter<true>> LLVMIRBuilder;

namespace simit {
namespace internal {

/// The number of index struct elements that are compiled into an edge struct.
extern const int NUM_EDGE_INDEX_ELEMENTS;

llvm::ConstantInt* llvmInt(long long int val, unsigned bits=32);
llvm::ConstantInt* llvmUInt(long long unsigned int val, unsigned bits=32);
llvm::ConstantFP* llvmFP(double val, unsigned bits=64);
// use llvm::ConstantInt::getTrue() or llvm::ConstantInt::getFalse() for boolean
// literals

void print(const std::string &format, std::initializer_list<llvm::Value*> args,
           LLVMIRBuilder *builder, llvm::Module *module);

llvm::Type *createLLVMType(ir::ScalarType stype);

llvm::Type *llvmPtrType(ir::ScalarType stype);

llvm::Constant *llvmPtr(llvm::Type *type, const void *data);
llvm::Constant *llvmPtr(const ir::Type &type, const void *data);

llvm::Constant *llvmPtr(const ir::Literal *literal);

ir::Type simitType(const llvm::Type *type);

llvm::Type *createLLVMType(const ir::TensorType *ttype);
llvm::StructType *createLLVMType(const ir::SetType *);
llvm::Type *createLLVMType(const ir::Type &);

/// Creates an empy llvm function.
llvm::Function *createFunction(const std::string &name,
                               const std::vector<ir::Var> &arguments,
                               const std::vector<ir::Var> &results,
                               llvm::Module *module);

std::ostream &operator<<(std::ostream &os, const llvm::Value &);
std::ostream &operator<<(std::ostream &os, const llvm::Type &);

}} // namespace simit::internal


#endif
