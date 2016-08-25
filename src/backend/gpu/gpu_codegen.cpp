#include "gpu_codegen.h"

#include "nvvm.h"

#include <fstream>
#include "llvm/ADT/SmallString.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Bitcode/ReaderWriter.h"
#include "llvm/Analysis/TargetLibraryInfo.h"

#include "llvm/IR/Intrinsics.h"

#include "error.h"
#include "backend/llvm/llvm_codegen.h"
#include "backend/llvm/llvm_util.h"

std::string libdevicePtxCache;
std::string intrinsicsPtxCache;

/// Declare method in the NVPTX LLVM library
namespace llvm {
ModulePass *createNVVMReflectPass(const StringMap<int>& Mapping);
}

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

void setNVVMModuleProps(llvm::Module *module) {
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
}

// LLVM Bitreader requires input buffers to align to 4 bytes
int alignBitreaderLength(int length) {
  if (length % 4 != 0) {
    length += 4 - (length % 4);
  }
  return length;
}

}  // anonymous namespace

llvm::Module *createNVVMModule(std::string name) {
  llvm::Module *module = new llvm::Module(name, LLVM_CTX);
  setNVVMModuleProps(module);
  return module;
}

std::string generatePtx(llvm::Module *module, int devMajor, int devMinor) {
  std::string mcpu;
  if (devMajor == 3 && devMinor >= 5 ||
      devMajor > 3) {
    mcpu = "sm_35";
  }
  else if (devMajor >= 3 && devMinor >= 0) {
    mcpu = "sm_30";
  }
  else {
    mcpu = "sm_20";
  }

  // Select target given the module's triple
  llvm::Triple triple(module->getTargetTriple());
  std::string errStr;
  const llvm::Target* target = nullptr;
  target = llvm::TargetRegistry::lookupTarget(triple.str(), errStr);
  iassert(target) << errStr;

  llvm::TargetOptions targetOptions;

  std::string features = "+ptx40";

  std::unique_ptr<llvm::TargetMachine> targetMachine(
      target->createTargetMachine(triple.str(), mcpu, features, targetOptions,
                                  // llvm::Reloc::PIC_,
                                  llvm::Reloc::Default,
                                  llvm::CodeModel::Default,
                                  llvm::CodeGenOpt::Default));

  // Make a passmanager and add emission to string
  llvm::legacy::PassManager pm;
  pm.add(new llvm::TargetLibraryInfoWrapperPass(triple));

  // Set up constant NVVM reflect mapping
  llvm::StringMap<int> reflectMapping;
  reflectMapping["__CUDA_FTZ"] = 1; // Flush denormals to zero
  pm.add(llvm::createNVVMReflectPass(reflectMapping));
  pm.add(llvm::createAlwaysInlinerPass());
  targetMachine->Options.MCOptions.AsmVerbose = true;
  llvm::SmallString<8> ptxStr;
  llvm::raw_svector_ostream outStream(ptxStr);
  outStream.SetUnbuffered();
  bool failed = targetMachine->addPassesToEmitFile(
      pm, outStream, targetMachine->CGFT_AssemblyFile, false);
  iassert(!failed);

  pm.run(*module);
  outStream.flush();
  return ptxStr.str();
}

extern "C" unsigned char simit_gpu_libdevice_compute_20[];
extern "C" int simit_gpu_libdevice_compute_20_length;
extern "C" unsigned char simit_gpu_libdevice_compute_30[];
extern "C" int simit_gpu_libdevice_compute_30_length;
extern "C" unsigned char simit_gpu_libdevice_compute_35[];
extern "C" int simit_gpu_libdevice_compute_35_length;

extern "C" unsigned char simit_gpu_intrinsics[];
extern "C" int simit_gpu_intrinsics_length;

