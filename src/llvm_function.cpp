#include "llvm_function.h"

#include <string>
#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm_codegen.h"
#include "graph.h"

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
LLVMFunction::init(const std::vector<std::string> &formals,
                   std::map<std::string, Actual> &actuals) {
  void *fptr = llvmFuncEE->getPointerToFunction(llvmFunc);
  if (llvmFunc->getArgumentList().size() == 0) {
    return (FuncPtrType)fptr;
  }
  else {
    std::string name = string(llvmFunc->getName()) + "_harness";
    std::vector<std::shared_ptr<ir::Argument>> noArgs;
    std::vector<std::shared_ptr<ir::Result>> noResults;
    llvm::Function *harness = createFunction(name, noArgs, noResults,
                                             llvm::Function::InternalLinkage,
                                             &module);
    auto entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", harness);

    llvm::SmallVector<llvm::Value*, 8> args;
    for (const std::string &formal : formals) {
      assert(actuals.find(formal) != actuals.end());
      Actual &actual = actuals.at(formal);
      switch (actual.getType()->getKind()) {
        case ir::Type::Tensor:
          args.push_back(llvmPtr(actual.getTensor()));
          break;
        case ir::Type::Set:
          const SetBase *set = actual.getSet();
          args.push_back(getInt32(set->getSize()));
          const ir::SetType *setType = setTypePtr(actual.getType());
          for (auto &field : setType->getElementType()->getFields()) {
            assert(field.second->isTensor());
            ir::TensorType *tensorType = tensorTypePtr(field.second);
            args.push_back(llvmPtr(tensorType, getFieldPtr(set,field.first)));
          }
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
