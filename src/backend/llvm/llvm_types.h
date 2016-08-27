#ifndef SIMIT_LLVM_TYPES_H
#define SIMIT_LLVM_TYPES_H

#include "llvm/IR/Type.h"
#include "llvm/IR/DerivedTypes.h"

namespace simit {
namespace ir {
class Type;
struct SetType;
struct UnstructuredSetType;
struct GridSetType;
struct TensorType;
struct ArrayType;
struct ScalarType;
}

namespace backend {

extern llvm::Type* const LLVM_VOID;

extern llvm::Type* const LLVM_FLOAT;
extern llvm::Type* const LLVM_DOUBLE;

extern llvm::IntegerType* const LLVM_BOOL;
extern llvm::IntegerType* const LLVM_INT;
extern llvm::IntegerType* const LLVM_INT8;
extern llvm::IntegerType* const LLVM_INT32;
extern llvm::IntegerType* const LLVM_INT64;

extern llvm::PointerType* const LLVM_FLOAT_PTR;
extern llvm::PointerType* const LLVM_DOUBLE_PTR;

extern llvm::PointerType* const LLVM_BOOL_PTR;
extern llvm::PointerType* const LLVM_INT_PTR;
extern llvm::PointerType* const LLVM_INT8_PTR;
extern llvm::PointerType* const LLVM_INT32_PTR;
extern llvm::PointerType* const LLVM_INT64_PTR;


llvm::Type*        llvmType(const ir::Type&,       unsigned addrspace=0);
llvm::StructType*  llvmType(const ir::SetType*,    unsigned addrspace=0,
                            bool packed=false);
llvm::StructType*  llvmType(const ir::UnstructuredSetType&,
                            unsigned addrspace=0, bool packed=false);
llvm::StructType*  llvmType(const ir::GridSetType&,
                            unsigned addrspace=0, bool packed=false);
llvm::PointerType* llvmType(const ir::TensorType&, unsigned addrspace=0);
llvm::PointerType* llvmType(const ir::ArrayType&,  unsigned addrspace=0);
llvm::Type*        llvmType(ir::ScalarType);

llvm::PointerType* llvmPtrType(ir::ScalarType stype, unsigned addrspace);

llvm::PointerType* llvmFloatPtrType(unsigned addrspace=0);
llvm::Type*        llvmFloatType();

llvm::PointerType* llvmComplexPtrType(unsigned addrspace=0);
llvm::StructType*  llvmComplexType();

}}
#endif
