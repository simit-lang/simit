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

#define LLVM_GENERIC_ADDRSPACE 0

#define LLVM_CONTEXT   llvm::getGlobalContext()

#define LLVM_VOID      llvm::Type::getVoidTy(LLVM_CONTEXT)

#define LLVM_INT       llvm::Type::getInt32Ty(LLVM_CONTEXT)
#define LLVM_INTPTR    llvm::Type::getInt32PtrTy(LLVM_CONTEXT)
#define LLVM_FLOAT     llvm::Type::getFloatTy(LLVM_CONTEXT)
#define LLVM_FLOATPTR  llvm::Type::getFloatPtrTy(LLVM_CONTEXT)
#define LLVM_DOUBLE    llvm::Type::getDoubleTy(LLVM_CONTEXT)
#define LLVM_DOUBLEPTR llvm::Type::getDoublePtrTy(LLVM_CONTEXT)
#define LLVM_BOOL      llvm::Type::getInt1Ty(LLVM_CONTEXT)
#define LLVM_BOOLPTR   llvm::Type::getInt1PtrTy(LLVM_CONTEXT)

#define LLVM_INT8      llvm::Type::getInt8Ty(LLVM_CONTEXT)
#define LLVM_INT8PTR   llvm::Type::getInt8PtrTy(LLVM_CONTEXT)
#define LLVM_INT32     llvm::Type::getInt32Ty(LLVM_CONTEXT)
#define LLVM_INT64     llvm::Type::getInt64Ty(LLVM_CONTEXT)

// Pointers with global addrspace
#define LLVM_INTPTR_GLOBAL    llvm::Type::getInt32PtrTy(LLVM_CONTEXT, 1)
#define LLVM_FLOATPTR_GLOBAL  llvm::Type::getFloatPtrTy(LLVM_CONTEXT, 1)
#define LLVM_DOUBLEPTR_GLOBAL llvm::Type::getDoublePtrTy(LLVM_CONTEXT, 1)
#define LLVM_BOOLPTR_GLOBAL   llvm::Type::getInt1PtrTy(LLVM_CONTEXT, 1)
#define LLVM_INT8PTR_GLOBAL   llvm::Type::getInt8PtrTy(LLVM_CONTEXT, 1)

typedef llvm::IRBuilder<true, llvm::ConstantFolder,
                        llvm::IRBuilderDefaultInserter<true>> LLVMIRBuilder;

namespace simit {
namespace internal {

llvm::ConstantInt* llvmInt(long long int val, unsigned bits=32);
llvm::ConstantInt* llvmUInt(long long unsigned int val, unsigned bits=32);
llvm::Constant* llvmFP(double val, unsigned bits=64);
llvm::Constant* llvmBool(bool val);

// Simit-specific utilities

/// The number of index struct elements that are compiled into an edge struct.
extern const int NUM_EDGE_INDEX_ELEMENTS;

extern bool singlePrecision;
llvm::Type *getLLVMFloatType();
llvm::Type *getLLVMFloatPtrType(unsigned addrspace=0);

llvm::Type *llvmPtrType(ir::ScalarType stype, unsigned addrspace);

llvm::Constant *llvmPtr(llvm::Type *type, const void *data);
llvm::Constant *llvmPtr(const ir::Type &type, const void *data,
                        unsigned addrspace=LLVM_GENERIC_ADDRSPACE);
llvm::Constant *llvmPtr(const ir::Literal *literal);
llvm::Constant *llvmVal(const ir::Literal *literal);

ir::Type simitType(const llvm::Type *type);

llvm::Type       *createLLVMType(const ir::Type &,
                                 unsigned addrspace=LLVM_GENERIC_ADDRSPACE);
llvm::StructType *createLLVMType(const ir::SetType *,
                                 unsigned addrspace=LLVM_GENERIC_ADDRSPACE);
llvm::Type       *createLLVMType(const ir::TensorType *ttype,
                                 unsigned addrspace=LLVM_GENERIC_ADDRSPACE);
llvm::Type       *createLLVMType(ir::ScalarType stype);

/// Creates an llvm function prototype
llvm::Function *createPrototype(const std::string &name,
                                const std::vector<ir::Var> &arguments,
                                const std::vector<ir::Var> &results,
                                llvm::Module *module,
                                bool externalLinkage=false,
                                bool doesNotThrow=true,
                                unsigned addrspace=LLVM_GENERIC_ADDRSPACE);

std::ostream &operator<<(std::ostream &os, const llvm::Module &);
std::ostream &operator<<(std::ostream &os, const llvm::Value &);
std::ostream &operator<<(std::ostream &os, const llvm::Type &);

}} // namespace simit::internal


#endif
