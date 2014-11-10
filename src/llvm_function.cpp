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
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Vectorize.h"
#include "llvm/Transforms/IPO.h"

#include "llvm/Support/Host.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Target/TargetLibraryInfo.h"
#include "llvm/IR/DataLayout.h"
#include "llvm/Target/TargetSubtargetInfo.h"

#include "llvm_codegen.h"
#include "graph.h"
#include "dis.h"

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
      requiresInit(requiresInit), deinit(nullptr),
      disassembler(new DisassemblerJITEventListener()) {

  string err;
  llvm::TargetOptions options;
  options.UnsafeFPMath = true;
  options.LessPreciseFPMADOption = true;
  auto target = llvm::TargetRegistry::getClosestTargetForJIT(err);
  auto targetmachine = target->createTargetMachine(llvm::sys::getDefaultTargetTriple(), llvm::sys::getHostCPUName(), "", options);
  const llvm::DataLayout *datalayout = targetmachine->getDataLayout();
  
  module->setTargetTriple(targetmachine->getTargetTriple());
  module->setDataLayout(datalayout->getStringRepresentation());

  llvm::PassManager mpm;
  llvm::TargetLibraryInfo *TLI = new llvm::TargetLibraryInfo(llvm::Triple(module->getTargetTriple()));
  mpm.add(TLI);
  mpm.add(new llvm::DataLayout(*datalayout));
  targetmachine->addAnalysisPasses(mpm);
  
  llvm::FunctionPassManager fpm(module);
  
  fpm.add(new llvm::DataLayout(*datalayout));
  targetmachine->addAnalysisPasses(fpm);

#if 0
  // Basic optimizations
  fpm.add(llvm::createBasicAliasAnalysisPass());
  fpm.add(llvm::createInstructionCombiningPass());
  fpm.add(llvm::createGVNPass());
  fpm.add(llvm::createCFGSimplificationPass());
  fpm.add(llvm::createPromoteMemoryToRegisterPass());

  // Loop optimizations
  fpm.add(llvm::createLICMPass());
  fpm.add(llvm::createLoopStrengthReducePass());
#else
  llvm::PassManagerBuilder Builder;
  Builder.OptLevel = 3;
  Builder.BBVectorize = Builder.LoopVectorize = true;
  Builder.populateFunctionPassManager(fpm);

  // This causes segfaults on mpm.run(*module) for unknown reasons:
  // Builder.populateModulePassManager(mpm);
  // Manually building a near-equivalent pass list, instead:
  {
    // mpm.add(llvm::createCFLAliasAnalysisPass());
    mpm.add(llvm::createTypeBasedAliasAnalysisPass());
    // mpm.add(llvm::createScopedNoAliasAAPass());
    mpm.add(llvm::createBasicAliasAnalysisPass());
    
    // causes segfault?!:
    // mpm.add(llvm::createIPSCCPPass());              // IP SCCP
    // mpm.add(llvm::createGlobalOptimizerPass());     // Optimize out global vars
    mpm.add(llvm::createDeadArgEliminationPass());  // Dead argument elimination
    mpm.add(llvm::createInstructionCombiningPass());// Clean up after IPCP & DAE
    mpm.add(llvm::createCFGSimplificationPass());   // Clean up after IPCP & DAE
    
    // this breaks vectorization?!
    // mpm.add(llvm::createPruneEHPass());             // Remove dead EH info
    // mpm.add(llvm::createFunctionAttrsPass());       // Set readonly/readnone attrs
    // mpm.add(llvm::createArgumentPromotionPass());   // Scalarize uninlined fn args
    // mpm.add(llvm::createScalarReplAggregatesPass(-1, false));
    
    mpm.add(llvm::createEarlyCSEPass());              // Catch trivial redundancies
    mpm.add(llvm::createJumpThreadingPass());         // Thread jumps.
    mpm.add(llvm::createCorrelatedValuePropagationPass()); // Propagate conditionals
    mpm.add(llvm::createCFGSimplificationPass());     // Merge & remove BBs
    mpm.add(llvm::createInstructionCombiningPass());  // Combine silly seq's
    
    mpm.add(llvm::createTailCallEliminationPass()); // Eliminate tail calls
    mpm.add(llvm::createCFGSimplificationPass());     // Merge & remove BBs
    mpm.add(llvm::createReassociatePass());           // Reassociate expressions
    mpm.add(llvm::createLoopRotatePass());            // Rotate Loop
    mpm.add(llvm::createLICMPass());                  // Hoist loop invariants
    mpm.add(llvm::createLoopUnswitchPass());
    mpm.add(llvm::createInstructionCombiningPass());
    mpm.add(llvm::createIndVarSimplifyPass());        // Canonicalize indvars
    mpm.add(llvm::createLoopIdiomPass());             // Recognize idioms like memset.
    mpm.add(llvm::createLoopDeletionPass());          // Delete dead loops
    
    mpm.add(llvm::createLoopUnrollPass());    // Unroll small loops
    
    // mpm.add(llvm::createMergedLoadStoreMotionPass()); // Merge ld/st in diamonds
    mpm.add(llvm::createGVNPass());  // Remove redundancies
    mpm.add(llvm::createMemCpyOptPass());             // Remove memcpy / form memset
    mpm.add(llvm::createSCCPPass());                  // Constant prop with SCCP
    
    mpm.add(llvm::createInstructionCombiningPass());
    mpm.add(llvm::createJumpThreadingPass());         // Thread jumps
    mpm.add(llvm::createCorrelatedValuePropagationPass());
    mpm.add(llvm::createDeadStoreEliminationPass());  // Delete dead stores
    
    mpm.add(llvm::createLoopRerollPass());

    // mpm.add(llvm::createLoadCombinePass());
    mpm.add(llvm::createAggressiveDCEPass());         // Delete dead instructions
    mpm.add(llvm::createCFGSimplificationPass()); // Merge & remove BBs
    mpm.add(llvm::createInstructionCombiningPass());  // Clean up after everything.
    
    mpm.add(llvm::createBarrierNoopPass());
    mpm.add(llvm::createLoopVectorizePass());
    mpm.add(llvm::createInstructionCombiningPass());
    
    mpm.add(llvm::createSLPVectorizerPass());   // Vectorize parallel scalar chains.
    mpm.add(llvm::createBBVectorizePass());
    mpm.add(llvm::createInstructionCombiningPass());
    mpm.add(llvm::createGVNPass()); // Remove redundancies
  }
#endif
  
  fpm.doInitialization();
  fpm.run(*llvmFunc);
  fpm.doFinalization();
  
  mpm.run(*module);

  if (getenv("SIMIT_LOG_ASM")) {
    executionEngine->RegisterJITEventListener(disassembler);
  }
  if (getenv("SIMIT_LOG_LLVM")) {
    print(std::cerr);
  }
}

