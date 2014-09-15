#include "llvm_backend.h"

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

#include "llvm_codegen.h"
#include "ir.h"
#include "llvm_function.h"
#include "storage.h"
#include "scopedmap.h"
#include "macros.h"

using namespace std;
using namespace simit::ir;
using namespace simit::internal;

namespace {

const char *VAL_SUFFIX = "_val";
const char *PTR_SUFFIX = "_ptr";
const char *OFFSET_SUFFIX = "_ofs";

/// Creates an execution engine that takes ownership of the module.
llvm::ExecutionEngine *createExecutionEngine(llvm::Module *module) {
    llvm::EngineBuilder engineBuilder(module);
    // TODO: Intialization
    auto ee = engineBuilder.create();
    assert(ee && "Could not create ExecutionEngine");
    return ee;
}

llvm::Constant *getDouble(double val, llvm::LLVMContext &ctx = LLVM_CONTEXT){
  return llvm::ConstantFP::get(ctx, llvm::APFloat(val));
}

llvm::Instruction::BinaryOps toLLVMBinaryOp(IndexExpr::Operator op,
                                            simit::ComponentType type) {
  using namespace simit;
  assert(type == ComponentType::INT || type == ComponentType::FLOAT);
  switch (op) {
    case IndexExpr::ADD:
      switch (type) {
        case ComponentType::INT:
          return llvm::Instruction::Add;
        case ComponentType::FLOAT:
          return llvm::Instruction::FAdd;
        default:
          UNREACHABLE;
      }
    case IndexExpr::SUB:
      switch (type) {
        case ComponentType::INT:
          return llvm::Instruction::Sub;
        case ComponentType::FLOAT:
          return llvm::Instruction::FSub;
        default:
          UNREACHABLE;
      }
    case IndexExpr::MUL:
      switch (type) {
        case ComponentType::INT:
          return llvm::Instruction::Mul;
        case ComponentType::FLOAT:
          return llvm::Instruction::FMul;
        default:
          UNREACHABLE;
      }
    case IndexExpr::DIV:
      assert(type == ComponentType::FLOAT);
      return llvm::Instruction::FDiv;
    case IndexExpr::NEG: // fall-through
    default:
      UNREACHABLE;
  }
}

llvm::Instruction::BinaryOps toLLVMBinaryOp(IndexVar::Operator op,
                                            simit::ComponentType type) {
  switch (op) {
    case IndexVar::Operator::FREE:
      assert(false && "Free index variables do not have an operator");
    case IndexVar::Operator::SUM:
      return toLLVMBinaryOp(IndexExpr::Operator::ADD, type);
    case IndexVar::Operator::PRODUCT:
      return toLLVMBinaryOp(IndexExpr::Operator::MUL, type);
    default:
      UNREACHABLE;
  }
}

llvm::Value *createValueComputation(std::string name,
                                    simit::ComponentType ctype,
                                    IndexExpr::Operator op,
                                    const vector<llvm::Value*> operands,
                                    llvm::IRBuilder<> *builder) {
  switch (op) {
    case IndexExpr::Operator::NONE:
      assert(operands.size() == 1);
      return operands[0];
    case IndexExpr::Operator::NEG: {
      assert (operands.size() == 1);
      switch (ctype) {
        case simit::ComponentType::INT:
          return builder->CreateNeg(operands[0], name);
        case simit::ComponentType::FLOAT:
          return builder->CreateFNeg(operands[0], name);
        default:
          UNREACHABLE;
      }
      break;
    }
    case IndexExpr::Operator::ADD: // fall-through
    case IndexExpr::Operator::SUB: // fall-through
    case IndexExpr::Operator::MUL: // fall-through
    case IndexExpr::Operator::DIV: {
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

  simit::ComponentType ctype = llvmToSimitType(resultStorage->getType());

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

typedef std::map<const IndexVar *, llvm::Value *> IndexVarMap;

/// Emits code that offsets ptr to index into tensor using idxVars.  The
/// indexMap is used to map the idxVars to llvm::Value loop indices.
llvm::Value *emitOffset(llvm::Value *ptr, TensorType *type,
                        const std::vector<std::shared_ptr<IndexVar>> &idxVars,
                        IndexVarMap &indexMap, size_t currNest,
                        llvm::IRBuilder<> *builder) {
  // OPT: Offsets are currently recomputed for identical accesses to different
  //      tensors in the emitted code (e.g. `C(i,j) = A(i,j)`).
  if (type->getOrder() > 0) {
    llvm::Value *offset = NULL;
    if (idxVars.size() == 1) {
      offset = indexMap[idxVars[0].get()];
    }
    else {
      int stride = 1;
      for (size_t i=1; i<idxVars.size(); ++i) {
        const std::shared_ptr<IndexVar> &iv = idxVars[i];
        const IndexSet &is = iv->getIndexSet().getFactors()[currNest];
        stride *= is.getSize();
      }

      const std::shared_ptr<IndexVar> &iv_first = idxVars[0];
      offset = builder->CreateMul(indexMap[idxVars[0].get()],
                                  builder->getInt32(stride),
                                  iv_first->getName()+OFFSET_SUFFIX,
                                  false, true);
      for (size_t i=1; i<idxVars.size()-1; ++i) {
        const std::shared_ptr<IndexVar> &iv = idxVars[i];
        const IndexSet &is = iv->getIndexSet().getFactors()[currNest];
        stride = stride / is.getSize();
        auto *iv_ofs = builder->CreateMul(indexMap[idxVars[i].get()],
                                          builder->getInt32(stride),
                                          iv->getName()+OFFSET_SUFFIX,
                                          false, true);
        offset = builder->CreateAdd(offset, iv_ofs);
      }

      const std::shared_ptr<IndexVar> &iv_last = idxVars[idxVars.size()-1];
      offset = builder->CreateAdd(offset,
                                  indexMap[idxVars[idxVars.size()-1].get()],
                                  iv_last->getName()+OFFSET_SUFFIX,
                                  false, true);
    }

    std::string ptrName = string(ptr->getName()) + PTR_SUFFIX;

    // OPT: It might be faster to cast indices to i64 before GEP
    //      (sext i32 %i to i64). LLVM seems to do this.
    ptr = builder->CreateInBoundsGEP(ptr, offset, ptrName);
  }
  return ptr;
}

typedef std::pair<const IndexedTensor&, llvm::Value*> OperandPair;
typedef std::vector<OperandPair> OperandPairVec;

/// Emit code to compute an IndexExpr.
llvm::Value *emitIndexExpr(const IndexExpr *indexExpr,
                           const std::vector<std::shared_ptr<IndexVar>> &domain,
                           IndexExpr::Operator op,
                           const OperandPairVec &operands,
                           llvm::Value *resultStorage,
                           llvm::IRBuilder<> *builder,
                           IndexVarMap &indexMap,
                           size_t currNest=0) {
  // OPT: This code should allow loop orders to be configurable
  // OPT: LLVM seems to convert doubles to integers for the purpose of passing
  //      them around a reduction loop. Explore this. E.g.:
  //        %conv = sitofp i32 %c_sum to double
  //        %c_sum_nxt = fadd double %c_val, %conv
  //        %conv3 = fptosi double %c_sum_nxt to i32) nad

  size_t currIdxVar = indexMap.size();

  assert(domain.size() > 0);
  assert(operands.size() > 0);
  assert(currIdxVar < domain.size());

  std::string resultName = resultStorage->getName();
  TensorType *resultType = tensorTypePtr(indexExpr->getType());
  simit::ComponentType resultCType = resultType->getComponentType();
  assert(resultCType == llvmToSimitType(resultStorage->getType()));

  size_t numNests = (domain.size() > 0)
      ? domain[0]->getIndexSet().getFactors().size() : 0;
  for (std::shared_ptr<IndexVar> idxVar : domain) {
    assert(idxVar->getIndexSet().getFactors().size() == numNests);
  }

  const std::shared_ptr<IndexVar> &iv = domain[currIdxVar];
  std::string idxName = iv->getName();
  const IndexSet &is = iv->getIndexSet().getFactors()[currNest];

  llvm::Function *f = builder->GetInsertBlock()->getParent();

  // Loop Header
  llvm::BasicBlock *entryBlock = builder->GetInsertBlock();

  llvm::BasicBlock *loopBodyStart =
  llvm::BasicBlock::Create(LLVM_CONTEXT, idxName+"_loop_body", f);
  builder->CreateBr(loopBodyStart);
  builder->SetInsertPoint(loopBodyStart);

  llvm::PHINode *idx = builder->CreatePHI(LLVM_INT32, 2, idxName);
  indexMap[iv.get()] = idx;
  idx->addIncoming(builder->getInt32(0), entryBlock);

  llvm::PHINode *reductionVal = NULL;
  llvm::Value *reductionValNext = NULL;
  if (iv->isReductionVariable()) {
    // Emit value to hold the result of the reduction
    std::string ropStr = IndexVar::operatorString(iv->getOperator());
    std::string name = resultName + "_" + ropStr;
    llvm::Type *type = toLLVMType(resultCType);
    reductionVal = builder->CreatePHI(type, 1, name);
    reductionVal->addIncoming(getDouble(0.0), entryBlock);
  }

  // Loop Body
  llvm::Value *resultPtr = NULL;
  if (currIdxVar < domain.size()-1) {
    emitIndexExpr(indexExpr, domain, op, operands, resultStorage,
                     builder, indexMap, currNest);
  }
  else {
    std::vector<llvm::Value *> operandVals;
    for (auto &operandPair : operands) {
      const IndexedTensor &operand = operandPair.first;
      llvm::Value *llvmOperand = operandPair.second;

      std::string operandName = llvmOperand->getName();
      std::string operandValName = operandName + VAL_SUFFIX;
      std::string operandPtrName = operandName + PTR_SUFFIX;

      assert(operand.getTensor()->getType()->isTensor());
      TensorType *operandType = tensorTypePtr(operand.getTensor()->getType());

      llvm::Value *operandPtr = emitOffset(llvmOperand, operandType,
                                           operand.getIndexVariables(),
                                           indexMap, currNest, builder);
      llvm::Value *operandVal = builder->CreateAlignedLoad(operandPtr, 8,
                                                           operandValName);
      operandVals.push_back(operandVal);
    }

    std::string resultValName = resultName + VAL_SUFFIX;
    std::string resultPtrName = resultName + PTR_SUFFIX;

    llvm::Value *resultVal = createValueComputation(resultValName,
                                                    resultCType, op,
                                                    operandVals, builder);

    assert(indexExpr->getType()->isTensor());
    TensorType *indexExprType = tensorTypePtr(indexExpr->getType());

    resultPtr = emitOffset(resultStorage, indexExprType,
                           indexExpr->getIndexVariables(),
                           indexMap, currNest, builder);

    if (!iv->isReductionVariable()) {
      builder->CreateAlignedStore(resultVal, resultPtr, 8);
    }
    else {
      auto binOp = toLLVMBinaryOp(iv->getOperator(), resultCType);
      std::string ropStr = IndexVar::operatorString(iv->getOperator());
      std::string name = resultName + "_" + ropStr + "_nxt";
      reductionValNext = builder->CreateBinOp(binOp, reductionVal, resultVal,
                                              name);
    }
  }

  // Loop Footer
  llvm::BasicBlock *loopBodyEnd = builder->GetInsertBlock();
  if (iv->isReductionVariable()) {
    assert(reductionVal && reductionValNext);
    reductionVal->addIncoming(reductionValNext, loopBodyEnd);
  }

  llvm::Value* i_nxt = builder->CreateAdd(idx, builder->getInt32(1),
                                          idxName+"_nxt", false, true);
  idx->addIncoming(i_nxt, loopBodyEnd);
  llvm::Value *numIter = builder->getInt32(is.getSize());
  llvm::Value *exitCond = builder->CreateICmpSLT(i_nxt, numIter,idxName+"_cmp");
  llvm::BasicBlock *loopEnd = llvm::BasicBlock::Create(LLVM_CONTEXT,
                                                       idxName+"_loop_end", f);
  builder->CreateCondBr(exitCond, loopBodyStart, loopEnd);
  builder->SetInsertPoint(loopEnd);

  if (iv->isReductionVariable()) {
    assert(reductionValNext && resultPtr);
    builder->CreateAlignedStore(reductionValNext, resultPtr, 8);
  }

  return resultStorage;
}

}  // unnamed namespace


namespace simit {
namespace internal {


// class LLVMBackend
bool LLVMBackend::llvmInitialized = false;

LLVMBackend::LLVMBackend() {
  if (!llvmInitialized) {
    llvm::InitializeNativeTarget();
    llvmInitialized = true;
  }

  module = new llvm::Module("Simit JIT", LLVM_CONTEXT);
  llvm::ExecutionEngine *ee = createExecutionEngine(module);
  executionEngine = std::shared_ptr<llvm::ExecutionEngine>(ee);
  builder = new llvm::IRBuilder<>(LLVM_CONTEXT);

  symtable = new ScopedMap<std::string, llvm::Value*>();
}

LLVMBackend::~LLVMBackend() {
  delete symtable;
  delete builder;
}

simit::Function *LLVMBackend::compile(simit::ir::Function *function) {
  TemporaryAllocator talloc;
  std::map<IRNode*, void*> temps = talloc.allocateTemporaries(function);

  llvm::Function *f = codegen(function, temps);
  if (f == NULL) return NULL;

  return new LLVMFunction(*function, f, executionEngine,
                          talloc.getTemporaries());
}

llvm::Function *LLVMBackend::codegen(simit::ir::Function *function,
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

void LLVMBackend::handle(simit::ir::Function *function) {
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

void LLVMBackend::handle(IndexExpr *t) {
  llvm::Value *result = NULL;

  auto domain = t->getDomain();

  IndexExpr::Operator op = t->getOperator();

  OperandPairVec operands;
  std::vector<llvm::Value *> llvmOperands;  // TODO: Remove this
  for (auto &operand : t->getOperands()) {
    assert(symtable->contains(operand.getTensor()->getName()));
    llvm::Value *llvmOperand = symtable->get(operand.getTensor()->getName());
    operands.push_back(OperandPair(operand, llvmOperand));

    llvmOperands.push_back(symtable->get(operand.getTensor()->getName()));
  }

  llvm::Value *resultStorage = storageLocations[t];
  assert(resultStorage);

  if (domain.size() == 0) {
    result = createScalarComputation(resultStorage, op, llvmOperands, builder);
  }
  else {
    IndexVarMap indexMap;
    result = emitIndexExpr(t, domain, op, operands, resultStorage, builder,
                           indexMap);
  }

  assert(result != NULL);
  symtable->insert(t->getName(), result);
}

}}  // namespace simit::internal
