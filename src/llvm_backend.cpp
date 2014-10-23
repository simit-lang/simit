#include "llvm_backend.h"

#include <cstdint>
#include <iostream>
#include <stack>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/ADT/SmallVector.h"
#include "llvm/Analysis/Verifier.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/ExecutionEngine/JIT.h"

#include "llvm_codegen.h"
#include "types.h"
#include "ir.h"
#include "ir_printer.h"
#include "llvm_function.h"
#include "macros.h"

using namespace std;
using namespace simit::ir;
using namespace simit::internal;

namespace simit {
namespace internal {

const std::string VAL_SUFFIX("_val");
const std::string PTR_SUFFIX("_ptr");
const std::string OFFSET_SUFFIX("_ofs");

// class LLVMBackend
bool LLVMBackend::llvmInitialized = false;

LLVMBackend::LLVMBackend() : val(nullptr) {
  if (!llvmInitialized) {
    llvm::InitializeNativeTarget();
    llvmInitialized = true;
  }

  builder = new llvm::IRBuilder<>(LLVM_CONTEXT);
}

LLVMBackend::~LLVMBackend() {
  delete builder;
}

simit::Function *LLVMBackend::compile(Func func) {
  module = new llvm::Module(func.getName(), LLVM_CONTEXT);

  llvm::Function *llvmFunc = createFunction(func.getName(), func.getArguments(),
                                            func.getResults(), module);
  auto entry = llvm::BasicBlock::Create(LLVM_CONTEXT, "entry", llvmFunc);
  builder->SetInsertPoint(entry);

  size_t i=0;
  auto &simitArgs = func.getArguments();
  for (auto &arg : llvmFunc->getArgumentList()) {
    // Load scalar arguments
    if (i++ < simitArgs.size()) {
      if (isScalarTensor(simitArgs[i-1].type)) {
        string valName = string(arg.getName()) + VAL_SUFFIX;
        llvm::Value *val = builder->CreateLoad(&arg, valName);
        symtable.insert(arg.getName(), val);
      }
      else {
        symtable.insert(arg.getName(), &arg);
      }
    }
    // Result
    else {
      symtable.insert(arg.getName(), &arg);
      results.insert(&arg);
    }
  }

  compile(func.getBody());

  builder->CreateRetVoid();

  results.clear();
  symtable.clear();
  return new LLVMFunction(func, llvmFunc, module);
}

llvm::Value *LLVMBackend::compile(const Expr &expr) {
  expr.accept(this);
  llvm::Value *tmp = val;
  val = nullptr;
  return tmp;
}

void LLVMBackend::compile(const Stmt &stmt) {
  stmt.accept(this);
  val = nullptr;
}

void LLVMBackend::visit(const FieldRead *op) {
  assert(false && "No code generation for this type");
}

void LLVMBackend::visit(const TensorRead *op) {
  assert(false && "No code generation for this type");
}

void LLVMBackend::visit(const TupleRead *op) {
  assert(false && "No code generation for this type");
}

void LLVMBackend::visit(const Map *op) {
  assert(false && "No code generation for this type");
}

void LLVMBackend::visit(const IndexedTensor *op) {
  assert(false && "No code generation for this type");
}

void LLVMBackend::visit(const IndexExpr *op) {
  assert(false && "No code generation for this type");
}

void LLVMBackend::visit(const FieldWrite *op) {
  assert(false && "No code generation for this type");
}

void LLVMBackend::visit(const TensorWrite *op) {
  assert(false && "No code generation for this type");
}

void LLVMBackend::visit(const Literal *op) {
  assert(op->type.isTensor() && "Only tensor literals supported for now");
  const TensorType *type = op->type.toTensor();

  if (type->order() == 0) {
    ScalarType ctype = type->componentType;
    switch (ctype.kind) {
      case ScalarType::Int: {
        assert(ctype.bytes() == 4 && "Only 4-byte ints currently supported");
        val = llvmInt(((int*)op->data)[0]);
        break;
      }
      case ScalarType::Float: {
        assert(ctype.bytes() == 8 && "Only 8-byte floats currently supported");
        val = llvmFP(((double*)op->data)[0]);
      }
    }
  }
  else {
    // Literal vectors, matrices tensors
    NOT_SUPPORTED_YET;
  }
  assert(val);
}

void LLVMBackend::visit(const VarExpr *op) {
  assert(symtable.contains(op->var.name));
  val = symtable.get(op->var.name);

  // Special case: check if the symbol is a scalar function result, in which
  // case we must load the value since we pass in all results as pointers.
  if (isScalarTensor(op->type) && results.find(val) != results.end()) {
    string valName = string(val->getName()) + VAL_SUFFIX;
    val = builder->CreateAlignedLoad(val, 8, valName);
  }
}

void LLVMBackend::visit(const Result *op) {
  cout << "Result" << endl;
}

void LLVMBackend::visit(const ir::Load *op) {
  llvm::Value *buffer = compile(op->buffer);
  llvm::Value *index = compile(op->index);

  string locName = string(buffer->getName()) + PTR_SUFFIX;
  llvm::Value *bufferLoc = builder->CreateInBoundsGEP(buffer, index, locName);

  string valName = string(buffer->getName()) + VAL_SUFFIX;
  val = builder->CreateAlignedLoad(bufferLoc, 8, valName);
}

void LLVMBackend::visit(const Call *op) {
  std::vector<llvm::Type*> argTypes;
  std::vector<llvm::Value*> args;
  llvm::Function *fun = nullptr;
  
  // compile arguments first
  for (auto a: op->actuals) {
    assert(isScalarTensor(a.type()));
    argTypes.push_back(llvmType(a.type().toTensor()->componentType));
    args.push_back(compile(a));
  }

  // these are intrinsic functions
  if (op->function == ir::Intrinsics::sin) {
    fun = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::sin, argTypes);
  }
  else if (op->function == ir::Intrinsics::cos) {
    fun = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::cos, argTypes);
  }
  else if (op->function == ir::Intrinsics::atan2) {
    // atan2 isn't an LLVM intrinsic
    auto ftype = llvm::FunctionType::get(LLVM_DOUBLE, argTypes, false);
    fun = llvm::cast<llvm::Function>(module->getOrInsertFunction("atan2", ftype));
  }
  else if (op->function == ir::Intrinsics::sqrt) {
    fun = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::sqrt, argTypes);
  }
  else if (op->function == ir::Intrinsics::log) {
    fun = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::log, argTypes);
  }
  else if (op->function == ir::Intrinsics::exp) {
    fun = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::exp, argTypes);
  }
  
  // if not an intrinsic function, try to find it in the module
  if (!fun) {
    fun = module->getFunction(op->function.getName());
  }
  
  if (fun)
    val = builder->CreateCall(fun, args);
  else
    assert(false && "Unsupported function call");
}

