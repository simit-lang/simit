#ifndef SIMIT_LLVM_VERSIONS_H
#define SIMIT_LLVM_VERSIONS_H

/// Detect version of LLVM and set up includes and macros based on this.

// LLVM 3.4:
// * Verifier.h moved
// * Needs to use JIT, not MCJIT, to support NVVM. This is the only version of
//   LLVM we support that also emits supported IR to NVVM.
#if LLVM_MAJOR_VERSION <= 3 && LLVM_MINOR_VERSION <= 4

#include "llvm/Analysis/Verifier.h"
#define LLVM_USE_JIT
#define LLVM_EB_WRAP(EB) (EB->setUseMCJIT(false))

#else

#include "llvm/IR/Verifier.h"
#define LLVM_USE_MCJIT
#if LLVM_MAJOR_VERSION <= 3 && LLVM_MINOR_VERSION <= 5
#define LLVM_EB_WRAP(EB) (EB->setUseMCJIT(true))
#else
#define LLVM_EB_WRAP(EB) EB
#endif

#endif

// LLVM 3.5:
// * Pass modules by unique_ptr
// * Metadata no longer identical to Value
#if LLVM_MAJOR_VERSION <= 3 && LLVM_MINOR_VERSION <= 5
#define LLVM_MOD_WRAP(MOD) MOD
#define LLVM_MD_WRAP(MD) MD
#define LLVM_Metadata llvm::Value
#else
#define LLVM_MOD_WRAP(MOD) std::unique_ptr<llvm::Module>(MOD)
#define LLVM_MD_WRAP(MD) llvm::ValueAsMetadata::get(MD)
#define LLVM_Metadata llvm::Metadata
#endif

// LLVM 3.6:
// * PassManager moved to LegacyPassManager
// * ConstantExpr::getGetElementPtr signature changed
#if LLVM_MAJOR_VERSION <=3 && LLVM_MINOR_VERSION <= 6

#include "llvm/PassManager.h"
namespace simit {
namespace backend {
typedef llvm::PassManager PassManager;
typedef llvm::FunctionPassManager FunctionPassManager;
}} 
#define LLVMgetGetElementPtr(GLOBAL, IDX) \
  llvm::ConstantExpr::getGetElementPtr(GLOBAL, IDX)

#else

#include "llvm/IR/LegacyPassManager.h"
namespace simit {
namespace backend {
typedef llvm::legacy::PassManager PassManager;
typedef llvm::legacy::FunctionPassManager FunctionPassManager;
}}
#define LLVMgetGetElementPtr(GLOBAL, IDX) \
  llvm::ConstantExpr::getGetElementPtr(nullptr, GLOBAL, IDX)

#endif


#ifdef LLVM_USE_JIT
#include "llvm/ExecutionEngine/JIT.h"
#else
#include "llvm/ExecutionEngine/MCJIT.h"
#endif

namespace simit {
namespace backend {

// Setting the data layout is handled in different ways in various LLVM versions
inline void setDataLayout(FunctionPassManager &fpm, llvm::DataLayout &dl,
                          llvm::Module *module) {
#if LLVM_MAJOR_VERSION <= 3 && LLVM_MINOR_VERSION <= 4
  fpm.add(new llvm::DataLayout(dl));
#elif LLVM_MAJOR_VERSION <= 3 && LLVM_MINOR_VERSION <= 6
  fpm.add(new llvm::DataLayoutPass(dl));
#else
  module->setDataLayout(dl);
#endif
}

// Getting the address of a global variable varies based on JIT vs. MCJIT
inline uint64_t getGlobalAddr(std::string name, llvm::Module *module,
                              std::shared_ptr<llvm::ExecutionEngine> ee) {
#ifdef LLVM_USE_JIT
  llvm::GlobalValue* tmp = module->getNamedValue(name);
  return (uint64_t)ee->getPointerToGlobal(tmp);
#else
  return ee->getGlobalValueAddress(name);
#endif
}

// Getting the address of a function varies based on JIT vs. MCJIT
typedef void (*VoidFuncPtr)();
inline VoidFuncPtr getGlobalFunc(llvm::Function *func,
                                 llvm::ExecutionEngine *ee) {
#ifdef LLVM_USE_JIT
  return (VoidFuncPtr)ee->getPointerToFunction(func);
#else
  uint64_t addr = ee->getFunctionAddress(func->getName());
  iassert(addr != 0)
      << "MCJIT prevents modifying the module after ExecutionEngine code "
      << "generation. Ensure all functions are created before fetching "
      << "function addresses.";
  return reinterpret_cast<VoidFuncPtr>(addr);
#endif
}

}}


#endif
