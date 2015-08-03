#include "llvm_function.h"

#include <string>
#include <vector>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Support/raw_ostream.h"

#if LLVM_MAJOR_VERSION <= 3 && LLVM_MINOR_VERSION <= 4
#include "llvm/Analysis/Verifier.h"
#else
#include "llvm/IR/Verifier.h"
#endif

#include "llvm_codegen.h"
#include "graph.h"
#include "indices.h"

using namespace std;

namespace simit {
namespace backend {

typedef void (*FuncPtrType)();

LLVMFunction::LLVMFunction(ir::Func simitFunc, llvm::Function *llvmFunc,
                           bool requiresInit, llvm::Module *module,
                           std::shared_ptr<llvm::EngineBuilder> engineBuilder)
    : Function(simitFunc), llvmFunc(llvmFunc), module(module),
      engineBuilder(engineBuilder), executionEngine(engineBuilder->create()),
      requiresInit(requiresInit),
  deinit(nullptr) {
}

LLVMFunction::~LLVMFunction() {
  if (deinit) {
    deinit();
  }
}

void LLVMFunction::print(std::ostream &os) const {
  std::string fstr;
  llvm::raw_string_ostream rsos(fstr);
  module->print(rsos, nullptr);
  os << rsos.str();
}

void LLVMFunction::printMachine(std::ostream &os) const {
  llvm::TargetMachine *target = engineBuilder->selectTarget();
  target->Options.PrintMachineCode = true;
  llvm::ExecutionEngine *printee(engineBuilder->create(target));
  printee->getPointerToFunction(llvmFunc);
  target->Options.PrintMachineCode = false;
}

backend::Function::FuncType
LLVMFunction::init(const vector<string> &formals, map<string, Actual> &actuals){
  iassert(formals.size() == llvmFunc->getArgumentList().size());

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
    auto llvmArgIt = llvmFunc->getArgumentList().begin();
    for (const std::string &formal : formals) {
      assert(actuals.find(formal) != actuals.end());
      Actual &actual = actuals.at(formal);

      switch (actual.getType().kind()) {
        case ir::Type::Tensor: {
          ir::Expr tensor = *actual.getTensor();
          const ir::Literal *literal = ir::to<ir::Literal>(tensor);

          llvm::Value *llvmActual = (llvmArgIt->getType()->isPointerTy())
              ? llvmPtr(const_cast<ir::Literal*>(literal))
              : llvmVal(literal);
          args.push_back(llvmActual);
          break;
        }
        case ir::Type::Element: {
          not_supported_yet;
          break;
        }
        case ir::Type::Set: {
          const ir::SetType *setType = actual.getType().toSet();
          Set *set = actual.getSet();

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
      ++llvmArgIt;
    }

    // Init function
    if (requiresInit) {
      createHarness(string(llvmFunc->getName())+".init", args)();
      deinit = createHarness(string(llvmFunc->getName())+".deinit", args);
    }

    // Compute function
    auto harness = createHarness(llvmFunc->getName(), args);
    iassert(!llvm::verifyModule(*module))
        << "LLVM module does not pass verification";
    return harness;
  }
}

LLVMFunction::FuncType
LLVMFunction::createHarness(const std::string &name,
                            const llvm::SmallVector<llvm::Value*,8> &args) {
  llvm::Function *llvmFunc = module->getFunction(name);
  std::string harnessName = name + ".harness";
  llvm::Function *harness = createPrototype(harnessName, {}, {}, module, true);
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
