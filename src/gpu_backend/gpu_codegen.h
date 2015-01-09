#ifndef GPU_CODEGEN_H
#define GPU_CODEGEN_H

#include "cuda.h"
#include "error.h"
#include <iostream>

#include "llvm/IR/Module.h"

// This will output the proper CUDA error strings in the event that a CUDA
// host call returns an error
#define checkCudaErrors(err)  __checkCudaErrors (err, __FILE__, __LINE__)

// These are the inline versions for all of the SDK helper functions
inline void __checkCudaErrors(CUresult err, const char *file, const int line) {
  if(CUDA_SUCCESS != err) {
    ierror << "checkCudaErrors() Driver API error = " << err
           << "from file <" << file
           << ", line " << line;
  }
}

namespace simit {
namespace internal {

// Make an llvm module with appropriate data layout for NVVM
llvm::Module *createNVVMModule(std::string name);

// Uses libnvvm to compile an LLVM IR module to PTX.
std::string generatePtx(const std::string &module,
                        int devMajor, int devMinor,
                        const char *moduleName);

}}  // namespace simit::internal

#endif
