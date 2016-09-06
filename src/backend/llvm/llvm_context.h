#ifndef SIMIT_LLVM_CONTEXT_H
#define SIMIT_LLVM_CONTEXT_H

namespace llvm {
class LLVMContext;
}

namespace simit {
namespace backend {
llvm::LLVMContext& getGlobalContext();
}}

#define LLVM_CTX simit::backend::getGlobalContext()
#endif
