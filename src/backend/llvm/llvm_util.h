#ifndef SIMIT_LLVM_UTIL_H
#define SIMIT_LLVM_UTIL_H

#include <ostream>

namespace llvm {
class Type;
class Value;
class Module;
}

namespace simit {
namespace backend {

std::ostream &operator<<(std::ostream &os, const llvm::Type &);
std::ostream &operator<<(std::ostream &os, const llvm::Value &);
std::ostream &operator<<(std::ostream &os, const llvm::Module &);

}}
#endif
