#include "llvm_codegen.h"

#include <cstdint>
#include <iostream>
#include <stack>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/JIT.h"

#include "ir.h"
#include "storage.h"
#include "symboltable.h"
#include "macros.h"

using namespace std;

#define LLVM_CONTEXT   llvm::getGlobalContext()

#define LLVM_VOID      llvm::Type::getVoidTy(LLVM_CONTEXT)
#define LLVM_INT       llvm::Type::getInt32Ty(LLVM_CONTEXT)
#define LLVM_INTPTR    llvm::Type::getInt32PtrTy(LLVM_CONTEXT)
#define LLVM_DOUBLE    llvm::Type::getDoubleTy(LLVM_CONTEXT)
#define LLVM_DOUBLEPTR llvm::Type::getDoublePtrTy(LLVM_CONTEXT)

#define LLVM_INT8      llvm::Type::getInt8Ty(LLVM_CONTEXT)
#define LLVM_INT32     llvm::Type::getInt32Ty(LLVM_CONTEXT)
#define LLVM_INT64     llvm::Type::getInt64Ty(LLVM_CONTEXT)

namespace {
using namespace simit::internal;

#define VAL_SUFFIX "_val"
#define PTR_SUFFIX "_ptr"

/// Creates an execution engine that takes ownership of the module.
llvm::ExecutionEngine *createExecutionEngine(llvm::Module *module) {
    llvm::EngineBuilder engineBuilder(module);
    // TODO: Intialization
    auto ee = engineBuilder.create();
    assert(ee && "Could not create ExecutionEngine");
    return ee;
}

llvm::Type *toLLVMType(const simit::internal::TensorType *type) {
  switch (type->getComponentType()) {
    case simit::Type::INT:
      return LLVM_INTPTR;
    case simit::Type::FLOAT:
      return LLVM_DOUBLEPTR;
    case simit::Type::ELEMENT:
      NOT_SUPPORTED_YET;
    default:
      UNREACHABLE;
  }
}

llvm::Constant *toLLVMPtr(const std::shared_ptr<Literal> &literal) {
  llvm::Constant *c = (sizeof(void*) == 4)
      ? llvm::ConstantInt::get(llvm::Type::getInt32Ty(LLVM_CONTEXT),
                               (int)(intptr_t)literal->getData())
      : llvm::ConstantInt::get(llvm::Type::getInt64Ty(LLVM_CONTEXT),
                               (intptr_t)literal->getData());

  // TODO: Do we have to free ctype?
  llvm::Type *ctype = toLLVMType(literal->getType());
  llvm::Constant *cptr = llvm::ConstantExpr::getIntToPtr(c, ctype);
  return cptr;
}

simit::Type llvmToSimitType(const llvm::Type *type) {
  if (type->isPointerTy()) {
    type = type->getPointerElementType();
  }

  if (type->isDoubleTy()) {
    return simit::FLOAT;
  }
  else if (type->isIntegerTy()) {
    return simit::INT;
  }
  else {
    UNREACHABLE;
  }
}

template <class AT, class RT>
llvm::FunctionType *createFunctionType(const vector<shared_ptr<AT>> &arguments,
                                       const vector<shared_ptr<RT>> &results) {
  vector<llvm::Type*> args;
  for (auto &arg : arguments) {
    args.push_back(toLLVMType(arg->getType()));
  }
  for (auto &result : results) {
    args.push_back(toLLVMType(result->getType()));
  }
  return llvm::FunctionType::get(LLVM_VOID, args, false);
}

template <class AT, class RT>
llvm::Function *createPrototype(const string &name,
                                const vector<shared_ptr<AT>> &arguments,
                                const vector<shared_ptr<RT>> &results,
                                llvm::GlobalValue::LinkageTypes linkage,
                                llvm::Module *module) {
  llvm::FunctionType *ft = createFunctionType(arguments, results);
  llvm::Function *f = llvm::Function::Create(ft, linkage, name, module);
  f->setDoesNotThrow();
  for (size_t i=0; i<f->getArgumentList().size(); ++i) {
    f->setDoesNotCapture(i+1);
  }

  auto ai = f->arg_begin();
  for (auto &arg : arguments) {
    ai->setName(arg->getName());
    ++ai;
  }
  for (auto &result : results) {
    ai->setName(result->getName());
    ++ai;
  }
  assert(ai == f->arg_end());
  return f;
}

llvm::Instruction::BinaryOps toLLVMBinaryOp(IndexExpr::Operator op,
                                            simit::Type type) {
  using namespace simit;
  assert(type == Type::INT || type == Type::FLOAT);
  switch (op) {
    case IndexExpr::ADD:
      switch (type) {
        case Type::INT:
          return llvm::Instruction::Add;
        case Type::FLOAT:
          return llvm::Instruction::FAdd;
        default:
          UNREACHABLE;
      }
    case IndexExpr::SUB:
      switch (type) {
        case Type::INT:
          return llvm::Instruction::Sub;
        case Type::FLOAT:
          return llvm::Instruction::FSub;
        default:
          UNREACHABLE;
      }
    case IndexExpr::MUL:
      switch (type) {
        case Type::INT:
          return llvm::Instruction::Mul;
        case Type::FLOAT:
          return llvm::Instruction::FMul;
        default:
          UNREACHABLE;
      }
    case IndexExpr::DIV:
      assert(type == Type::FLOAT);
      return llvm::Instruction::FDiv;
    case IndexExpr::NEG: // fallthrough
    default:
      UNREACHABLE;
  }
}

llvm::Value *createValueComputation(std::string name, simit::Type ctype,
                                    IndexExpr::Operator op,
                                    const vector<llvm::Value*> operands,
                                    llvm::IRBuilder<> *builder) {
  switch (op) {
    case IndexExpr::NEG: {
      assert (operands.size() == 1);
      switch (ctype) {
        case simit::Type::INT:
          return builder->CreateNeg(operands[0], name);
        case simit::Type::FLOAT:
          return builder->CreateFNeg(operands[0], name);
        default:
          UNREACHABLE;
      }
      break;
    }
    case IndexExpr::ADD: // fallthrough
    case IndexExpr::SUB: // fallthrough
    case IndexExpr::MUL: // fallthrough
    case IndexExpr::DIV: {
      assert (operands.size() == 2);
      return builder->CreateBinOp(toLLVMBinaryOp(op, ctype),
                                  operands[0], operands[1], name);
    }
    default:
      UNREACHABLE;
  }
}

llvm::Value *createScalarComputation(llvm::Value *resultStorage,
                                     IndexExpr::Operator op,
                                     const vector<llvm::Value*> operands,
                                     llvm::IRBuilder<> *builder) {
  assert(operands.size() > 0);

  simit::Type ctype = llvmToSimitType(resultStorage->getType());

  std::vector<llvm::Value *> operandVals;
  for (llvm::Value *operandPtr : operands) {
    std::string operandName = operandPtr->getName();
    llvm::Value *operandVal =
        builder->CreateAlignedLoad(operandPtr, 8, operandName+VAL_SUFFIX);
    operandVals.push_back(operandVal);
  }

  std::string resultValName = std::string(resultStorage->getName())+VAL_SUFFIX;
  llvm::Value *resultVal = createValueComputation(resultValName, ctype, op,
                                                  operandVals, builder);
  builder->CreateAlignedStore(resultVal, resultStorage, 8);
  return resultVal;
}

llvm::Value *createIndexComputation(const IndexExpr::IndexVarPtrVector &domain,
                                    IndexExpr::Operator op,
                                    const vector<llvm::Value*> operands,
                                    llvm::Value *resultStorage,
                                    llvm::IRBuilder<> *builder,
                                    int currNest=0, int currIdxVar=0) {
  assert(domain.size() > 0);
  assert(operands.size() > 0);

  simit::Type ctype = llvmToSimitType(resultStorage->getType());
  size_t numNests = (domain.size() > 0)
      ? domain[0]->getIndexSet().getFactors().size() : 0;

  for (IndexExpr::IndexVarPtr idxVar : domain) {
    assert(idxVar->getIndexSet().getFactors().size() == numNests);
  }

  const IndexSet &is = domain[currIdxVar]->getIndexSet().getFactors()[currNest];

  // We generate loops where the outer loops iterate over blocks along each
  // dimension and the inner loops over each block.
  // OPT: This code should allow loop orders to be configurable
  llvm::Function *f = builder->GetInsertBlock()->getParent();

  // Loop Header
  llvm::BasicBlock *entryBlock = builder->GetInsertBlock();

  llvm::BasicBlock *loopBodyStart =
  llvm::BasicBlock::Create(LLVM_CONTEXT, "loop_body", f);
  builder->CreateBr(loopBodyStart);
  builder->SetInsertPoint(loopBodyStart);

  llvm::PHINode *i = builder->CreatePHI(LLVM_INT32, 2, "i");
  i->addIncoming(builder->getInt32(0), entryBlock);

  // Loop Body
  std::vector<llvm::Value *> operandVals;
  for (llvm::Value *operand : operands) {
    std::string operandName = operand->getName();
    llvm::Value *operandPtr =
        builder->CreateInBoundsGEP(operand, i, operandName+PTR_SUFFIX);
    llvm::Value *operandVal =
        builder->CreateAlignedLoad(operandPtr, 8, operandName+VAL_SUFFIX);
    operandVals.push_back(operandVal);
  }

  std::string resultValName = std::string(resultStorage->getName())+VAL_SUFFIX;
  llvm::Value *resultVal = createValueComputation(resultValName, ctype, op,
                                                  operandVals, builder);
  llvm::Value *resultPtr =
      builder->CreateInBoundsGEP(resultStorage, i,
                                 resultStorage->getName() + PTR_SUFFIX);
  builder->CreateAlignedStore(resultVal, resultPtr, 8);

  // Loop Footer
  llvm::Value* i_nxt = builder->CreateAdd(i, builder->getInt32(1), "i_nxt");
  llvm::Value *numIter = builder->getInt32(is.getSize());
  llvm::Value *exitCond = builder->CreateICmpEQ(i_nxt, numIter, "exitcond");
  llvm::BasicBlock *loopBodyEnd = builder->GetInsertBlock();
  llvm::BasicBlock *loopEnd = llvm::BasicBlock::Create(LLVM_CONTEXT,
                                                       "loop_end", f);
  builder->CreateCondBr(exitCond, loopEnd, loopBodyStart);
  builder->SetInsertPoint(loopEnd);

  // Add phi backedges for loop
  i->addIncoming(i_nxt, loopBodyEnd);

  return resultStorage;
}


/// A Simit function that has been compiled with LLVM.
class LLVMCompiledFunction : public CompiledFunction {
 public:
  LLVMCompiledFunction(llvm::Function *f,
                       const std::shared_ptr<llvm::ExecutionEngine> &fee,
                       const std::vector<std::shared_ptr<Storage>> &storage)
      : f(f), fee(fee), storage(storage), module("Harness Module", LLVM_CONTEXT) {
    fee->addModule(&module);
  }

