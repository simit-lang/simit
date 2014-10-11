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
#include "ir_printer.h"
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
                                            const ScalarType *type) {
  using namespace simit;
  switch (op) {
    case IndexExpr::ADD:
      switch (type->kind) {
        case ScalarType::Int:
          return llvm::Instruction::Add;
        case ScalarType::Float:
          return llvm::Instruction::FAdd;
        default:
          UNREACHABLE;
      }
    case IndexExpr::SUB:
      switch (type->kind) {
        case ScalarType::Int:
          return llvm::Instruction::Sub;
        case ScalarType::Float:
          return llvm::Instruction::FSub;
        default:
          UNREACHABLE;
      }
    case IndexExpr::MUL:
      switch (type->kind) {
        case ScalarType::Int:
          return llvm::Instruction::Mul;
        case ScalarType::Float:
          return llvm::Instruction::FMul;
        default:
          UNREACHABLE;
      }
    case IndexExpr::DIV:
      assert(type->isFloat());
      return llvm::Instruction::FDiv;
    case IndexExpr::NEG: // fall-through
    default:
      UNREACHABLE;
  }
}

llvm::Instruction::BinaryOps toLLVMBinaryOp(const IndexVar &indexVar,
                                            const ScalarType *type) {
  assert(indexVar.isReductionVar() &&
         "Free index variables do not have an operator");

  switch (indexVar.getOperator().getKind()) {
    case ReductionOperator::Sum:
      return toLLVMBinaryOp(IndexExpr::Operator::ADD, type);
    default:
      UNREACHABLE;
  }
}

