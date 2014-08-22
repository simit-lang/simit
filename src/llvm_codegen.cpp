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

#include "macros.h"
#include "ir.h"
#include "symboltable.h"

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
      UNREACHABLE_DEFAULT;
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
          UNREACHABLE_DEFAULT;
      }
    case IndexExpr::SUB:
      switch (type) {
        case Type::INT:
          return llvm::Instruction::Sub;
        case Type::FLOAT:
          return llvm::Instruction::FSub;
        default:
          UNREACHABLE_DEFAULT;
      }
    case IndexExpr::MUL:
      switch (type) {
        case Type::INT:
          return llvm::Instruction::Mul;
        case Type::FLOAT:
          return llvm::Instruction::FMul;
        default:
          UNREACHABLE_DEFAULT;
      }
    case IndexExpr::DIV:
      assert(type == Type::FLOAT);
      return llvm::Instruction::FDiv;
    case IndexExpr::NEG: // fallthrough
    default:
      UNREACHABLE_DEFAULT;
  }
}


/// A Simit function that has been compiled with LLVM.
class LLVMCompiledFunction : public CompiledFunction {
 public:
  LLVMCompiledFunction(llvm::Function *f,
                       const std::shared_ptr<llvm::ExecutionEngine> &fee)
      : f(f), fee(fee), module("Harness Module", LLVM_CONTEXT) {
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
    cout << fstr << endl;
  }

 private:
  llvm::Function *f;
  std::shared_ptr<llvm::ExecutionEngine> fee;
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
  llvm::Function *f = codegen(function);
  if (f == NULL) return NULL;
  return new LLVMCompiledFunction(f, executionEngine);
}

llvm::Function *LLVMCodeGen::codegen(Function *function) {
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

llvm::Value *
LLVMCodeGen::createScalarOp(const std::string &name, IndexExpr::Operator op,
                            const vector<IndexExpr::IndexedTensor> &operands) {
  for (auto &operand : operands) {
    assert(operand.getTensor()->getOrder() == 0);
  }

  switch (op) {
    case IndexExpr::NEG: {
      assert (operands.size() == 1);
      IndexExpr::IndexedTensor operand = operands[0];
      std::string operandName = operand.getTensor()->getName();

      assert(symtable->contains(operandName));
      llvm::Value *val = symtable->get(operandName);

      if (val->getType()->isPointerTy()) {
        val = builder->CreateLoad(val, operandName + VAL_SUFFIX);
      }

      simit::Type ctype = operand.getTensor()->getType()->getComponentType();
      switch (ctype) {
        case INT:
          return builder->CreateNeg(val, name);
        case FLOAT:
          return builder->CreateFNeg(val, name);
        case ELEMENT:
          assert(false && "Cannot negate element");
          break;
        default:
          UNREACHABLE_DEFAULT;
      }
    }
    case IndexExpr::ADD: // fallthrough
    case IndexExpr::SUB: // fallthrough
    case IndexExpr::MUL: // fallthrough
    case IndexExpr::DIV: {
      assert (operands.size() == 2);
      IndexExpr::IndexedTensor l = operands[0];
      IndexExpr::IndexedTensor r = operands[1];
      std::string lname = l.getTensor()->getName();
      std::string rname = r.getTensor()->getName();

      assert(symtable->contains(lname));
      assert(symtable->contains(rname));
      llvm::Value *lval = symtable->get(lname);
      llvm::Value *rval = symtable->get(rname);

      if (lval->getType()->isPointerTy()) {
        lval = builder->CreateLoad(lval, lname + VAL_SUFFIX);
      }
      if (rval->getType()->isPointerTy()) {
        rval = builder->CreateLoad(rval, rname + VAL_SUFFIX);
      }

      simit::Type ctype = l.getTensor()->getType()->getComponentType();
      return builder->CreateBinOp(toLLVMBinaryOp(op, ctype), lval, rval, name);
    }
    default:
      UNREACHABLE_DEFAULT;
  }

  return NULL;
}

void LLVMCodeGen::handle(IndexExpr *t) {
  llvm::Value *result = NULL;

  const std::vector<IndexExpr::IndexVarPtr> &domain = t->getDomain();
  IndexExpr::Operator op = t->getOperator();
  const std::vector<IndexExpr::IndexedTensor> &operands = t->getOperands();

  if (domain.size() == 0) {
    llvm::Value *resultStorage = storageLocations[t];
    assert(resultStorage);
    result = createScalarOp(t->getName(), op, operands);
    builder->CreateStore(result, resultStorage);
  }
  else {
    llvm::Value *resultStorage = storageLocations[t];
    assert(resultStorage);
  }

  assert(result != NULL);
  symtable->insert(t->getName(), result);
}

}}  // namespace simit::internal