  ~LLVMCompiledFunction() { fee->removeModule(&module); }

  void bind(const std::vector<std::shared_ptr<Literal>> &arguments,
            const std::vector<std::shared_ptr<Literal>> &results) {
    void *fptr = fee->getPointerToFunction(f);
    if (arguments.size() == 0 and results.size() == 0) {
      setRunPtr((RunPtrType)fptr);
    }
    else {
      std::string name = string(f->getName()) + "_harness";
      std::vector<std::shared_ptr<TensorNode>> noArgs;
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
      llvm::CallInst *call = llvm::CallInst::Create(f, args, "", entry);
      call->setCallingConv(f->getCallingConv());
      call->setTailCall();
      llvm::ReturnInst::Create(f->getContext(), entry);
      setRunPtr((RunPtrType)fee->getPointerToFunction(harness));
    }
  }

  void print(std::ostream &os) const {
    std::string fstr;
    llvm::raw_string_ostream rsos(fstr);
    f->print(rsos);
  }

 private:
  llvm::Function *f;
  std::shared_ptr<llvm::ExecutionEngine> fee;
  std::vector<std::shared_ptr<Storage>> storage;
  llvm::Module module;
};
}  // unnamed namespace


namespace simit {
namespace internal {


// class LLVMCodeGen
bool LLVMCodeGen::llvmInitialized = false;

LLVMCodeGen::LLVMCodeGen() {
  if (!llvmInitialized) {
    llvm::InitializeNativeTarget();
    llvmInitialized = true;
  }

  module = new llvm::Module("Simit JIT", LLVM_CONTEXT);
  llvm::ExecutionEngine *ee = createExecutionEngine(module);
  executionEngine = std::shared_ptr<llvm::ExecutionEngine>(ee);
  builder = new llvm::IRBuilder<>(LLVM_CONTEXT);

  symtable = new SymbolTable<llvm::Value*>();
}

LLVMCodeGen::~LLVMCodeGen() {
  delete symtable;
}

CompiledFunction *LLVMCodeGen::compile(Function *function) {
  TemporaryAllocator talloc;
  std::map<IRNode*, void*> temps = talloc.allocateTemporaries(function);

  llvm::Function *f = codegen(function, temps);
  if (f == NULL) return NULL;

  return new LLVMCompiledFunction(f, executionEngine, talloc.getTemporaries());
}

llvm::Function *LLVMCodeGen::codegen(Function *function,
                                     const std::map<IRNode*, void*> &temps) {
  // TODO: Add temporaries as pointer values to storageLocations
  visit(function);
  storageLocations.clear();

  if (isAborted()) {
    return NULL;
  }
  builder->CreateRetVoid();

  llvm::Value *value = resultStack.top();
  resultStack.pop();
  assert(llvm::isa<llvm::Function>(value));
  llvm::Function *f = llvm::cast<llvm::Function>(value);
  verifyFunction(*f);
  return f;
}

void LLVMCodeGen::handle(Function *function) {
  llvm::Function *f = createPrototype(function->getName(),
                                      function->getArguments(),
                                      function->getResults(),
                                      llvm::Function::ExternalLinkage,
                                      module);
  auto entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", f);
  builder->SetInsertPoint(entry);
  for (auto &arg : f->getArgumentList()) {
    symtable->insert(arg.getName(), &arg);
  }

  for (auto &result : function->getResults()) {
    IRNode *resultValue = result->getValue().get();
    storageLocations[resultValue] = symtable->get(result->getName());
  }
  
  if (f == NULL) {  // TODO: Remove check
    abort();
    return;
  }
  resultStack.push(f);
}

void LLVMCodeGen::handle(IndexExpr *t) {
  llvm::Value *result = NULL;

  const std::vector<IndexExpr::IndexVarPtr> &domain = t->getDomain();
  IndexExpr::Operator op = t->getOperator();

  std::vector<llvm::Value *> llvmOperands;
  for (auto &operand : t->getOperands()) {
    assert(symtable->contains(operand.getTensor()->getName()));
    llvmOperands.push_back(symtable->get(operand.getTensor()->getName()));
  }

  llvm::Value *resultStorage = storageLocations[t];
  assert(resultStorage);

  if (domain.size() == 0) {
    result = createScalarComputation(resultStorage, op, llvmOperands, builder);
  }
  else {
    result = createIndexComputation(domain, op, llvmOperands,
                                    resultStorage, builder);
  }

  assert(result != NULL);
  symtable->insert(t->getName(), result);
}

}}  // namespace simit::internal
