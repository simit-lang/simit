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

LLVMFunction::LLVMFunction(simit::internal::Function *simitFunc,
                           llvm::Function *llvmFunc,
                           const shared_ptr<llvm::ExecutionEngine> &llvmFuncEE,
                           const std::vector<std::shared_ptr<Storage>> &storage)
    : simitFunc(simitFunc), llvmFunc(llvmFunc), llvmFuncEE(llvmFuncEE),
  storage(storage), module("Harness", LLVM_CONTEXT) {
  llvmFuncEE->addModule(&module);
}

LLVMFunction::~LLVMFunction() {
  llvmFuncEE->removeModule(&module);
}

void LLVMFunction::bind(const std::vector<std::shared_ptr<Literal>> &arguments,
                        const std::vector<std::shared_ptr<Literal>> &results) {
  // Typecheck:
  auto &formalArguments = simitFunc->getArguments();
  assert(arguments.size() == formalArguments.size());
  for (size_t i=0; i<arguments.size(); ++i) {
    assert(*arguments[i]->getType() == *formalArguments[i]->getType());
  }
  auto &formalResults  = simitFunc->getResults();
  assert(results.size() == formalResults.size());
  for (size_t i=0; i<results.size(); ++i) {
    assert(*results[i]->getType() == *formalResults[i]->getType());
  }

  void *fptr = llvmFuncEE->getPointerToFunction(llvmFunc);
  if (arguments.size() == 0 and results.size() == 0) {
    setRunPtr((RunPtrType)fptr);
  }
  else {
    std::string name = string(llvmFunc->getName()) + "_harness";
    std::vector<std::shared_ptr<Expression>> noArgs;
    llvm::Function *harness = createPrototype(name, noArgs, noArgs,
                                              llvm::Function::InternalLinkage,
                                              &module);
    auto entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", harness);
    llvm::SmallVector<llvm::Value*, 8> args;
    for (auto &argument : arguments) {
      args.push_back(toLLVMPtr(argument));
    }
    for (auto &result : results) {
      args.push_back(toLLVMPtr(result));
    }
    llvm::CallInst *call = llvm::CallInst::Create(llvmFunc, args, "", entry);
    call->setCallingConv(llvmFunc->getCallingConv());
    call->setTailCall();
    llvm::ReturnInst::Create(llvmFunc->getContext(), entry);
    setRunPtr((RunPtrType)llvmFuncEE->getPointerToFunction(harness));
  }
}

void LLVMFunction::print(std::ostream &os) const {
  std::string fstr;
  llvm::raw_string_ostream rsos(fstr);
  llvmFunc->print(rsos);
  os << fstr;
}

}} // unnamed namespace