void LLVMBackend::visit(const Neg *op) {
  assert(isScalarTensor(op->type));
  llvm::Value *a = compile(op->a);

  switch (op->type.toTensor()->componentType.kind) {
    case ScalarType::Int:
      val = builder->CreateNeg(a);
      break;
    case ScalarType::Float:
      val = builder->CreateFNeg(a);
      break;
  }
}

void LLVMBackend::visit(const Add *op) {
  assert(isScalarTensor(op->type));

  llvm::Value *a = compile(op->a);
  llvm::Value *b = compile(op->b);

  switch (op->type.toTensor()->componentType.kind) {
    case ScalarType::Int:
      val = builder->CreateAdd(a, b);
      break;
    case ScalarType::Float:
      val = builder->CreateFAdd(a, b);
      break;
  }
}

void LLVMBackend::visit(const Sub *op) {
  assert(isScalarTensor(op->type));

  llvm::Value *a = compile(op->a);
  llvm::Value *b = compile(op->b);

  switch (op->type.toTensor()->componentType.kind) {
    case ScalarType::Int:
      val = builder->CreateSub(a, b);
      break;
    case ScalarType::Float:
      val = builder->CreateFSub(a, b);
      break;
  }
}

void LLVMBackend::visit(const Mul *op) {
  assert(isScalarTensor(op->type));

  llvm::Value *a = compile(op->a);
  llvm::Value *b = compile(op->b);

  switch (op->type.toTensor()->componentType.kind) {
    case ScalarType::Int:
      val = builder->CreateMul(a, b);
      break;
    case ScalarType::Float:
      val = builder->CreateFMul(a, b);
      break;
  }
}