llvm::Value *createValueComputation(std::string name,
                                    const ScalarType *ctype,
                                    IndexExpr::Operator op,
                                    const vector<llvm::Value*> operands,
                                    llvm::IRBuilder<> *builder) {
  switch (op) {
    case IndexExpr::Operator::NONE:
      assert(operands.size() == 1);
      return operands[0];
    case IndexExpr::Operator::NEG: {
      assert (operands.size() == 1);
      switch (ctype->kind) {
        case ScalarType::Int:
          return builder->CreateNeg(operands[0], name);
        case ScalarType::Float:
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

llvm::Value *emitScalarComputation(IndexExpr *t,
                                   IndexExpr::Operator op,
                                   const vector<llvm::Value*> operands,
                                   llvm::IRBuilder<> *builder) {
  assert(operands.size() > 0);
  assert(t->getType().isTensor());

  const ScalarType *ctype = t->getType().toTensor()->componentType.toScalar();

  std::vector<llvm::Value *> operandVals;
  for (llvm::Value *operandPtr : operands) {
    std::string operandName = operandPtr->getName();
    llvm::Value *operandVal =
        builder->CreateAlignedLoad(operandPtr, 8, operandName+VAL_SUFFIX);
    operandVals.push_back(operandVal);
  }

  std::string resultValName = std::string(t->getName())+VAL_SUFFIX;
  llvm::Value *resultVal = createValueComputation(resultValName, ctype, op,
                                                  operandVals, builder);
  return resultVal;
}

typedef std::map<const IndexVar *, llvm::Value *> IndexVarMap;

/// Emits code that offsets ptr to index into tensor using idxVars.  The
/// indexMap is used to map the idxVars to llvm::Value loop indices.
/// \todo OPT: Offsets are currently recomputed for identical accesses to
/// different tensors in the emitted code (e.g. `C(i,j) = A(i,j)`).
llvm::Value *emitOffset(llvm::Value *ptr, const Type &type,
                        const std::vector<std::shared_ptr<IndexVar>> &idxVars,
                        IndexVarMap &indexMap, size_t currNest,
                        llvm::IRBuilder<> *builder) {
  assert(type.isTensor());
  const TensorType *tensorType = type.toTensor();

  if (tensorType->order() > 0) {
    llvm::Value *offset = NULL;
    if (idxVars.size() == 1) {
      offset = indexMap[idxVars[0].get()];
    }
    else {
      int stride = 1;
      for (size_t i=1; i<idxVars.size(); ++i) {
        const std::shared_ptr<IndexVar> &iv = idxVars[i];
        const IndexSet &is = iv->getDomain().getFactors()[currNest];
        assert(is.getKind() == IndexSet::Range &&
               "Only range offsets currently supported");
        stride *= is.getSize();
      }

      const std::shared_ptr<IndexVar> &iv_first = idxVars[0];
      offset = builder->CreateMul(indexMap[idxVars[0].get()],
                                  builder->getInt32(stride),
                                  iv_first->getName()+OFFSET_SUFFIX,
                                  false, true);
      for (size_t i=1; i<idxVars.size()-1; ++i) {
        const std::shared_ptr<IndexVar> &iv = idxVars[i];
        const IndexSet &is = iv->getDomain().getFactors()[currNest];

        assert(is.getKind() == IndexSet::Range &&
               "Only range offsets currently supported");
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
// OPT: This code should allow loop orders to be configurable
// OPT: LLVM seems to convert doubles to integers for the purpose of passing
//      them around a reduction loop. Explore this. E.g.:
//        %conv = sitofp i32 %c_sum to double
//        %c_sum_nxt = fadd double %c_val, %conv
//        %conv3 = fptosi double %c_sum_nxt to i32) nad
llvm::Value *emitIndexExpr(const IndexExpr *indexExpr,
                           const std::vector<std::shared_ptr<IndexVar>> &domain,
                           IndexExpr::Operator op,
                           const OperandPairVec &operands,
                           llvm::Value *resultStorage,
                           const ScopedMap<std::string,llvm::Value*> &symtable,
                           llvm::IRBuilder<> *builder,
                           IndexVarMap &indexMap,
                           size_t currNest=0) {
  size_t currIdxVar = indexMap.size();

  assert(domain.size() > 0);
  assert(operands.size() > 0);
  assert(currIdxVar < domain.size());

  std::string resultName = resultStorage->getName();
  const TensorType *resultType = indexExpr->getType().toTensor();

  assert(resultType->componentType == simitType(resultStorage->getType()));
  const ScalarType *resultCType = resultType->componentType.toScalar();

  size_t numNests = (domain.size() > 0)
      ? domain[0]->getDomain().getFactors().size() : 0;
  for (std::shared_ptr<IndexVar> idxVar : domain) {
    assert(idxVar->getDomain().getFactors().size() == numNests);
  }

  const std::shared_ptr<IndexVar> &iv = domain[currIdxVar];
  std::string idxName = iv->getName();

  // Compute dimension size
  const IndexDomain &dimension = iv->getDomain();
  llvm::Value *numIter = NULL;
  for (const IndexSet &is : dimension.getFactors()) {
    llvm::Value *isSize = NULL;
    switch (is.getKind()) {
      case IndexSet::Range:
        isSize = builder->getInt32(is.getSize());
        break;
      case IndexSet::Set:
        assert(symtable.contains(is.getSetName()));
        isSize = symtable.get(is.getSetName());
        break;
      case IndexSet::Dynamic:
        NOT_SUPPORTED_YET;
        break;
    }
    assert(isSize);
    if (numIter == NULL) {
      numIter = isSize;
    }
    else {
      numIter = builder->CreateMul(numIter, isSize, idxName+"_num");
    }
  }
  assert(numIter);

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
  if (iv->isReductionVar()) {
    // Emit value to hold the result of the reduction
    std::string ropStr = iv->getOperator().getName();
    std::string name = resultName + "_" + ropStr;
    llvm::Type *type = llvmType(resultCType);
    reductionVal = builder->CreatePHI(type, 1, name);
    reductionVal->addIncoming(getDouble(0.0), entryBlock);
  }

  // Loop Body
  llvm::Value *resultPtr = NULL;
  if (currIdxVar < domain.size()-1) {
    emitIndexExpr(indexExpr, domain, op, operands, resultStorage,
                  symtable, builder, indexMap, currNest);
  }
  else {
    std::vector<llvm::Value *> operandVals;
    for (auto &operandPair : operands) {
      const IndexedTensor &operand = operandPair.first;
      llvm::Value *llvmOperand = operandPair.second;

      std::string operandName = llvmOperand->getName();
      std::string operandValName = operandName + VAL_SUFFIX;
      std::string operandPtrName = operandName + PTR_SUFFIX;

      llvm::Value *operandPtr = emitOffset(llvmOperand,
                                           operand.getTensor()->getType(),
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

    resultPtr = emitOffset(resultStorage, indexExpr->getType(),
                           indexExpr->getIndexVariables(),
                           indexMap, currNest, builder);

    if (!iv->isReductionVar()) {
      builder->CreateAlignedStore(resultVal, resultPtr, 8);
    }
    else {
      auto binOp = toLLVMBinaryOp(*iv, resultCType);
      std::string ropStr = iv->getOperator().getName();
      std::string name = resultName + "_" + ropStr + "_nxt";
      reductionValNext = builder->CreateBinOp(binOp, reductionVal, resultVal,
                                              name);
    }
  }

  // Loop Footer
  llvm::BasicBlock *loopBodyEnd = builder->GetInsertBlock();
  if (iv->isReductionVar()) {
    assert(reductionVal && reductionValNext);
    reductionVal->addIncoming(reductionValNext, loopBodyEnd);
  }

  llvm::Value* i_nxt = builder->CreateAdd(idx, builder->getInt32(1),
                                          idxName+"_nxt", false, true);
  idx->addIncoming(i_nxt, loopBodyEnd);

  llvm::Value *exitCond = builder->CreateICmpSLT(i_nxt, numIter,idxName+"_cmp");
  llvm::BasicBlock *loopEnd = llvm::BasicBlock::Create(LLVM_CONTEXT,
                                                       idxName+"_loop_end", f);
  builder->CreateCondBr(exitCond, loopBodyStart, loopEnd);
  builder->SetInsertPoint(loopEnd);

  if (iv->isReductionVar()) {
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
  assert(!verifyFunction(*f));
  return f;
}

namespace {
class StorageLocationMapper : public IRBackwardVisitor {
public:
  StorageLocationMapper(map<ir::Expression*,llvm::Value*> *storageLocations,
                        ScopedMap<std::string,llvm::Value*> *symtable)
      : symtable(symtable), storageLocations(*storageLocations) {}

  void mapFunc(ir::Function *f) {
    visit(f);
  }

  void handle(ir::Result *result) {
    Expression *resultValue = result->getValues()[0].get();

    std::string name = resultValue->getName();

    // FIXME: workaround for bad handling of field reads
    if (name == "") {
      auto field = dynamic_cast<FieldWrite*>(resultValue);
      name = field->getTarget()->getName() + "." + field->getFieldName();
    }

    storageLocations[resultValue] = symtable->get(name);
  }

  void handle(ir::FieldWrite *fieldWrite) {
    Expression *writtenValue = fieldWrite->getValue().get();
    storageLocations[writtenValue] = storageLocations[fieldWrite];
  }

private:
  ScopedMap<std::string,llvm::Value*> *symtable;
  std::map<ir::Expression*,llvm::Value*> &storageLocations;
};
}

void LLVMBackend::handle(simit::ir::Function *function) {
  llvm::Function *f = createFunction(function->getName(),
                                     function->getArguments(),
                                     function->getResults(),
                                     llvm::Function::ExternalLinkage,
                                     module);
  auto entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", f);
  builder->SetInsertPoint(entry);
  for (auto &arg : f->getArgumentList()) {
    symtable->insert(arg.getName(), &arg);
  }

  StorageLocationMapper(&storageLocations,symtable).mapFunc(function);

  assert(f != NULL);
  resultStack.push(f);
}

void LLVMBackend::handle(IndexExpr *t) {
  llvm::Value *result = NULL;

  auto domain = t->getDomain();

  IndexExpr::Operator op = t->getOperator();

  OperandPairVec operands;
  std::vector<llvm::Value *> llvmOperands;  // TODO: Remove this
  for (auto &operand : t->getOperands()) {
    std::string name = operand.getTensor()->getName();

    // FIXME: workaround for bad handling of field reads
    if (name == "") {
      auto field = dynamic_cast<FieldRead*>(operand.getTensor().get());
      name = field->getTarget()->getName() + "." + field->getFieldName();
    }

    assert(symtable->contains(name));
    llvm::Value *llvmOperand = symtable->get(name);
    operands.push_back(OperandPair(operand, llvmOperand));

    llvmOperands.push_back(symtable->get(name));
  }

  if (domain.size() == 0) {
    result = emitScalarComputation(t, op, llvmOperands, builder);
    if (storageLocations.find(t) != storageLocations.end()) {
      builder->CreateAlignedStore(result, storageLocations[t], 8);
    }
  }
  else {
    assert(storageLocations.find(t) != storageLocations.end());
    IndexVarMap indexMap;
    result = emitIndexExpr(t, domain, op, operands, storageLocations[t],
                           *symtable, builder, indexMap);
  }

  assert(result != NULL);
  symtable->insert(t->getName(), result);
}

}}  // namespace simit::internal
