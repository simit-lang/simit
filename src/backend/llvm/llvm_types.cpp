#include "llvm_types.h"

#include <vector>
#include "types.h"
#include "llvm/IR/Type.h"

using namespace std;
using namespace simit::ir;

#include "llvm_defines.h"

namespace simit {
namespace backend {

llvm::Type* const LLVM_VOID              = llvm::Type::getVoidTy(LLVM_CTX);

llvm::IntegerType* const LLVM_BOOL       = llvm::Type::getInt1Ty(LLVM_CTX);
llvm::IntegerType* const LLVM_INT        = llvm::Type::getInt32Ty(LLVM_CTX);
llvm::IntegerType* const LLVM_INT8       = llvm::Type::getInt8Ty(LLVM_CTX);
llvm::IntegerType* const LLVM_INT32      = llvm::Type::getInt32Ty(LLVM_CTX);
llvm::IntegerType* const LLVM_INT64      = llvm::Type::getInt64Ty(LLVM_CTX);

llvm::Type* const LLVM_FLOAT             = llvm::Type::getFloatTy(LLVM_CTX);
llvm::Type* const LLVM_DOUBLE            = llvm::Type::getDoubleTy(LLVM_CTX);

llvm::PointerType* const LLVM_FLOAT_PTR  = llvm::Type::getFloatPtrTy(LLVM_CTX);
llvm::PointerType* const LLVM_DOUBLE_PTR = llvm::Type::getDoublePtrTy(LLVM_CTX);

llvm::PointerType* const LLVM_BOOL_PTR   = llvm::Type::getInt1PtrTy(LLVM_CTX);
llvm::PointerType* const LLVM_INT_PTR    = llvm::Type::getInt32PtrTy(LLVM_CTX);
llvm::PointerType* const LLVM_INT8_PTR   = llvm::Type::getInt8PtrTy(LLVM_CTX);
llvm::PointerType* const LLVM_INT32_PTR  = llvm::Type::getInt32PtrTy(LLVM_CTX);
llvm::PointerType* const LLVM_INT64_PTR  = llvm::Type::getInt64PtrTy(LLVM_CTX);

/// One for endpoints, two for neighbor index
extern const int NUM_EDGE_INDEX_ELEMENTS = 3;


llvm::Type* llvmType(const Type& type, unsigned addrspace) {
  switch (type.kind()) {
    case Type::Tensor:
      return llvmType(*type.toTensor(), addrspace);
    case Type::Element:
      not_supported_yet;
      break;
    case Type::Set:
      return llvmType(*type.toSet(), addrspace);
    case Type::Tuple:
      ierror << "Tuples not supported in the backend";
      break;
    case Type::Array:
      return llvmType(*type.toArray(), addrspace);
  }
  unreachable;
  return nullptr;
}

// TODO: replace anonymous struct with one struct per element and set type
llvm::StructType *llvmType(const ir::SetType& setType, unsigned addrspace,
                           bool packed) {
  const ElementType *elemType = setType.elementType.toElement();
  vector<llvm::Type*> llvmFieldTypes;

  if (setType.kind == SetType::Unstructured) {
    // Set size
    llvmFieldTypes.push_back(LLVM_INT);

    // Edge indices (if the set is an edge set)
    if (setType.endpointSets.size() > 0) {
      // Endpoints
      llvmFieldTypes.push_back(
          llvm::Type::getInt32PtrTy(LLVM_CTX, addrspace));

      // Neighbor Index
      // row starts (block row)
      llvmFieldTypes.push_back(
          llvm::Type::getInt32PtrTy(LLVM_CTX, addrspace));
      // col indexes (block column)
      llvmFieldTypes.push_back(
          llvm::Type::getInt32PtrTy(LLVM_CTX, addrspace));
    }
  }
  else if (setType.kind == SetType::LatticeLink) {
    int dims = setType.dimensions;
    // Dimension sizes
    for (int i = 0; i < dims; ++i) {
      llvmFieldTypes.push_back(LLVM_INT);
    }
    // NO edge indices
  }
  else {
    not_supported_yet;
  }

  // Fields
  for (const Field &field : elemType->fields) {
    llvmFieldTypes.push_back(llvmType(field.type, addrspace));
  }
  return llvm::StructType::get(LLVM_CTX, llvmFieldTypes, packed);
}

llvm::PointerType* llvmType(const TensorType& type, unsigned addrspace) {
  return llvmPtrType(type.getComponentType(), addrspace);
}

llvm::PointerType* llvmType(const ir::ArrayType& type, unsigned addrspace) {
  return llvmPtrType(type.elementType, addrspace);
}

llvm::Type* llvmType(ScalarType stype) {
  switch (stype.kind) {
    case ScalarType::Int:
      return LLVM_INT;
    case ScalarType::Float:
      return llvmFloatType();
    case ScalarType::Boolean:
      return LLVM_BOOL;
    case ScalarType::Complex:
      return llvmComplexType();
    case ScalarType::String:
      return LLVM_INT8_PTR;
  }
  unreachable;
  return nullptr;
}

llvm::Type *llvmFloatType() {
  return ScalarType::singleFloat() ? LLVM_FLOAT : LLVM_DOUBLE;
}

llvm::StructType *llvmComplexType() {
  vector<llvm::Type*> fieldTypes = {llvmFloatType(), llvmFloatType()};
  const bool packed = true;
  return llvm::StructType::get(LLVM_CTX, fieldTypes, packed);
}

llvm::PointerType *llvmPtrType(ScalarType stype, unsigned addrspace) {
  switch (stype.kind) {
    case ScalarType::Int:
      return llvm::Type::getInt32PtrTy(LLVM_CTX, addrspace);
    case ScalarType::Float:
      return llvmFloatPtrType(addrspace);
    case ScalarType::Boolean:
      return llvm::Type::getInt1PtrTy(LLVM_CTX, addrspace);
    case ScalarType::Complex:
      return llvmComplexPtrType(addrspace);
    case ScalarType::String:
    {
      const auto charPtrType = llvm::Type::getInt8PtrTy(LLVM_CTX, addrspace);
      return llvm::PointerType::get(charPtrType, addrspace);
    }
  }
  unreachable;
  return nullptr;
}

llvm::PointerType *llvmFloatPtrType(unsigned addrspace) {
  if (ScalarType::singleFloat()) {
    return llvm::Type::getFloatPtrTy(LLVM_CTX, addrspace);
  }
  else {
    return llvm::Type::getDoublePtrTy(LLVM_CTX, addrspace);
  }
}

llvm::PointerType *llvmComplexPtrType(unsigned addrspace) {
  return llvm::PointerType::get(llvmComplexType(), addrspace);
}

}}
