#include "llvm_codegen.h"

#include "llvm/IR/Instructions.h"
#include "llvm/IR/IRBuilder.h"
#include <vector>

#include "llvm_types.h"

/**
 * This file is compiled with -fno-rtti to avoid undefined RTTI info for LLVM
 * classes to be linked in. It appears certain methods of IRBuilder need to
 * wrapped in this way.
 */

using namespace llvm;
using namespace std;

namespace simit {
namespace backend {

Value *llvmCreateComplex(LLVMIRBuilder *builder, Value *real, Value *imag) {
  Value *zero = llvmComplex(0, 0);
  Value *partial = builder->CreateInsertValue(zero, real, 0);
  return builder->CreateInsertValue(partial, imag, 1);
}

Value *llvmComplexGetReal(LLVMIRBuilder *builder, Value *c) {
  return builder->CreateExtractValue(c, 0, "real");
}

Value *llvmComplexGetImag(LLVMIRBuilder *builder, Value *c) {
  return builder->CreateExtractValue(c, 1, "imag");
}

ConstantInt *llvmInt(long long int val, unsigned bits) {
  return ConstantInt::get(LLVM_CTX, APInt(bits, val, true));
}

ConstantInt *llvmUInt(long long unsigned int val, unsigned bits) {
  return ConstantInt::get(LLVM_CTX, APInt(bits, val, false));
}

Constant *llvmFP(double val, unsigned bits) {
  return ConstantFP::get(llvmFloatType(), val);
}

Constant* llvmBool(bool val) {
  int intVal = (val) ? 1 : 0;
  return ConstantInt::get(LLVM_CTX, APInt(1, intVal, false));
}

Constant* llvmComplex(double real, double imag) {
  return ConstantStruct::get(llvmComplexType(),
                             llvmFP(real), llvmFP(imag), nullptr);
}

Constant *llvmPtr(PointerType* type, const void* data) {
  Constant* c = (sizeof(void*) == 4)
      ? ConstantInt::get(Type::getInt32Ty(LLVM_CTX), (int)(intptr_t)data)
      : ConstantInt::get(Type::getInt64Ty(LLVM_CTX), (intptr_t)data);
  return ConstantExpr::getIntToPtr(c, type);
}

Value *llvmCreateInBoundsGEP(LLVMIRBuilder *builder, Value *buffer,
                             Value *index, const Twine &name) {
  return builder->CreateInBoundsGEP(buffer, index, name);
}

Value *llvmCreateInBoundsGEP(LLVMIRBuilder *builder, Value *buffer,
                             std::vector<Value*> indices, const Twine &name) {
  return builder->CreateInBoundsGEP(buffer, indices, name);
}

Value *llvmCreateExtractValue(LLVMIRBuilder *builder, Value *agg,
                              ArrayRef<unsigned> idxs, const Twine& name) {
  return builder->CreateExtractValue(agg, idxs, name);
}

#define LLVM_CMP_WRAPPER(NAME) \
  Value *llvm##NAME(LLVMIRBuilder *builder, Value *a, Value *b,   \
                    const Twine& name) {                          \
    return builder->NAME(a, b, name);                             \
  }

LLVM_CMP_WRAPPER(CreateFCmpOEQ)
LLVM_CMP_WRAPPER(CreateICmpEQ)
LLVM_CMP_WRAPPER(CreateFCmpONE)
LLVM_CMP_WRAPPER(CreateICmpNE)
LLVM_CMP_WRAPPER(CreateFCmpOGT)
LLVM_CMP_WRAPPER(CreateICmpSGT)
LLVM_CMP_WRAPPER(CreateFCmpOLT)
LLVM_CMP_WRAPPER(CreateICmpSLT)
LLVM_CMP_WRAPPER(CreateFCmpOGE)
LLVM_CMP_WRAPPER(CreateICmpSGE)
LLVM_CMP_WRAPPER(CreateFCmpOLE)
LLVM_CMP_WRAPPER(CreateICmpSLE)

PHINode *llvmCreatePHI(LLVMIRBuilder *builder, Type *ty, unsigned num,
                       const Twine &name) {
  return builder->CreatePHI(ty, num, name);
}

}}
