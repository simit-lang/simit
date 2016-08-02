#ifndef GPU_CODEGEN_H
#define GPU_CODEGEN_H

#include "cuda.h"
#include "error.h"
#include <iostream>

#include "llvm/IR/Module.h"
#include "backend/llvm/llvm_defines.h"
#include "backend/llvm/llvm_types.h"

// This will output the proper CUDA error strings in the event that a CUDA
// host call returns an error
#define checkCudaErrors(err)  __checkCudaErrors (err, __FILE__, __LINE__)

// These are the inline versions for all of the SDK helper functions
inline void __checkCudaErrors(CUresult err, const char *file, const int line) {
  if(CUDA_SUCCESS != err) {
    const char *errName;
    const char *errStr;
    cuGetErrorName(err, &errName);
    cuGetErrorString(err, &errStr);
    ierror << "checkCudaErrors() Driver API error = " << errName
           << "(" << err << "):\n"
           << errStr << "\n"
           << "from file <" << file
           << ", line " << line;
  }
}

namespace simit {
namespace backend {

// CUDA-specific LLVM types
inline llvm::StructType *getOrCreateDim3Ty() {
  static llvm::StructType *dim3Ty;
  if (!dim3Ty) {
    std::vector<llvm::Type*> dim3Types = { LLVM_INT, LLVM_INT, LLVM_INT };
    dim3Ty = llvm::StructType::create(
        llvm::ArrayRef<llvm::Type*>(dim3Types), "dim3");
  }
  return dim3Ty;
}

inline llvm::PointerType *getOrCreateCUStreamPtrTy() {
  static llvm::PointerType *cuStreamPtrTy;
  if (!cuStreamPtrTy) {
    llvm::StructType *cuStreamTy = llvm::StructType::create(
      LLVM_CTX, "struct.CUstream_st");
    cuStreamPtrTy = llvm::PointerType::get(cuStreamTy, 0);
  }
  return cuStreamPtrTy;
}

// Make an llvm module with appropriate data layout for NVVM
llvm::Module *createNVVMModule(std::string name);

// Uses libnvvm to compile an LLVM IR module to PTX.
std::string generatePtx(llvm::Module *module,
                        int devMajor, int devMinor,
                        const char *moduleName);

// Add an NVVM annotation to a given LLVM entity
void addNVVMAnnotation(llvm::Value *target, std::string annot,
                       llvm::Value *value, llvm::Module *module);

}}
#endif
