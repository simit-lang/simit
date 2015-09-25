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
#include "tensor_index.h"
#include "path_indices.h"
#include "util/collections.h"
#include "util/util.h"
#include "llvm_util.h"

using namespace std;
using namespace simit::ir;

namespace simit {
namespace backend {

typedef void (*FuncPtrType)();

LLVMFunction::LLVMFunction(ir::Func func, const ir::Storage &storage,
                           llvm::Function* llvmFunc, llvm::Module* module,
                           std::shared_ptr<llvm::EngineBuilder> engineBuilder)
    : Function(func), llvmFunc(llvmFunc), module(module),
      engineBuilder(engineBuilder), executionEngine(engineBuilder->create()),
      initialized(false), deinit(nullptr), storage(storage) {

  const Environment& env = getEnvironment();

  // Initialize extern pointers
  for (const VarMapping& externMapping : env.getExterns()) {
    Var bindable = externMapping.getVar();

    // Store a pointer to each of the bindable's extern in externPtrs
    vector<void**> extPtrs;
    for (const Var& ext : externMapping.getMappings()) {
      llvm::GlobalValue* llvmExt = module->getNamedValue(ext.getName());
      void** extPtr = (void**)executionEngine->getPointerToGlobal(llvmExt);
      *extPtr = nullptr;
      extPtrs.push_back(extPtr);
    }
    iassert(!util::contains(this->externPtrs, bindable.getName()));
    this->externPtrs.insert({bindable.getName(), extPtrs});
  }

  // Initialize temporary pointers
  for (const Var& tmp : env.getTemporaries()) {
    iassert(tmp.getType().isTensor())
        << "Only support tensor temporaries";
    llvm::GlobalValue* llvmTmp = module->getNamedValue(tmp.getName());
    void** tmpPtr = (void**)executionEngine->getPointerToGlobal(llvmTmp);
    *tmpPtr = nullptr;
    temporaryPtrs.insert({tmp.getName(), tmpPtr});
  }

  // Initialize tensorIndex ptrs
  for (const TensorIndex& tensorIndex : env.getTensorIndices()) {
    const Var& coords = tensorIndex.getCoordArray();
    llvm::GlobalValue* llvmCoords = module->getNamedValue(coords.getName());
    const uint32_t** coordsPtr =
        (const uint32_t**)executionEngine->getPointerToGlobal(llvmCoords);
    *coordsPtr = nullptr;

    const Var& sinks = tensorIndex.getSinkArray();
    llvm::GlobalValue* llvmSinks = module->getNamedValue(sinks.getName());
    const uint32_t** sinksPtr =
        (const uint32_t**)executionEngine->getPointerToGlobal(llvmSinks);
    *sinksPtr = nullptr;

    const pe::PathExpression& pexpr = tensorIndex.getPathExpression();
    tensorIndexPtrs.insert({pexpr, {coordsPtr, sinksPtr}});
  }

}

LLVMFunction::~LLVMFunction() {
  if (deinit) {
    deinit();
  }
  for (auto& tmpPtr : temporaryPtrs) {
    free(*tmpPtr.second);
    *tmpPtr.second = nullptr;
  }
}

void LLVMFunction::bind(const std::string& name, simit::Set* set) {
  iassert(hasBindable(name));
  iassert(getBindableType(name).isSet());

  if (hasArg(name)) {
    arguments[name] = std::unique_ptr<Actual>(new SetActual(set));
    initialized = false;
  }
  else {
    globals[name] = std::unique_ptr<Actual>(new SetActual(set));
    const ir::SetType* setType = getGlobalType(name).toSet();

    // Write set size to extern
    iassert(util::contains(externPtrs, name) && externPtrs.at(name).size()==1);
    auto externSizePtr = (int*)externPtrs.at(name)[0];
    *externSizePtr = set->getSize();

    // Write field pointers to extern
    void** externFieldsPtr = (void**)(externSizePtr + 1);
    for (auto& field : setType->elementType.toElement()->fields) {
      *externFieldsPtr = set->getFieldData(field.name);
      ++externFieldsPtr;
    }
  }
}

void LLVMFunction::bind(const std::string& name, void* data) {
  iassert(hasBindable(name));
  if (hasArg(name)) {
    arguments[name] = std::unique_ptr<Actual>(new TensorActual(data));
    initialized = false;
  }
  else if (hasGlobal(name)) {
    globals[name] = std::unique_ptr<Actual>(new TensorActual(data));
    iassert(util::contains(externPtrs, name) && externPtrs.at(name).size()==1);
    *externPtrs.at(name)[0] = data;
  }
}

void LLVMFunction::bind(const std::string& name, const int* rowPtr,
                        const int* colInd, void* data) {
  iassert(hasBindable(name));
  tassert(!hasArg(name)) << "Only support global sparse matrices";

  if (hasGlobal(name)) {
    iassert(util::contains(externPtrs,name))
        << "extern " << util::quote(name) << " does not have any extern ptrs";
    iassert(externPtrs.at(name).size() == 3)
        << "extern " << util::quote(name) << " has wrong size "
        << externPtrs.at(name).size();

    // Sparse matrix externs are ordered: data, rowPtr, colInd
    *externPtrs.at(name)[0] = data;
    *externPtrs.at(name)[1] = (void*)rowPtr;
    *externPtrs.at(name)[2] = (void*)colInd;
  }
}

size_t LLVMFunction::size(const ir::IndexDomain& dimension) {
  size_t result = 1;
  for (const ir::IndexSet& indexSet : dimension.getIndexSets()) {
    switch (indexSet.getKind()) {
      case ir::IndexSet::Range:
        result *= indexSet.getSize();
        break;
      case ir::IndexSet::Set: {
        ir::Expr setExpr = indexSet.getSet();
        iassert(ir::isa<ir::VarExpr>(setExpr))
            << "Attempting to get the static size of a runtime dynamic set: "
            << quote(setExpr);
        string setName = ir::to<ir::VarExpr>(setExpr)->var.getName();

        iassert(util::contains(arguments, setName) ||
                util::contains(globals, setName));
        Actual* setActual = util::contains(arguments, setName)
                            ? arguments.at(setName).get()
                            : globals.at(setName).get();
        iassert(isa<SetActual>(setActual));
        Set* set = to<SetActual>(setActual)->getSet();
        result *= set->getSize();
        break;
      }
      case ir::IndexSet::Single:
      case ir::IndexSet::Dynamic:
        not_supported_yet;
    }
    iassert(result != 0);
  }
  return result;
}

Function::FuncType LLVMFunction::init() {
  pe::PathIndexBuilder piBuilder;

  for (auto& pair : arguments) {
    string name = pair.first;
    Actual* actual = pair.second.get();
    if (isa<SetActual>(actual)) {
      Set* set = to<SetActual>(actual)->getSet();
      piBuilder.bind(name,set);
    }
  }

  const Environment& environment = getEnvironment();

  // Initialize indices
  initIndices(piBuilder, environment);

  // Initialize temporaries
  for (const Var& tmp : environment.getTemporaries()) {
    iassert(util::contains(temporaryPtrs, tmp.getName()));
    const Type& type = tmp.getType();

    if (type.isTensor()) {
      const ir::TensorType* tensorType = type.toTensor();
      unsigned order = tensorType->order();
      iassert(order <= 2) << "Higher-order tensors not supported";

      if (order == 1) {
        // Vectors are currently always dense
        IndexDomain vecDimension = tensorType->getDimensions()[0];
        Type blockType = tensorType->getBlockType();
        size_t blockSize = blockType.toTensor()->size();
        size_t componentSize = tensorType->componentType.bytes();
        *temporaryPtrs.at(tmp.getName()) =
            calloc(size(vecDimension) *blockSize, componentSize);
      }
      else if (order == 2) {
        iassert(environment.hasTensorIndex(tmp))
          << "No tensor index for: " << tmp;
        const pe::PathExpression& pexpr =
            environment.getTensorIndex(tmp).getPathExpression();
        iassert(util::contains(pathIndices, pexpr));
        Type blockType = tensorType->getBlockType();
        size_t blockSize = blockType.toTensor()->size();
        size_t componentSize = tensorType->componentType.bytes();
        size_t matSize = pathIndices.at(pexpr).numNeighbors() *
                         blockSize * componentSize;
        *temporaryPtrs.at(tmp.getName()) = malloc(matSize);
      }
    }
    else {
      unreachable << "don't know how to initialize temporary "
                  << util::quote(tmp);
    }
  }

  // Compile a harness void function without arguments that calls the simit
  // llvm function with pointers to the arguments.
  Function::FuncType func;
  initialized = true;
  vector<string> formals = getArgs();
  iassert(formals.size() == llvmFunc->getArgumentList().size());
  if (llvmFunc->getArgumentList().size() == 0) {
    llvm::Function *initFunc = getInitFunc();
    llvm::Function *deinitFunc = getDeinitFunc();
    auto init = (FuncPtrType)executionEngine->getPointerToFunction(initFunc);
    init();
    deinit = (FuncPtrType)executionEngine->getPointerToFunction(deinitFunc);
    func = (FuncPtrType)executionEngine->getPointerToFunction(llvmFunc);
  }
  else {
    llvm::SmallVector<llvm::Value*, 8> args;
    auto llvmArgIt = llvmFunc->getArgumentList().begin();
    for (const std::string& formal : formals) {
      iassert(util::contains(arguments, formal));

      llvm::Argument* llvmFormal = llvmArgIt++;
      Actual* actual = arguments.at(formal).get();
      ir::Type type = getArgType(formal);
      iassert(type.kind() == ir::Type::Set || type.kind() == ir::Type::Tensor);

      class InitActual : public ActualVisitor {
      public:
        llvm::Value* result;
        Type type;
        llvm::Argument* llvmFormal;
        llvm::Value* init(Actual* a, const Type& t, llvm::Argument* f) {
          this->type = t;
          this->llvmFormal = f;
          a->accept(this);
          return result;
        }

        void visit(SetActual* actual) {
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

          result = llvm::ConstantStruct::get(llvmSetType, setData);
        }

        void visit(TensorActual* actual) {
          const ir::TensorType* tensorType = type.toTensor();
          void* tensorData = actual->getData();
          result = (llvmFormal->getType()->isPointerTy())
                   ? llvmPtr(*tensorType, tensorData)
                   : llvmVal(*tensorType, tensorData);
        }
      };
      llvm::Value* llvmActual = InitActual().init(actual, type, llvmFormal);
      args.push_back(llvmActual);
    }

    // Create Init/deinit function harnesses
    auto init = createHarness(string(llvmFunc->getName())+".init", args);
    init();
    deinit = createHarness(string(llvmFunc->getName())+".deinit", args);

    // Compute function
    func = createHarness(llvmFunc->getName(), args);
    iassert(!llvm::verifyModule(*module))
        << "LLVM module does not pass verification";
  }
  return func;
}

void LLVMFunction::print(std::ostream &os) const {
  std::string fstr;
  llvm::raw_string_ostream rsos(fstr);
  module->print(rsos, nullptr);
  os << rsos.str();
}

void LLVMFunction::printMachine(std::ostream &os) const {
  // TODO: Make printMachine write to os, instead of stderr
  llvm::TargetMachine *target = engineBuilder->selectTarget();
  target->Options.PrintMachineCode = true;
  llvm::ExecutionEngine *printee(engineBuilder->create(target));
  printee->getPointerToFunction(llvmFunc);
  target->Options.PrintMachineCode = false;
}

void LLVMFunction::initIndices(pe::PathIndexBuilder& piBuilder,
                               const Environment& environment) {
  // Initialize indices
  for (const TensorIndex& tensorIndex : environment.getTensorIndices()) {
    pe::PathExpression pexpr = tensorIndex.getPathExpression();
    pe::PathIndex pidx = piBuilder.buildSegmented(pexpr, 0);
    pathIndices.insert({pexpr, pidx});

    pair<const uint32_t**,const uint32_t**> ptrPair = tensorIndexPtrs.at(pexpr);

    if (isa<pe::SegmentedPathIndex>(pidx)) {
      const pe::SegmentedPathIndex* spidx = to<pe::SegmentedPathIndex>(pidx);
      *ptrPair.first = spidx->getCoordData();
      *ptrPair.second = spidx->getSinkData();
    }
    else {
      not_supported_yet << "doesn't know how to initialize this pathindex type";
    }
  }
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
