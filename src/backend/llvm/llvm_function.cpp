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

#include "llvm_types.h"
#include "llvm_codegen.h"

#include "backend/actual.h"
#include "graph.h"
#include "indices.h"
#include "util/collections.h"

using namespace std;
using namespace simit::ir;

namespace simit {
namespace backend {

typedef void (*FuncPtrType)();

LLVMFunction::LLVMFunction(ir::Func func, llvm::Function* llvmFunc,
                           llvm::Module* module,
                           std::shared_ptr<llvm::EngineBuilder> engineBuilder)
    : Function(func), llvmFunc(llvmFunc), module(module),
      engineBuilder(engineBuilder), executionEngine(engineBuilder->create()),
      initialized(false), deinit(nullptr) {

  for (const ir::Var& arg : func.getArguments()) {
    actuals[arg.getName()] = new Actual(false);
  }
  for (const ir::Var& res : func.getResults()) {
    // Skip results that alias an argument
    if (actuals.find(res.getName()) != actuals.end()) {
      actuals[res.getName()]->setOutput(true);
      continue;
    }
    actuals[res.getName()] = new Actual(true);
  }

  for (const string& global : getGlobals()) {
    Type type = getGlobalType(global);

    llvm::GlobalValue* llvmGlobal = module->getNamedValue(global);
    void** globalPtr = (void**)executionEngine->getPointerToGlobal(llvmGlobal);
    *globalPtr = nullptr;
    this->globals.insert({global, globalPtr});
  }
}

LLVMFunction::~LLVMFunction() {
  if (deinit) {
    deinit();
  }
  for (auto &actual : actuals) {
    delete actual.second;
  }
}

void LLVMFunction::bindTensor(const std::string& bindable, void* data) {
  iassert(hasBindable(bindable));
  if (hasArg(bindable)) {
    actuals.at(bindable)->bindTensor(data);
    initialized = false;
  }
  else if (hasGlobal(bindable)) {
    *globals.at(bindable) = data;
  }
}

void LLVMFunction::bindSet(const std::string& bindable, simit::Set* set) {
  tassert(hasArg(bindable)) << "Global sets not supported yet";
  actuals[bindable]->bindSet(set);
  actuals[bindable]->setOutput(true);
  initialized = false;
}

Function::FuncType LLVMFunction::init() {
  initialized = true;
  vector<string> formals = getArgs();

  iassert(formals.size() == llvmFunc->getArgumentList().size());

  if (llvmFunc->getArgumentList().size() == 0) {
    llvm::Function *initFunc = getInitFunc();
    llvm::Function *deinitFunc = getDeinitFunc();
    ((FuncPtrType)executionEngine->getPointerToFunction(initFunc))();
    auto fptr = executionEngine->getPointerToFunction(deinitFunc);
    deinit = FuncType((FuncPtrType)fptr);
    return FuncType((FuncPtrType)executionEngine->getPointerToFunction(llvmFunc));
  }
  else {
    llvm::SmallVector<llvm::Value*, 8> args;
    auto llvmArgIt = llvmFunc->getArgumentList().begin();
    for (const std::string &formal : formals) {
      assert(actuals.find(formal) != actuals.end());
      Actual *actual = actuals.at(formal);

      ir::Type type = getArgType(formal);
      switch (type.kind()) {
        case ir::Type::Tensor: {
          const ir::TensorType* tensorType = type.toTensor();
          void* tensorData = actual->getTensorData();
          llvm::Value *llvmActual = (llvmArgIt->getType()->isPointerTy())
              ? llvmPtr(*tensorType, tensorData)
              : llvmVal(*tensorType, tensorData);
          args.push_back(llvmActual);
          break;
        }
        case ir::Type::Element: {
          not_supported_yet;
          break;
        }
        case ir::Type::Set: {
          const ir::SetType *setType = type.toSet();
          Set *set = actual->getSet();

          llvm::StructType *llvmSetType = llvmType(*setType);

          vector<llvm::Constant*> setData;

          // Set size
          setData.push_back(llvmInt(set->getSize()));

          // Edge indices (if the set is an edge set)
          if (setType->endpointSets.size() > 0) {
            // Endpoints index
            setData.push_back(llvmPtr(LLVM_INT_PTR, set->getEndpointsData()));

            // Edges index
            // TODO

            // Neighbor index
            const internal::NeighborIndex *nbrs = set->getNeighborIndex();
            setData.push_back(llvmPtr(LLVM_INT_PTR, nbrs->getStartIndex()));
            setData.push_back(llvmPtr(LLVM_INT_PTR, nbrs->getNeighborIndex()));
          }

          // Fields
          for (auto &field : setType->elementType.toElement()->fields) {
            assert(field.type.isTensor());
            setData.push_back(llvmPtr(*field.type.toTensor(),
                                      set->getFieldData(field.name)));
          }

          llvm::Value *llvmSet= llvm::ConstantStruct::get(llvmSetType, setData);
          args.push_back(llvmSet);
          break;
        }
        case ir::Type::Tuple: {
          not_supported_yet;
          break;
        }
        case ir::Type::Array: {
          not_supported_yet;
          break;
        }
      }
      ++llvmArgIt;
    }

    // Create Init/deinit function harnesses
    createHarness(string(llvmFunc->getName())+".init", args)();
    deinit = createHarness(string(llvmFunc->getName())+".deinit", args);

    // Compute function
    auto harness = createHarness(llvmFunc->getName(), args);
    iassert(!llvm::verifyModule(*module))
        << "LLVM module does not pass verification";
    return harness;
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

LLVMFunction::FuncType
LLVMFunction::createHarness(const std::string &name,
                            const llvm::SmallVector<llvm::Value*,8> &args) {
  llvm::Function *llvmFunc = module->getFunction(name);
  std::string harnessName = name + ".harness";
  llvm::Function *harness = createPrototype(harnessName, {}, {}, module, true);
  auto entry = llvm::BasicBlock::Create(LLVM_CTX, "entry", harness);
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
