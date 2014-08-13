#include "codegen.h"

#include <iostream>
#include <stack>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
//#include "llvm/IR/Type.h"
//#include "llvm/IR/DerivedTypes.h"

#include "llvm/Analysis/Verifier.h"
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

namespace simit {
namespace internal {

typedef IndexExpr::IndexedTensor IndexedTensor;


static llvm::Type *toLLVMType(const simit::internal::TensorType *type) {
  llvm::Type *llvmType = NULL;
  if (type->getOrder() == 0) {
    switch (type->getComponentType()) {
      case Type::INT:
        llvmType = LLVM_INTPTR;
        break;
      case Type::FLOAT:
        llvmType = LLVM_DOUBLEPTR;
        break;
      case Type::ELEMENT:
        return NULL;  // TODO: not supported yet
        break;
      default:
        assert(false);
        break;
    }
  }
  else {
    return NULL;  // TODO: not supported yet
  }

  assert(llvmType != NULL);
  return llvmType;
}

static inline llvm::ConstantInt* constant(int val) {
    return llvm::ConstantInt::get(LLVM_CONTEXT, llvm::APInt(32, val, false));
}


class LLVMCodeGenImpl : public IRVisitor {
 public:
  LLVMCodeGenImpl() : module{"Simit JIT", LLVM_CONTEXT},
                      builder{LLVM_CONTEXT} {}

  BinaryFunction *compileToFunctionPointer(Function *function);
  llvm::Function *codegen(Function *function);

  void handle(Function *function);
  void handle(Argument      *t);
  void handle(Result        *t);
  void handle(LiteralTensor *t);
  void handle(IndexExpr     *t);
  void handle(VariableStore *t);
  
 private:
  llvm::Module              module;
  llvm::IRBuilder<>         builder;
  SymbolTable<llvm::Value*> symtable;
  std::stack<llvm::Value*>  results;

  llvm::Function *createFunctionPrototype(Function *function);
  llvm::Value *createScalarOp(const std::string &name, IndexExpr::Operator op,
                              const vector<IndexedTensor> &operands);
};

BinaryFunction *LLVMCodeGenImpl::compileToFunctionPointer(Function *function) {
  llvm::Function *f = codegen(function);
  if (f == NULL) return NULL;

  f->dump();
  // Pack up the llvm::Function in a simit BinaryFunction object
  return NULL;
}
llvm::Function *LLVMCodeGenImpl::codegen(Function *function) {
  visit(function);
  if (isAborted()) {
    return NULL;
  }
  builder.CreateRetVoid();

  llvm::Value *value = results.top();
  results.pop();
  assert(llvm::isa<llvm::Function>(value));
  return llvm::cast<llvm::Function>(value);
}

void LLVMCodeGenImpl::handle(Argument *t) {
  cout << "Argument:  " << *t << endl;
}

void LLVMCodeGenImpl::handle(Result *t) {
  cout << "Result:    " << *t << endl;
  llvm::Function *f = llvm::cast<llvm::Function>(value);
  verifyFunction(*f);
  return f;
}

void LLVMCodeGenImpl::handle(LiteralTensor *t) {
  cout << "Literal:   " << *t << endl;
}

void LLVMCodeGenImpl::handle(IndexExpr *t) {
  cout << "IndexExpr: " << *t << endl;
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
  symtable[t->getName()] = result;
}

void LLVMCodeGenImpl::handle(VariableStore *t) {
  cout << "Store   :  " << *t << endl;

  auto val = symtable[t->getValue()->getName()];
  assert(val != NULL);

  auto target = symtable[t->getTarget()->getName()];
  assert(target != NULL);

  builder.CreateStore(val, target);
}

llvm::Function *LLVMCodeGenImpl::createFunctionPrototype(Function *function) {
  vector<llvm::Type*> args;
  for (auto &arg : function->getArguments()) {
    auto llvmType = toLLVMType(arg->getType());
    if (llvmType == NULL) return NULL;  // TODO: Remove check
    args.push_back(llvmType);
  }
  for (auto &result : function->getResults()) {
    auto llvmType = toLLVMType(result->getType());
    if (llvmType == NULL) return NULL;  // TODO: Remove check
    args.push_back(llvmType);
  }
  llvm::FunctionType *ft = llvm::FunctionType::get(LLVM_VOID, args, false);
  llvm::Function *f = llvm::Function::Create(ft,
                                             llvm::Function::ExternalLinkage,
                                             function->getName(),
                                             &module);
  // Name arguments and results
  auto ai = f->arg_begin();
  for (auto &arg : function->getArguments()) {
    ai->setName(arg->getName());
    symtable[arg->getName()] = ai;
    ++ai;
  }
  for (auto &result : function->getResults()) {
    ai->setName(result->getName());
    symtable[result->getName()] = ai;
    ++ai;
  }
  assert(ai == f->arg_end());
  return f;
}

void LLVMCodeGenImpl::handle(Function *function) {
  llvm::Function *f = createFunctionPrototype(function);
  if (f == NULL) {  // TODO: Remove check
    abort();
    return;
  }
  auto entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", f);
  builder.SetInsertPoint(entry);

  results.push(f);
}

llvm::Value *
LLVMCodeGenImpl::createScalarOp(const std::string &name, IndexExpr::Operator op,
                                const vector<IndexedTensor> &operands) {
  llvm::Value *value = NULL;
  switch (op) {
    case IndexExpr::NEG: {
      assert (operands.size() == 1);
      auto operandName = operands[0].getTensor()->getName();
      auto val = symtable[operandName];
      assert(val != NULL);
      auto type = val->getType();
      if (type->isPointerTy()) {
        auto addr = builder.CreateGEP(val, constant(0), operandName + "_ptr");
        val = builder.CreateLoad(addr, operandName + "_val");
      }
      value = builder.CreateFNeg(val, name);
      break;
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
  }

  assert(value != NULL);
  return value;
}


/* class LLVMCodeGen */
LLVMCodeGen::LLVMCodeGen() : impl(new LLVMCodeGenImpl()) {
}

LLVMCodeGen::~LLVMCodeGen() {
  delete impl;
}

BinaryFunction *LLVMCodeGen::compileToFunctionPointer(Function *function) {
  cout << *function << endl;
  return impl->compileToFunctionPointer(function);
}

}}  // namespace simit::internal
