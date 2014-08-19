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

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/JIT.h"

#include "irvisitors.h"
#include "macros.h"
#include "ir.h"
#include "tensor.h"
#include "symboltable.h"

using namespace std;

#define LLVM_CONTEXT   llvm::getGlobalContext()

#define LLVM_VOID      llvm::Type::getVoidTy(LLVM_CONTEXT)
#define LLVM_INT       llvm::Type::getInt32Ty(LLVM_CONTEXT)
#define LLVM_INTPTR    llvm::Type::getInt32PtrTy(LLVM_CONTEXT)
#define LLVM_DOUBLE    llvm::Type::getDoubleTy(LLVM_CONTEXT)
#define LLVM_DOUBLEPTR llvm::Type::getDoublePtrTy(LLVM_CONTEXT)

namespace {
using namespace simit::internal;

inline llvm::ConstantInt *constant(const int val) {
  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(LLVM_CONTEXT), val);
}

llvm::Type *toLLVMType(const simit::Type &type) {
  llvm::Type *llvmType;
  switch (type) {
    case simit::Type::INT:
      llvmType = LLVM_INTPTR;
      break;
    case simit::Type::FLOAT:
      llvmType = LLVM_DOUBLEPTR;
      break;
    case simit::Type::ELEMENT:
      NOT_SUPPORTED_YET;
      break;
    default:
      UNREACHABLE_DEFAULT;
  }
  return llvmType;
}

llvm::Type *toLLVMType(const simit::internal::TensorType *type) {
  llvm::Type *llvmType = NULL;
  if (type->getOrder() == 0) {
    llvmType = toLLVMType(type->getComponentType());
  }
  else {
    return NULL;  // TODO: not supported yet
  }

  assert(llvmType != NULL);
  return llvmType;
}

llvm::Constant *toLLVMPtr(const std::shared_ptr<Literal> &literal) {
  llvm::Constant *c = (sizeof(void*) == 4)
      ? llvm::ConstantInt::get(llvm::Type::getInt32Ty(LLVM_CONTEXT),
                               (int)(intptr_t)literal->getData())
      : llvm::ConstantInt::get(llvm::Type::getInt64Ty(LLVM_CONTEXT),
                               (intptr_t)literal->getData());

  // TODO: Do we have to free ctype?
  llvm::Type *ctype = toLLVMType(literal->getType()->getComponentType());
  llvm::Constant *cptr = llvm::ConstantExpr::getIntToPtr(c, ctype);
  return cptr;
}

template <class AT, class RT>
llvm::FunctionType *createFunctionType(const vector<shared_ptr<AT>> &arguments,
                                       const vector<shared_ptr<RT>> &results) {
 // Create function harness
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

/// Creates an execution engine that takes ownership of the module.
llvm::ExecutionEngine *createExecutionEngine(llvm::Module *module) {
    llvm::EngineBuilder engineBuilder(module);
    // TODO: Intialization
    auto ee = engineBuilder.create();
    assert(ee && "Could not create ExecutionEngine");
    return ee;
}

class LLVMCompiledFunction : public CompiledFunction {
 public:
  LLVMCompiledFunction(llvm::Function *f,
                       const std::shared_ptr<llvm::ExecutionEngine> &fee)
      : f(f), fee(fee), module("Harness Module", LLVM_CONTEXT) {
    fee->addModule(&module);
  }

  ~LLVMCompiledFunction() { fee->removeModule(&module); }

  virtual void bind(const std::vector<std::shared_ptr<Literal>> &arguments,
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
  if (isAborted()) {
    return NULL;
  }
  builder->CreateRetVoid();

  llvm::Value *value = resultStack.top();
  resultStack.pop();
  assert(llvm::isa<llvm::Function>(value));
  llvm::Function *f = llvm::cast<llvm::Function>(value);
  f->dump();
  verifyFunction(*f);
  return f;
}

void LLVMCodeGen::handle(Function *function) {
  llvm::Function *f = createPrototype(function->getName(),
                                      function->getArguments(),
                                      function->getResults(),
                                      llvm::Function::ExternalLinkage,
                                      module);
  cout << "Function" << endl;
  auto entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", f);
  builder->SetInsertPoint(entry);
  for (auto &arg : f->getArgumentList()) {
    symtable->insert(arg.getName(), &arg);
  }
  
  if (f == NULL) {  // TODO: Remove check
    abort();
    return;
  }
  resultStack.push(f);
}

void LLVMCodeGen::handle(IndexExpr *t) {
  cout << "Index Expression" << endl;
  llvm::Value *result = NULL;

  auto domain = t->getDomain();
  auto op = t->getOperator();
  auto &operands = t->getOperands();

  if (domain.size() == 0) {
    result = createScalarOp(t->getName(), op, operands);
  }
  else {
    NOT_SUPPORTED_YET;
  }

  assert(result != NULL);
  symtable->insert(t->getName(), result);

  cout << "End of Index Expression" << endl;
}

void LLVMCodeGen::handle(VariableStore *t) {
  assert(symtable->contains(t->getValue()->getName()));
  auto val = symtable->get(t->getValue()->getName());
  assert(val != NULL);

  assert(symtable->contains(t->getValue()->getName()));
  auto target = symtable->get(t->getTarget()->getName());
  assert(target != NULL);

  builder->CreateStore(val, target);
}

llvm::Value *
LLVMCodeGen::createScalarOp(const std::string &name, IndexExpr::Operator op,
                            const vector<IndexExpr::IndexedTensor> &operands) {
  switch (op) {
    case IndexExpr::NEG: {
      assert (operands.size() == 1);
      IndexExpr::IndexedTensor operand = operands[0];
      std::string operandName = operand.getTensor()->getName();

      assert(symtable->contains(operandName));
      llvm::Value *val = symtable->get(operandName);

      if (val->getType()->isPointerTy()) {
        val = builder->CreateLoad(val, operandName + "_val");
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
    case IndexExpr::ADD:
      NOT_SUPPORTED_YET;
      break;
    case IndexExpr::SUB:
      NOT_SUPPORTED_YET;
      break;
    case IndexExpr::MUL:
      NOT_SUPPORTED_YET;
      break;
    case IndexExpr::DIV:
      NOT_SUPPORTED_YET;
      break;
    default:
      UNREACHABLE_DEFAULT;
  }

  return NULL;
}

}}  // namespace simit::internal
