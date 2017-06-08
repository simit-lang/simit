#ifndef SIMIT_LLVM_DATA_LAYOUTS
#define SIMIT_LLVM_DATA_LAYOUTS

/// Define data layouts for set types in one consistent location.
/// All references to set data layouts should refer to this interface.

#include "ir.h"

#include "llvm_codegen.h"

namespace llvm {
class Value;
}

namespace simit {
class Set;

namespace ir {
class Type;
}

namespace backend {

class SetLayout {
public:
  /// Get the size of the ith dimension
  virtual llvm::Value* getSize(unsigned i) = 0;
  /// Get the total number of elements in the set
  virtual llvm::Value* getTotalSize() = 0;
  /// Get the endpoints array
  virtual llvm::Value* getEpsArray() = 0;
  /// Get the offset to the fields pointers
  virtual int getFieldsOffset() = 0;
};

/// Unstructured set layout (cardinality 0):
/// <size> <f1> <f2> ...
class UnstructuredSetLayout : public SetLayout {
public:
  virtual llvm::Value* getSize(unsigned i);
  virtual llvm::Value* getTotalSize();

  inline virtual llvm::Value* getEpsArray() {simit_unreachable; return nullptr;}

  virtual int getFieldsOffset();

  static llvm::Value* makeSet(Set *actual, ir::Type type);
  static void writeSet(Set *actual, ir::Type type, void *externPtr);

  UnstructuredSetLayout(ir::Expr set, llvm::Value *value, LLVMIRBuilder *builder)
      : set(set), value(value), builder(builder) {
    simit_iassert(set.type().isUnstructuredSet());
  }
private:
  ir::Expr set;
  llvm::Value *value;
  LLVMIRBuilder *builder;
};


/// Unstructured edge set layout:
/// <size> <eps_ptr> <nbrs_start_ptr> <nbrs_ptr> <f1> <f2> ...
class UnstructuredEdgeSetLayout : public UnstructuredSetLayout {
public:
  virtual llvm::Value* getEpsArray();

  virtual int getFieldsOffset();

  static llvm::Value* makeSet(Set *actual, ir::Type type);
  static void writeSet(Set *actual, ir::Type type, void *externPtr);

  UnstructuredEdgeSetLayout(ir::Expr set, llvm::Value *value,
                            LLVMIRBuilder *builder)
      : UnstructuredSetLayout(set, value, builder),
        set(set), value(value), builder(builder) {
    simit_iassert(set.type().isUnstructuredSet());
  }
private:
  ir::Expr set;
  llvm::Value *value;
  LLVMIRBuilder *builder;
};

/// Grid edge set layout:
/// <sizes_ptr> <eps_ptr> <nbrs_start_ptr> <nbrs_ptr> <f1> <f2> ...
class GridSetLayout : public SetLayout {
public:
  virtual llvm::Value* getSize(unsigned i);
  virtual llvm::Value* getTotalSize();
  virtual llvm::Value* getEpsArray();
  virtual int getFieldsOffset();

  static llvm::Value* makeSet(Set *actual, ir::Type type);
  static void writeSet(Set *actual, ir::Type type, void *externPtr);
  
  GridSetLayout(ir::Expr set, llvm::Value *value, LLVMIRBuilder *builder)
      : set(set), value(value), builder(builder) {}

private:
  ir::Expr set;
  llvm::Value *value;
  LLVMIRBuilder *builder;
};

std::shared_ptr<SetLayout> getSetLayout(
    ir::Expr set, llvm::Value *value, LLVMIRBuilder *builder);

/// Build llvm set struct from runtime Set object
llvm::Value* makeSet(Set *actual, ir::Type type);

/// Write set pointers to extern pointer structure
void writeSet(Set *actual, ir::Type type, void *externPtr);

}} // namespace simit::backend

#endif // SIMIT_LLVM_DATA_LAYOUTS