std::vector<std::string> generateLibraryPtx(int devMajor, int devMinor) {
  if (libdevicePtxCache.size() > 0 &&
      intrinsicsPtxCache.size() > 0) {
    return {libdevicePtxCache, intrinsicsPtxCache};
  }

  // Build libdevice (math libraries, etc.) module
  //
  // Reference:
  // http://docs.nvidia.com/cuda/libdevice-users-guide/basic-usage.html
  //
  // The device to libdevice version mapping is weird (note 3.1-3.4=compute_20)
  //    2.0 ≤ Arch < 3.0   libdevice.compute_20.XX.bc
  //    Arch = 3.0         libdevice.compute_30.XX.bc
  //    3.1 ≤ Arch < 3.5   libdevice.compute_20.XX.bc
  //    Arch = 3.5         libdevice.compute_35.XX.bc
  // Identify device by Compute API level
  const char *libdevice;
  int libdevice_length;
  if (devMajor == 3 && devMinor >= 5 ||
      devMajor > 3) {
    libdevice = reinterpret_cast<const char*>(simit_gpu_libdevice_compute_35);
    libdevice_length = simit_gpu_libdevice_compute_35_length;
  }
  else if (devMajor == 3 && devMinor >= 0) {
    libdevice = reinterpret_cast<const char*>(simit_gpu_libdevice_compute_30);
    libdevice_length = simit_gpu_libdevice_compute_30_length;
  } else {
    libdevice = reinterpret_cast<const char*>(simit_gpu_libdevice_compute_20);
    libdevice_length = simit_gpu_libdevice_compute_20_length;
  }
  llvm::SMDiagnostic errReport;
  libdevice_length = alignBitreaderLength(libdevice_length);
  llvm::MemoryBufferRef libdeviceBuf(
      llvm::StringRef(libdevice, libdevice_length), "libdevice");
  std::unique_ptr<llvm::Module> libdeviceModule =
      llvm::parseIR(libdeviceBuf, errReport, LLVM_CTX);
  iassert((bool)libdeviceModule)
      << "Failed to load libdevice: " << printToString(errReport);
  setNVVMModuleProps(libdeviceModule.get());
  logModule(libdeviceModule.get(), "simit-libdevice.ll");
  std::string libdevicePtx = generatePtx(libdeviceModule.get(), devMajor, devMinor);

  // Build intrinsics module
  const char *intrinsics = reinterpret_cast<const char*>(simit_gpu_intrinsics);
  int intrinsics_length = alignBitreaderLength(simit_gpu_intrinsics_length);
  llvm::MemoryBufferRef intrinsicsBuf(
      llvm::StringRef(intrinsics, intrinsics_length), "intrinsics");
  std::unique_ptr<llvm::Module> intrinsicsModule =
      llvm::parseIR(intrinsicsBuf, errReport, LLVM_CTX);
  iassert((bool)intrinsicsModule)
      << "Failed to load intrinsics: " << printToString(errReport);
  setNVVMModuleProps(intrinsicsModule.get());
  logModule(intrinsicsModule.get(), "simit-intrinsics.ll");
  std::string intrinsicsPtx = generatePtx(intrinsicsModule.get(), devMajor, devMinor);

  // Cache the compiled libries
  libdevicePtxCache = libdevicePtx;
  intrinsicsPtxCache = intrinsicsPtx;

  return {libdevicePtx, intrinsicsPtx};
}

void addNVVMAnnotation(llvm::Value *target, std::string annot,
                       llvm::Value *value, llvm::Module *module) {
  llvm::Metadata *mdVals[] = {
    llvm::ValueAsMetadata::get(target),
    llvm::MDString::get(LLVM_CTX, annot),
    llvm::ValueAsMetadata::get(value)
  };
  llvm::MDNode *node = llvm::MDNode::get(LLVM_CTX, mdVals);
  llvm::NamedMDNode *nvvmAnnot = module
      ->getOrInsertNamedMetadata("nvvm.annotations");
  nvvmAnnot->addOperand(node);
}

}}
