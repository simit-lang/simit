#include "llvm_codegen.h"

#include "llvm/IR/Constants.h"

using namespace simit::internal;

namespace simit {
namespace internal {

llvm::Type *toLLVMType(const simit::ComponentType type) {
  assert(isValidComponentType(type));
  switch (type) {
    case simit::ComponentType::INT:
      return LLVM_INT;
    case simit::ComponentType::FLOAT:
      return LLVM_DOUBLE;
  }
}

llvm::Type *toLLVMType(const std::shared_ptr<Type> &type){
  switch (type->getKind()) {
    case Type::Tensor:
      assert(isValidComponentType(tensorTypePtr(type)->getComponentType()));
      switch (tensorTypePtr(type)->getComponentType()) {
        case simit::ComponentType::INT:
          return LLVM_INTPTR;
        case simit::ComponentType::FLOAT:
          return LLVM_DOUBLEPTR;
      }
      break;
    case Type::Set:
      NOT_SUPPORTED_YET;
      break;
  }
}

llvm::Constant *toLLVMPtr(const std::shared_ptr<Literal> &literal) {
  llvm::Constant *c = (sizeof(void*) == 4)
      ? llvm::ConstantInt::get(llvm::Type::getInt32Ty(LLVM_CONTEXT),
                               (int)(intptr_t)literal->getData())
      : llvm::ConstantInt::get(llvm::Type::getInt64Ty(LLVM_CONTEXT),
                               (intptr_t)literal->getData());

  llvm::Type *ctype = toLLVMType(literal->getType());
  llvm::Constant *cptr = llvm::ConstantExpr::getIntToPtr(c, ctype);
  return cptr;
}

simit::ComponentType llvmToSimitType(const llvm::Type *type) {
  if (type->isPointerTy()) {
    type = type->getPointerElementType();
  }

  if (type->isDoubleTy()) {
    return simit::FLOAT;
  }
  else if (type->isIntegerTy()) {
    return simit::INT;
  }
  else {
    UNREACHABLE;
  }
}

}} // namespace simit::internal
