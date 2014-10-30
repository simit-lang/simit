#include "llvm_function.h"

#include <string>
#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/PassManager.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Transforms/Scalar.h"

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

  llvm::FunctionPassManager fpm(module);
  fpm.add(new llvm::DataLayout(*executionEngine->getDataLayout()));

  // Basic optimizations
  fpm.add(llvm::createBasicAliasAnalysisPass());
  fpm.add(llvm::createInstructionCombiningPass());
  fpm.add(llvm::createGVNPass());
  fpm.add(llvm::createCFGSimplificationPass());
  fpm.add(llvm::createPromoteMemoryToRegisterPass());

  // Loop optimizations
  fpm.add(llvm::createLICMPass());
  fpm.add(llvm::createLoopStrengthReducePass());

  fpm.doInitialization();
  fpm.run(*llvmFunc);

  requiresInitCall = simitFunc.getTemporaries().size() > 0;
  deinit = nullptr;
}

LLVMFunction::~LLVMFunction() {
  if (deinit) {
    deinit();
  }
  executionEngine->removeModule(module);
  delete executionEngine;
  delete module;
}

void LLVMFunction::print(std::ostream &os) const {
  std::string fstr;
  llvm::raw_string_ostream rsos(fstr);
  llvmFunc->print(rsos);
//  module->dump();
  os << rsos.str();
}

LLVMFunction::FuncPtrType
LLVMFunction::createHarness(const std::string &name,
                            const llvm::SmallVector<llvm::Value*,8> &args) {
  llvm::Function *llvmFunc = module->getFunction(name);
  std::string harnessName = name + ".harness";
  llvm::Function *harness = createFunction(harnessName, {}, {}, module);
  auto entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", harness);
  llvm::CallInst *call = llvm::CallInst::Create(llvmFunc, args, "",entry);
  call->setCallingConv(llvmFunc->getCallingConv());
  llvm::ReturnInst::Create(module->getContext(), entry);
  return ((FuncPtrType)executionEngine->getPointerToFunction(harness));
}

simit::Function::FuncPtrType LLVMFunction::init(const vector<string> &formals,
                                                map<string, Actual> &actuals) {
  if (llvmFunc->getArgumentList().size() == 0) {
    if (requiresInitCall) {
      std::string name = llvmFunc->getName();
      llvm::Function *initFunc = module->getFunction(name+".init");
      ((FuncPtrType)executionEngine->getPointerToFunction(initFunc))();

      llvm::Function *deinitFunc = module->getFunction(name+".deinit");
      deinit = ((FuncPtrType)executionEngine->getPointerToFunction(deinitFunc));
    }

    return (FuncPtrType)executionEngine->getPointerToFunction(llvmFunc);
  }
  else {
    llvm::SmallVector<llvm::Value*, 8> args;
    for (const std::string &formal : formals) {
      assert(actuals.find(formal) != actuals.end());
      Actual &actual = actuals.at(formal);
      switch (actual.getType().kind()) {
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
          const ir::SetType *setType = actual.getType().toSet();
          const SetBase *set = actual.getSet();

          llvm::StructType *llvmSetType = createLLVMType(setType);

          vector<llvm::Constant*> setData;

          // Set size
          setData.push_back(llvmInt(set->getSize()));

          // Edge indices (if the set is an edge set)
          if (setType->endpointSets.size() > 0) {
            setData.push_back(llvmPtr(LLVM_INTPTR, getEndpointsPtr(set)));
          }

          // Fields
          for (auto &field : setType->elementType.toElement()->fields) {
            assert(field.second.type.isTensor());
            setData.push_back(llvmPtr(field.second.type,
                                      getFieldPtr(set, field.first)));
          }

          llvm::Value *llvmSet= llvm::ConstantStruct::get(llvmSetType, setData);
          args.push_back(llvmSet);
          break;
        }
        case ir::Type::Tuple: {
          NOT_SUPPORTED_YET;
          break;
        }
      }
    }

    // Init function
    if (requiresInitCall) {
      createHarness(string(llvmFunc->getName())+".init", args)();
      deinit = createHarness(string(llvmFunc->getName())+".deinit", args);
    }

    // Compute function
    return createHarness(llvmFunc->getName(), args);
  }
}

}} // unnamed namespace
