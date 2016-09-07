#include "llvm_context.h"

#include "llvm/IR/LLVMContext.h"

namespace simit {
namespace backend {

llvm::LLVMContext& getGlobalContext() {
  static llvm::LLVMContext ctx;
  return ctx;
}

}}