LLVMFunction::~LLVMFunction() {
  if (deinit) {
    deinit();
  }
  executionEngine->removeModule(module);
  delete executionEngine;
  delete module;
  delete disassembler;
}

void LLVMFunction::print(std::ostream &os) const {
  std::string fstr;
  llvm::raw_string_ostream rsos(fstr);
  module->print(rsos, nullptr);
  os << rsos.str();
}

simit::Function::FuncPtrType LLVMFunction::init(const vector<string> &formals,
                                                map<string, Actual> &actuals) {
  if (llvmFunc->getArgumentList().size() == 0) {
    if (requiresInit) {
      llvm::Function *initFunc = getInitFunc();
      llvm::Function *deinitFunc = getDeinitFunc();
      ((FuncPtrType)executionEngine->getPointerToFunction(initFunc))();
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
            assert(field.type.isTensor());
            setData.push_back(llvmPtr(field.type, getFieldPtr(set,field.name)));
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
    if (requiresInit) {
      createHarness(string(llvmFunc->getName())+".init", args)();
      deinit = createHarness(string(llvmFunc->getName())+".deinit", args);
    }

    // Compute function
    return createHarness(llvmFunc->getName(), args);
  }
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

llvm::Function *LLVMFunction::getInitFunc() const {
  return module->getFunction(string(llvmFunc->getName()) + ".init");
}

llvm::Function *LLVMFunction::getDeinitFunc() const {
  return module->getFunction(string(llvmFunc->getName()) + ".deinit");
}

}} // unnamed namespace
