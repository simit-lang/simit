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
#include "indices.h"

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
                           bool requiresInit, llvm::Module *module)
    : Function(simitFunc), llvmFunc(llvmFunc), module(module),
      executionEngine(createExecutionEngine(module)),
      requiresInit(requiresInit), deinit(nullptr) {

  llvm::FunctionPassManager fpm(module);
  fpm.add(new llvm::DataLayout(*executionEngine->getDataLayout()));

  // Basic optimizations
  fpm.add(llvm::createBasicAliasAnalysisPass());
  fpm.add(llvm::createInstructionCombiningPass());
  fpm.add(llvm::createGVNPass());
  fpm.add(llvm::createCFGSimplificationPass());
  fpm.add(llvm::createPromoteMemoryToRegisterPass());
  fpm.add(llvm::createAggressiveDCEPass());

  // Loop optimizations
  fpm.add(llvm::createLICMPass());
  fpm.add(llvm::createLoopStrengthReducePass());

  fpm.doInitialization();
  fpm.run(*llvmFunc);
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
  module->print(rsos, nullptr);
  os << rsos.str();
}

simit::Function::FuncType LLVMFunction::init(const vector<string> &formals,
                                             map<string, Actual> &actuals) {
  if (llvmFunc->getArgumentList().size() == 0) {
    if (requiresInit) {
      llvm::Function *initFunc = getInitFunc();
      llvm::Function *deinitFunc = getDeinitFunc();
      ((FuncPtrType)executionEngine->getPointerToFunction(initFunc))();
      auto fptr = executionEngine->getPointerToFunction(deinitFunc);
      deinit = FuncType((FuncPtrType)fptr);
    }
    return FuncType((FuncPtrType)executionEngine->getPointerToFunction(llvmFunc));
  }
  else {
    llvm::SmallVector<llvm::Value*, 8> args;
    for (const std::string &formal : formals) {
      assert(actuals.find(formal) != actuals.end());
      Actual &actual = actuals.at(formal);
      switch (actual.getType().kind()) {
        case ir::Type::Tensor: {
          auto actualPtr = ir::to<ir::Literal>(*actual.getTensor());
          args.push_back(llvmPtr(const_cast<ir::Literal*>(actualPtr)));
          break;
        }
        case ir::Type::Element: {
          not_supported_yet;
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
            // Endpoints index
            setData.push_back(llvmPtr(LLVM_INTPTR, set->getEndpointsData()));

            // Edges index
            // TODO

            // Neighbor index
            const internal::NeighborIndex *nbrs = set->getNeighborIndex();
            setData.push_back(llvmPtr(LLVM_INTPTR,nbrs->getStartIndex()));
            setData.push_back(llvmPtr(LLVM_INTPTR,nbrs->getNeighborIndex()));
          }

          // Fields
          for (auto &field : setType->elementType.toElement()->fields) {
            assert(field.type.isTensor());
            setData.push_back(llvmPtr(field.type, set->getFieldData(field.name)));
          }

          llvm::Value *llvmSet= llvm::ConstantStruct::get(llvmSetType, setData);
          args.push_back(llvmSet);
          break;
        }
        case ir::Type::Tuple: {
          not_supported_yet;
          break;
        }
      }
    }

    // Init function
    if (requiresInit) {
      createHarness(string(llvmFunc->getName())+".init", args)();
      deinit = createHarness(string(llvmFunc->getName())+".deinit", args);
    }

    // Compute function
    return createHarness(llvmFunc->getName(), args);
  }
}

LLVMFunction::FuncType
LLVMFunction::createHarness(const std::string &name,
                            const llvm::SmallVector<llvm::Value*,8> &args) {
  llvm::Function *llvmFunc = module->getFunction(name);
  std::string harnessName = name + ".harness";
  llvm::Function *harness = createPrototype(harnessName, {},{}, module);
  auto entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", harness);
  llvm::CallInst *call = llvm::CallInst::Create(llvmFunc, args, "",entry);
  call->setCallingConv(llvmFunc->getCallingConv());
  llvm::ReturnInst::Create(module->getContext(), entry);
  return ((FuncPtrType)executionEngine->getPointerToFunction(harness));
}

llvm::Function *LLVMFunction::getInitFunc() const {
  return module->getFunction(string(llvmFunc->getName()) + ".init");
}

llvm::Function *LLVMFunction::getDeinitFunc() const {
  return module->getFunction(string(llvmFunc->getName()) + ".deinit");
}

}} // unnamed namespace