void LLVMBackend::visit(const Div *op) {
  assert(isScalarTensor(op->type));

  llvm::Value *a = compile(op->a);
  llvm::Value *b = compile(op->b);

  switch (op->type.toTensor()->componentType.kind) {
    case ScalarType::Int:
      // TODO: Figure out what's the deal with integer div. Cast to fp, div and
      // truncate?
      NOT_SUPPORTED_YET;
      break;
    case ScalarType::Float:
      val = builder->CreateFDiv(a, b);
      break;
  }
}


void LLVMBackend::visit(const AssignStmt *op) {
  llvm::Value *value = compile(op->value);
  string name = op->var.name;

  // Check if lhs already exist (re-assigning to variable)
  if (symtable.contains(name)) {
    llvm::Value *nameNode = symtable.get(name);

    // Special case: check if the symbol is a function result, in which case
    // it is an assignment of a scalar to a result.  Since we pass in all
    // results as pointers we must store it to the variable ptr.
    if (results.find(nameNode) != results.end()) {
      builder->CreateStore(value, nameNode);
      value->setName(name + VAL_SUFFIX);
    }
    else {
      value->setName(name);
    }
  }
  else {
    value->setName(name);
    symtable.insert(name, value);
  }
}

void LLVMBackend::visit(const ir::Store *op) {
  llvm::Value *buffer = compile(op->buffer);
  llvm::Value *index = compile(op->index);
  llvm::Value *value = compile(op->value);

  string locName = string(buffer->getName()) + PTR_SUFFIX;
  llvm::Value *bufferLoc = builder->CreateInBoundsGEP(buffer, index, locName);
  builder->CreateAlignedStore(value, bufferLoc, 8);
}

void LLVMBackend::visit(const For *op) {
  std::string iName = op->var.name;
  IndexSet domain = op->domain;

  llvm::Value *iMax;
  if (domain.getKind() == IndexSet::Range) {
    iMax = llvmInt(domain.getSize());
  }
  else {
    NOT_SUPPORTED_YET;
  }
  assert(iMax);

  llvm::Function *llvmFunc = builder->GetInsertBlock()->getParent();

  // Loop Header
  llvm::BasicBlock *entryBlock = builder->GetInsertBlock();

  llvm::BasicBlock *loopBodyStart =
      llvm::BasicBlock::Create(LLVM_CONTEXT, iName+"_loop_body", llvmFunc);
  builder->CreateBr(loopBodyStart);
  builder->SetInsertPoint(loopBodyStart);

  llvm::PHINode *i = builder->CreatePHI(LLVM_INT32, 2, iName);
  i->addIncoming(builder->getInt32(0), entryBlock);

  // Loop Body
  symtable.scope();
  symtable.insert(iName, i);
  compile(op->body);
  symtable.unscope();

  // Loop Footer
  llvm::BasicBlock *loopBodyEnd = builder->GetInsertBlock();
  llvm::Value *i_nxt = builder->CreateAdd(i, builder->getInt32(1),
                                          iName+"_nxt", false, true);
  i->addIncoming(i_nxt, loopBodyEnd);

  llvm::Value *exitCond = builder->CreateICmpSLT(i_nxt, iMax, iName+"_cmp");
  llvm::BasicBlock *loopEnd = llvm::BasicBlock::Create(LLVM_CONTEXT,
                                                       iName+"_loop_end", llvmFunc);
  builder->CreateCondBr(exitCond, loopBodyStart, loopEnd);
  builder->SetInsertPoint(loopEnd);
}

void LLVMBackend::visit(const IfThenElse *op) {
  cout << "IfThenElse" << endl;
  NOT_SUPPORTED_YET;
}

void LLVMBackend::visit(const Block *op) {
  compile(op->first);
  compile(op->rest);
}

}}  // namespace simit::internal
