#include "gpu_codegen.h"

#include "nvvm.h"

#include "error.h"
#include "llvm_codegen.h"

namespace simit {
namespace internal {

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
  llvm::Module *module = new llvm::Module(name, LLVM_CONTEXT);

  // Set appropriate data layout
  if (sizeof(void*) == 8)
    module->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
                          "i64:64:64-f32:32:32-f64:64:64-v16:16:16-v32:32:32-"
                          "v64:64:64-v128:128:128-n16:32:64");
  else
    module->setDataLayout("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
                          "i64:64:64-f32:32:32-f64:64:64-v16:16:16-v32:32:32-"
                          "v64:64:64-v128:128:128-n16:32:64");
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

std::string generatePtx(const std::string &module,
                        int devMajor, int devMinor,
                        const char *moduleName) {
  std::cout << "DEBUG: generatePtx" << std::endl
	    << module << std::endl;
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

  // Create NVVM compilation unit from LLVM IR
  checkNVVMCall(nvvmAddModuleToProgram(compileUnit,
                                       module.c_str(), module.size(),
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

  std::cout << "Ptx code: " << std::endl << std::string(ptx) << std::endl;

  return std::string(ptx);
}

void addNVVMAnnotation(llvm::Value *target, std::string annot,
                       llvm::Value *value, llvm::Module *module) {
  llvm::Value *mdVals[] = {
    target, llvm::MDString::get(LLVM_CONTEXT, annot), value
  };
  llvm::MDNode *node = llvm::MDNode::get(LLVM_CONTEXT, mdVals);
  llvm::NamedMDNode *nvvmAnnot = module
      ->getOrInsertNamedMetadata("nvvm.annotations");
  nvvmAnnot->addOperand(node);
}

}}  // namespace simit::internal
