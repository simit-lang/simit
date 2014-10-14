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

llvm::ExecutionEngine *createExecutionEngine(llvm::Module *module) {
  llvm::EngineBuilder engineBuilder(module);
  llvm::ExecutionEngine *ee = engineBuilder.create();
  assert(ee && "Could not create ExecutionEngine");
  return ee;
}

LLVMFunction::LLVMFunction(ir::Func simitFunc, llvm::Function *llvmFunc,
                           llvm::Module *module)
    : Function(simitFunc), llvmFunc(llvmFunc), module(module),
      executionEngine(createExecutionEngine(module)) {
  executionEngine->getPointerToFunction(llvmFunc);  // Compile function
}

LLVMFunction::~LLVMFunction() {
  executionEngine->removeModule(module);
  delete executionEngine;
  delete module;
}

void LLVMFunction::print(std::ostream &os) const {
  std::string fstr;
  llvm::raw_string_ostream rsos(fstr);
  llvmFunc->print(rsos);
  os << fstr;
}

simit::Function::FuncPtrType LLVMFunction::init(const vector<string> &formals,
                                                map<string, Actual> &actuals) {
  void *fptr = executionEngine->getPointerToFunction(llvmFunc);
  if (llvmFunc->getArgumentList().size() == 0) {
    return (FuncPtrType)fptr;
  }
  else {
    std::string name = string(llvmFunc->getName()) + "_harness";
    std::vector<ir::Expr> noArgs;
    std::vector<ir::Expr> noResults;
    llvm::Function *harness = createFunction(name, noArgs, noResults, module);
    auto entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", harness);

    llvm::SmallVector<llvm::Value*, 8> args;
    for (const std::string &formal : formals) {
      assert(actuals.find(formal) != actuals.end());
      Actual &actual = actuals.at(formal);
      switch (actual.getType().kind()) {
        case ir::Type::Scalar: // fall-through
        case ir::Type::Tensor: {
          auto actualPtr = ir::to<ir::Literal>(actual.getTensor()->expr());
          args.push_back(llvmPtr(const_cast<ir::Literal*>(actualPtr)));
          break;
        }
        case ir::Type::Element: {
          NOT_SUPPORTED_YET;
          break;
        }
        case ir::Type::Set: {
          const SetBase *set = actual.getSet();
          args.push_back(llvmInt32(set->getSize()));
          const ir::SetType *setType = actual.getType().toSet();
          for (auto &field : setType->elementType.toElement()->fields) {
            assert(field.second.isTensor());
            args.push_back(llvmPtr(field.second, getFieldPtr(set,field.first)));
          }
          break;
        }
        case ir::Type::Tuple: {
          NOT_SUPPORTED_YET;
          break;
        }
      }
    }

    llvm::CallInst *call = llvm::CallInst::Create(llvmFunc, args, "", entry);
    call->setCallingConv(llvmFunc->getCallingConv());
    call->setTailCall();
    llvm::ReturnInst::Create(llvmFunc->getContext(), entry);
    return (FuncPtrType)executionEngine->getPointerToFunction(harness);
  }
}

}} // unnamed namespace
