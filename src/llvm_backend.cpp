#include "llvm_backend.h"

#include <cstdint>
#include <iostream>
#include <stack>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/JIT.h"

#include "llvm_codegen.h"
#include "ir.h"
#include "ir_printer.h"
#include "llvm_function.h"
#include "storage.h"
#include "scopedmap.h"
#include "macros.h"

using namespace std;
using namespace simit::ir;
using namespace simit::internal;

namespace simit {
namespace internal {


// class LLVMBackend
bool LLVMBackend::llvmInitialized = false;

LLVMBackend::LLVMBackend() {
  if (!llvmInitialized) {
    llvm::InitializeNativeTarget();
    llvmInitialized = true;
  }

  module = new llvm::Module("Simit JIT", LLVM_CONTEXT);

  llvm::EngineBuilder engineBuilder(module);
  llvm::ExecutionEngine *ee = engineBuilder.create();
  assert(ee && "Could not create ExecutionEngine");

  executionEngine = std::shared_ptr<llvm::ExecutionEngine>(ee);
  builder = new llvm::IRBuilder<>(LLVM_CONTEXT);

}

LLVMBackend::~LLVMBackend() {
  delete symtable;
  delete builder;
}

simit::Function *LLVMBackend::compile(simit::ir::Func *function) {
  return NULL;
}

}}  // namespace simit::internal
