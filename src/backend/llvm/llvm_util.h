#ifndef SIMIT_LLVM_UTIL_H
#define SIMIT_LLVM_UTIL_H

#include <ostream>

namespace llvm {
class Function;
class ExecutionEngine;
class Type;
class Value;
class Module;
class SMDiagnostic;
}

namespace simit {
namespace backend {

std::ostream &operator<<(std::ostream &os, const llvm::Type &);
std::ostream &operator<<(std::ostream &os, const llvm::Value &);
std::ostream &operator<<(std::ostream &os, const llvm::Module &);

std::string printToString(const llvm::SMDiagnostic &);

// Get the address of a function with checking
typedef void (*VoidFuncPtr)();
VoidFuncPtr getGlobalFunc(llvm::Function *func, llvm::ExecutionEngine *ee);

void logModule(llvm::Module *module, std::string fileName);

}}
#endif
