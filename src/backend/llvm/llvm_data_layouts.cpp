#include "llvm_data_layouts.h"

#include "init.h"
#include "ir.h"
#include "llvm_codegen.h"
#include "llvm_types.h"
#include "types.h"
#include "graph.h"

#include "llvm/IR/Value.h"
#include "llvm/IR/IRBuilder.h"

namespace simit {
namespace backend {

llvm::Value* UnstructuredSetLayout::getSize(unsigned i) {
  iassert(i == 0) << "Only 1 explicit dimension for unstructured sets";
  return builder->CreateExtractValue(value, {0}, util::toString(set)+".size()");
}

llvm::Value* UnstructuredSetLayout::getTotalSize() {
  return builder->CreateExtractValue(value, {0}, util::toString(set)+".size()");
}

int UnstructuredSetLayout::getFieldsOffset() {
  // Must skip size
  return 1;
}

llvm::Value* UnstructuredSetLayout::makeSet(Set *actual, ir::Type type) {
  iassert(actual->getKind() == Set::Unstructured);
  iassert(actual->getCardinality() == 0);

  const ir::UnstructuredSetType *setType = type.toUnstructuredSet();
  llvm::StructType *llvmSetType = llvmType(*setType);
  vector<llvm::Constant*> setData;

  // Set size
  setData.push_back(llvmInt(actual->getSize()));
  // Fields
  for (auto &field : setType->elementType.toElement()->fields) {
    assert(field.type.isTensor());
    setData.push_back(llvmPtr(*field.type.toTensor(),
                              actual->getFieldData(field.name)));
  }
  return llvm::ConstantStruct::get(llvmSetType, setData);
}

void UnstructuredSetLayout::writeSet(
    Set *actual, ir::Type type, void *externPtr) {
  iassert(actual->getKind() == Set::Unstructured);
  iassert(actual->getCardinality() == 0);

  const ir::SetType *setType = type.toSet();

  // Set size
  ((int*)externPtr)[0] = actual->getSize();
  void **externPtrCast = (void**)(((int*)externPtr)+1);
  // Fields
  for (auto &field : setType->elementType.toElement()->fields) {
    assert(field.type.isTensor());
    *externPtrCast = actual->getFieldData(field.name);
    externPtrCast++;
  }
}

llvm::Value* UnstructuredEdgeSetLayout::getEpsArray() {
  return builder->CreateExtractValue(value, {1}, util::toString(set)+".eps()");
}

int UnstructuredEdgeSetLayout::getFieldsOffset() {
  // Must skip size, eps
  return 2;
}

llvm::Value* UnstructuredEdgeSetLayout::makeSet(Set *actual, ir::Type type) {
  iassert(actual->getKind() == Set::Unstructured);

  const ir::UnstructuredSetType *setType = type.toUnstructuredSet();
  llvm::StructType *llvmSetType = llvmType(*setType);
  vector<llvm::Constant*> setData;

  // Set size
  setData.push_back(llvmInt(actual->getSize()));

  // Endpoints index
  setData.push_back(llvmPtr(LLVM_INT_PTR, actual->getEndpointsData()));

  // Fields
  for (auto &field : setType->elementType.toElement()->fields) {
    iassert(field.type.isTensor());
    setData.push_back(llvmPtr(*field.type.toTensor(),
                              actual->getFieldData(field.name)));
  }
  return llvm::ConstantStruct::get(llvmSetType, setData);
}

void UnstructuredEdgeSetLayout::writeSet(Set *actual, ir::Type type,
                                         void *externPtr) {
  iassert(actual->getKind() == Set::Unstructured);

  const ir::SetType *setType = type.toSet();

  // Set size
  ((int*)externPtr)[0] = actual->getSize();
  int **externPtrCast = (int**)(((int*)externPtr)+1);

  // Endpoints index
  externPtrCast[0] = actual->getEndpointsData();

  // Fields
  void **externPtrFieldCast = (void**)(externPtrCast+3);
  for (auto &field : setType->elementType.toElement()->fields) {
    iassert(field.type.isTensor());
    *externPtrFieldCast = actual->getFieldData(field.name);
    externPtrFieldCast++;
  }
}

llvm::Value* LatticeEdgeSetLayout::getSize(unsigned i) {
  iassert(i < set.type().toLatticeLinkSet()->dimensions);
  auto sizes =
      builder->CreateExtractValue(value, {0}, util::toString(set)+".sizes()");
  std::string name = string(sizes->getName()) + "[" + std::to_string(i) + "]";
  auto out = builder->CreateInBoundsGEP(sizes, llvmInt(i), name);
  return builder->CreateLoad(out);
}

llvm::Value* LatticeEdgeSetLayout::getTotalSize() {
  unsigned dims = set.type().toLatticeLinkSet()->dimensions;
  // directional dimension
  llvm::Value *total = llvmInt(set.type().toLatticeLinkSet()->dimensions);
  // lattice sites dimensions
  for (unsigned i = 0; i < dims; ++i) {
    total = builder->CreateMul(total, getSize(i),
                               util::toString(set)+".totalSize()");
  }
  return total;
}

llvm::Value* LatticeEdgeSetLayout::getEpsArray() {
  iassert(!kIndexlessStencils)
      << "Endpoints array undefined when in indexless mode";
  return builder->CreateExtractValue(value, {1}, util::toString(set)+".eps()");
}

int LatticeEdgeSetLayout::getFieldsOffset() {
  // Must skip size, eps
  return 2;
}

llvm::Value* LatticeEdgeSetLayout::makeSet(Set *actual, ir::Type type) {
  iassert(actual->getKind() == Set::LatticeLink);

  const ir::LatticeLinkSetType *setType = type.toLatticeLinkSet();
  llvm::StructType *llvmSetType = llvmType(*setType);
  vector<llvm::Constant*> setData;

  // Set sizes
  unsigned ndims = setType->dimensions;
  const vector<int> &dimensions = actual->getDimensions();
  uassert(dimensions.size() == ndims)
      << "Lattice link set with wrong number of dimensions: "
      << dimensions.size() << " passed, but " << ndims
      << " required";
  setData.push_back(llvmPtr(LLVM_INT_PTR, dimensions.data()));
    
  // CSR data: only set if kIndexlessStencils is false, otherwise
  // we set these to NULL.
  if (kIndexlessStencils) {
    // NULL pointers for endpoints
    setData.push_back(llvmPtr(LLVM_INT_PTR, NULL));
  }
  else {
    // Endpoints index
    setData.push_back(llvmPtr(LLVM_INT_PTR, actual->getEndpointsData()));
  }
    
  // Fields
  for (auto &field : setType->elementType.toElement()->fields) {
    assert(field.type.isTensor());
    setData.push_back(llvmPtr(*field.type.toTensor(),
                              actual->getFieldData(field.name)));
  }
  return llvm::ConstantStruct::get(llvmSetType, setData);
}

void LatticeEdgeSetLayout::writeSet(Set *actual, ir::Type type,void *externPtr){
  iassert(actual->getKind() == Set::LatticeLink);

  const ir::SetType *setType = type.toSet();
  int** externPtrCast = (int**)externPtr;

  // Set sizes
  const vector<int> &dimensions = actual->getDimensions();
  ((const int**)externPtrCast)[0] = dimensions.data();
    
  // CSR data: only set if kIndexlessStencils is false, otherwise
  // we set these to NULL.
  if (kIndexlessStencils) {
    // Three NULL pointers for endpoints, nbrs_start, and nbrs
    externPtrCast[1] = NULL;
    externPtrCast[2] = NULL;
    externPtrCast[3] = NULL;
  }
  else {
    // Endpoints index
    externPtrCast[1] = actual->getEndpointsData();
  }

  void **externPtrFieldCast = (void**)(externPtrCast+4);
  // Fields
  for (auto &field : setType->elementType.toElement()->fields) {
    assert(field.type.isTensor());

    *externPtrFieldCast = actual->getFieldData(field.name);
    externPtrFieldCast++;
  }
}

std::shared_ptr<SetLayout> getSetLayout(
    ir::Expr set, llvm::Value *value, SimitIRBuilder *builder) {
  iassert(set.type().isSet());
  if (set.type().isUnstructuredSet()) {
    if (set.type().toUnstructuredSet()->getCardinality() == 0) {
      return std::shared_ptr<SetLayout>(
          new UnstructuredSetLayout(set, value, builder));
    }
    else {
      return std::shared_ptr<SetLayout>(
          new UnstructuredEdgeSetLayout(set, value, builder));
    }
  }
  else if (set.type().isLatticeLinkSet()) {
    return std::shared_ptr<SetLayout>(
        new LatticeEdgeSetLayout(set, value, builder));
  }
  else {
    unreachable;
    return nullptr;
  }
}

/// Build llvm set struct from runtime Set object
llvm::Value* makeSet(Set *actual, ir::Type type) {
  iassert(type.isSet());
  if (type.isUnstructuredSet()) {
    if (type.toUnstructuredSet()->getCardinality() == 0) {
      return UnstructuredSetLayout::makeSet(actual, type);
    }
    else {
      return UnstructuredEdgeSetLayout::makeSet(actual, type);
    }
  }
  else if (type.isLatticeLinkSet()) {
    return LatticeEdgeSetLayout::makeSet(actual, type);
  }
  else {
    unreachable;
    return nullptr;
  }
}

/// Write set pointers to extern pointer structure
void writeSet(Set *actual, ir::Type type, void *externPtr) {
  iassert(type.isSet());
  if (type.isUnstructuredSet()) {
    if (type.toUnstructuredSet()->getCardinality() == 0) {
      return UnstructuredSetLayout::writeSet(actual, type, externPtr);
    }
    else {
      return UnstructuredEdgeSetLayout::writeSet(actual, type, externPtr);
    }
  }
  else if (type.isLatticeLinkSet()) {
    return LatticeEdgeSetLayout::writeSet(actual, type, externPtr);
  }
  else {
    unreachable;
  }
}

}} // namespace simit::backend
