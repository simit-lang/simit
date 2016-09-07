#include "llvm_util.h"

#include "error.h"

#include "llvm/ExecutionEngine/ExecutionEngine.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/Module.h"

#include <fstream>

namespace simit {
namespace backend {

std::ostream &operator<<(std::ostream &os, const llvm::Type &type) {
  std::string str;
  llvm::raw_string_ostream ss(str);
  type.print(ss);
  return os << ss.str();
}

std::ostream &operator<<(std::ostream &os, const llvm::Value &value) {
  std::string str;
  llvm::raw_string_ostream ss(str);
  value.print(ss);
  return os << ss.str();
}

std::ostream &operator<<(std::ostream &os, const llvm::Module &module) {
  std::string fstr;
  llvm::raw_string_ostream rsos(fstr);
  module.print(rsos, nullptr);
  os << rsos.str();
  return os;
}

std::string printToString(const llvm::SMDiagnostic &diag) {
  std::string errStr;
  llvm::raw_string_ostream rsos(errStr);
  diag.print(diag.getFilename().data(), rsos);
  return rsos.str();
}

VoidFuncPtr getGlobalFunc(llvm::Function *func, llvm::ExecutionEngine *ee) {
  uint64_t addr = ee->getFunctionAddress(func->getName());
  iassert(addr != 0)
      << "MCJIT prevents modifying the module after ExecutionEngine code "
      << "generation. Ensure all functions are created before fetching "
      << "function addresses.";
  return reinterpret_cast<VoidFuncPtr>(addr);
}


void logModule(llvm::Module *module, std::string fileName) {
#ifdef SIMIT_DEBUG
  std::string llStr;
  llvm::raw_string_ostream llOstr(llStr);
  llOstr << *module;
  std::ofstream llFile(fileName, std::ofstream::trunc);
  llFile << llStr << std::endl;
  llFile.close();
#endif
}

}}
