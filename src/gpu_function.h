#ifndef SIMIT_GPU_FUNCTION_H
#define SIMIT_GPU_FUNCTION_H

#include <string>
#include <map>
#include <vector>

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"

#include "function.h"

namespace simit {
namespace internal {

class GPUFunction : public simit::Function {
 public:
  GPUFunction(simit::ir::Func simitFunc, llvm::Function *llvmFunc, llvm::Module *llvmModule);
  ~GPUFunction();

  void print(std::ostream &os) const;

 private:
  FuncType init(const std::vector<std::string> &formals,
                   std::map<std::string, Actual> &actuals);

  std::unique_ptr<llvm::Function> llvmFunc;
  std::unique_ptr<llvm::Module> llvmModule;
};

}
}

#endif // SIMIT_GPU_FUNCTION_H
