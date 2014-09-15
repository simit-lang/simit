#include "llvm_function.h"

#include <string>
#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm_codegen.h"

using namespace std;

namespace simit {
namespace internal {

LLVMFunction::LLVMFunction(const ir::Function &simitFunc,
                           llvm::Function *llvmFunc,
                           const shared_ptr<llvm::ExecutionEngine> &llvmFuncEE,
                           const std::vector<std::shared_ptr<Storage>> &storage)
    : Function(simitFunc), llvmFunc(llvmFunc),
      llvmFuncEE(llvmFuncEE), storage(storage), module("Harness", LLVM_CONTEXT) {
  llvmFuncEE->addModule(&module);
}

LLVMFunction::~LLVMFunction() {
  llvmFuncEE->removeModule(&module);
}

void LLVMFunction::print(std::ostream &os) const {
  std::string fstr;
  llvm::raw_string_ostream rsos(fstr);
  llvmFunc->print(rsos);
  os << fstr;
}

simit::Function::FuncPtrType
LLVMFunction::init(std::map<std::string, Actual> &actuals) {
  void *fptr = llvmFuncEE->getPointerToFunction(llvmFunc);
  if (llvmFunc->getArgumentList().size() == 0) {
    return (FuncPtrType)fptr;
  }
  else {
    std::string name = string(llvmFunc->getName()) + "_harness";
    std::vector<std::shared_ptr<ir::Expression>> noArgs;
    llvm::Function *harness = createPrototype(name, noArgs, noArgs,
                                              llvm::Function::InternalLinkage,
                                              &module);
    auto entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", harness);
    llvm::SmallVector<llvm::Value*, 8> args;

    for (const llvm::Argument &arg : llvmFunc->getArgumentList()) {
      std::string argName(arg.getName());
      assert(actuals.find(argName) != actuals.end());
      auto &actual = actuals[argName];
      switch (actual.getType()->getKind()) {
        case ir::Type::Tensor:
          args.push_back(toLLVMPtr(actual.getTensor()));
          break;
        case ir::Type::Set:
          NOT_SUPPORTED_YET;
          break;
      }
    }
    llvm::CallInst *call = llvm::CallInst::Create(llvmFunc, args, "", entry);
    call->setCallingConv(llvmFunc->getCallingConv());
    call->setTailCall();
    llvm::ReturnInst::Create(llvmFunc->getContext(), entry);
    return (FuncPtrType)llvmFuncEE->getPointerToFunction(harness);
  }
}

}} // unnamed namespace
