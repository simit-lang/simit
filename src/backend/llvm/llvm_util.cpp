#include "llvm_util.h"

#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"

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

}}
