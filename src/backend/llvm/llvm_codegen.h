#ifndef SIMIT_LLVM_CODEGEN_H
#define SIMIT_LLVM_CODEGEN_H

#include <string>
#include <vector>

#include "llvm/IR/Function.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm_defines.h"

namespace simit {
namespace ir {
struct TensorType;
struct Literal;
class Var;
}

namespace backend {

typedef llvm::IRBuilder<true, llvm::ConstantFolder,
                        llvm::IRBuilderDefaultInserter<true>> LLVMIRBuilder;

class SimitIRBuilder : public LLVMIRBuilder {
public:
  SimitIRBuilder(llvm::LLVMContext &C) : LLVMIRBuilder(C) {}
  llvm::Value* CreateComplex(llvm::Value *real, llvm::Value *imag);
  llvm::Value* ComplexGetReal(llvm::Value *c);
  llvm::Value* ComplexGetImag(llvm::Value *c);
};

llvm::ConstantInt* llvmInt(long long int val, unsigned bits=32);
llvm::ConstantInt* llvmUInt(long long unsigned int val, unsigned bits=32);
llvm::Constant*    llvmFP(double val, unsigned bits=64);
llvm::Constant*    llvmBool(bool val);
llvm::Constant*    llvmComplex(double real, double imag);

// Simit-specific utilities

/// The number of index struct elements that are compiled into an edge struct.
extern const int NUM_EDGE_INDEX_ELEMENTS;

llvm::Constant* llvmPtr(llvm::PointerType* type, const void *data);

llvm::Constant* llvmPtr(const ir::TensorType& type, const void *data,
                        unsigned addrspace=0);
llvm::Constant* llvmPtr(const ir::Literal& literal);

llvm::Constant* llvmVal(const ir::TensorType& type, const void *data);
llvm::Constant* llvmVal(const ir::Literal& literal);

llvm::Constant* defaultInitializer(llvm::Type* type);

/// Creates an llvm function prototype
llvm::Function *createPrototypeLLVM(const std::string& name,
                                    const std::vector<std::string>& argNames,
                                    const std::vector<llvm::Type*>& argTypes,
                                    llvm::Module* module,
                                    bool externalLinkage,
                                    bool doesNotThrow=true);
llvm::Function* createPrototype(const std::string& name,
                                const std::vector<ir::Var>& arguments,
                                const std::vector<ir::Var>& results,
                                llvm::Module* module,
                                bool externalLinkage,
                                bool doesNotThrow=true,
                                bool scalarsByValue=true,
                                unsigned addrspace=0);

llvm::GlobalVariable* createGlobal(llvm::Module *module, const ir::Var& var,
                                   llvm::GlobalValue::LinkageTypes linkage,
                                   unsigned addrspace, bool packed);

}}
#endif
