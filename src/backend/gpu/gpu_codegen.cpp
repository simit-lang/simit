#include "gpu_codegen.h"

#include "nvvm.h"

#include <fstream>
#include "llvm/Support/raw_ostream.h"
#include "llvm/Bitcode/ReaderWriter.h"

#include "error.h"
#include "backend/llvm/llvm_codegen.h"
#include "backend/llvm/llvm_versions.h"

namespace simit {
namespace backend {

namespace {

/// checkNVVMCall - Verifies that NVVM result code is success, or terminate
/// otherwise.
void checkNVVMCall(nvvmResult res) {
  iassert(res == NVVM_SUCCESS) << "libnvvm call failed";
}

std::string utostr(uint num) {
  return std::to_string(num);
}

}  // anonymous namespace

llvm::Module *createNVVMModule(std::string name) {
  llvm::Module *module = new llvm::Module(name, LLVM_CTX);

  // Set appropriate data layout
  if (sizeof(void*) == 8) {
    module->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
                          "i64:64:64-f32:32:32-f64:64:64-v16:16:16-v32:32:32-"
                          "v64:64:64-v128:128:128-n16:32:64");
    module->setTargetTriple("nvptx64-nvidia-cuda");
  }
  else {
    not_supported_yet << "We haven't tested PTX32 at all, and several things are known to overlook this.";
    module->setDataLayout("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
                          "i64:64:64-f32:32:32-f64:64:64-v16:16:16-v32:32:32-"
                          "v64:64:64-v128:128:128-n16:32:64");
    module->setTargetTriple("nvptx-nvidia-cuda");
  }
  return module;
}

extern "C" unsigned char simit_gpu_libdevice_compute_20[];
extern "C" int simit_gpu_libdevice_compute_20_length;
extern "C" unsigned char simit_gpu_libdevice_compute_30[];
extern "C" int simit_gpu_libdevice_compute_30_length;
extern "C" unsigned char simit_gpu_libdevice_compute_35[];
extern "C" int simit_gpu_libdevice_compute_35_length;

extern "C" unsigned char simit_gpu_intrinsics[];
extern "C" int simit_gpu_intrinsics_length;

std::string generatePtx(llvm::Module *module,
                        int devMajor, int devMinor,
                        const char *moduleName) {
  nvvmProgram compileUnit;
  nvvmResult res;

  // NVVM Initialization
  checkNVVMCall(nvvmCreateProgram(&compileUnit));
  
  // Add libdevice (math libraries, etc.) as initial module
  //
  // Reference:
  // http://docs.nvidia.com/cuda/libdevice-users-guide/basic-usage.html
  //
  // The device to libdevice version mapping is weird (note 3.1-3.4=compute_20)
  //    2.0 ≤ Arch < 3.0   libdevice.compute_20.XX.bc
  //    Arch = 3.0         libdevice.compute_30.XX.bc
  //    3.1 ≤ Arch < 3.5   libdevice.compute_20.XX.bc
  //    Arch = 3.5         libdevice.compute_35.XX.bc
  const char *libdevice;
  int libdevice_length;
  if (devMajor == 3 && devMajor == 0) {
    libdevice = reinterpret_cast<const char*>(simit_gpu_libdevice_compute_30);
    libdevice_length = simit_gpu_libdevice_compute_30_length;
  } else if (devMajor == 3 && devMajor == 5) {
    libdevice = reinterpret_cast<const char*>(simit_gpu_libdevice_compute_35);
    libdevice_length = simit_gpu_libdevice_compute_35_length;
  } else {
    libdevice = reinterpret_cast<const char*>(simit_gpu_libdevice_compute_20);
    libdevice_length = simit_gpu_libdevice_compute_20_length;
  }
  checkNVVMCall(nvvmAddModuleToProgram(compileUnit,
                                       libdevice, libdevice_length,
                                       "libdevice"));

  // Add intrinsics bitcode library
  const char *intrinsics = reinterpret_cast<const char*>(simit_gpu_intrinsics);
  checkNVVMCall(nvvmAddModuleToProgram(compileUnit, intrinsics,
                                       simit_gpu_intrinsics_length,
                                       "intrinsics"));

  // Export IR to string
  std::string llStr;
  llvm::raw_string_ostream llOstr(llStr);
  llOstr << *module;
  std::ofstream llFile("simit.ll", std::ofstream::trunc);
  llFile << llStr << std::endl;
  llFile.close();
  
  std::string bcStr;
  llvm::raw_string_ostream bcOstr(bcStr);
  llvm::WriteBitcodeToFile(module, bcOstr);
  bcOstr.flush();
  
  std::cerr << "Bitcode: " << bcStr.size() << " bytes\n";
  
  std::ofstream bcFile("simit.bc", std::ofstream::trunc | std::ofstream::binary);
  bcFile << bcStr << std::endl;
  bcFile.close();
  
  // Create NVVM compilation unit from LLVM IR
  checkNVVMCall(nvvmAddModuleToProgram(compileUnit,
                                       // NOTE: this can also use llStr:
                                       bcStr.c_str(), bcStr.size(),
                                       moduleName));

  std::string computeArg = "-arch=compute_";
  computeArg += utostr(devMajor);
  computeArg += utostr(devMinor);

  const char *options[] = { computeArg.c_str() };

  // Compile LLVM IR into PTX
  res = nvvmCompileProgram(compileUnit, 1, options);
  if (res != NVVM_SUCCESS) {
    std::cerr << "nvvmCompileProgram failed!" << std::endl;
    size_t logSize;
    nvvmGetProgramLogSize(compileUnit, &logSize);
    char *msg = new char[logSize];
    nvvmGetProgramLog(compileUnit, msg);
    std::cerr << msg << std::endl;
    delete [] msg;
    ierror << "nvvmCompileProgram failed";
  }

  size_t ptxSize;
  checkNVVMCall(nvvmGetCompiledResultSize(compileUnit, &ptxSize));
  char *ptx = new char[ptxSize];
  checkNVVMCall(nvvmGetCompiledResult(compileUnit, ptx));
  checkNVVMCall(nvvmDestroyProgram(&compileUnit));

  std::string ptxStr(ptx);
  delete [] ptx;
  return ptxStr;
}

void addNVVMAnnotation(llvm::Value *target, std::string annot,
                       llvm::Value *value, llvm::Module *module) {
  LLVM_Metadata *mdVals[] = {
    LLVM_MD_WRAP(target),
    llvm::MDString::get(LLVM_CTX, annot),
    LLVM_MD_WRAP(value)
  };
  llvm::MDNode *node = llvm::MDNode::get(LLVM_CTX, mdVals);
  llvm::NamedMDNode *nvvmAnnot = module
      ->getOrInsertNamedMetadata("nvvm.annotations");
  nvvmAnnot->addOperand(node);
}

}}
