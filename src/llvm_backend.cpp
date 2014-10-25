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

const std::string VAL_SUFFIX(".val");
const std::string PTR_SUFFIX(".ptr");
const std::string LEN_SUFFIX(".len");

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

  for (auto &arg : llvmFunc->getArgumentList()) {
    symtable.insert(arg.getName(), &arg);
  }

  if (func.getBody().defined()) {
    compile(func.getBody());
  }

  builder->CreateRetVoid();

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
  Expr setOrElem = op->elementOrSet;
  string fieldName = op->fieldName;
  assert(setOrElem.type().isElement() || setOrElem.type().isSet());

  const ElementType *elemType = nullptr;
  if (setOrElem.type().isElement()) {
    elemType = setOrElem.type().toElement();
  }
  else {
    elemType = setOrElem.type().toSet()->elementType.toElement();
  }

  llvm::Value *setOrElemValue = compile(op->elementOrSet);

  unsigned fieldLoc = elemType->fields.at(fieldName).location;
  val = builder->CreateExtractValue(setOrElemValue, {fieldLoc},
                                    setOrElemValue->getName()+"."+fieldName);
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
  NOT_SUPPORTED_YET;
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

  // Special case: check if the symbol is a scalar and the llvm value is a ptr,
  // in which case we must load the value.  This case arises because we keep
  // many scalars on the stack.  One exceptions to this are loop variables.
  if (isScalarTensor(op->type) && val->getType()->isPointerTy()) {
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
  assert(isScalarTensor(op->value.type()) &&
         "assignment non-scalars should have been lowered by now");

  llvm::Value *value = compile(op->value);
  string varName = op->var.name;

  // Assigned for the first time
  if (!symtable.contains(varName)) {
    ScalarType type = op->value.type().toTensor()->componentType;
    llvm::Value *var = builder->CreateAlloca(llvmType(type));
    symtable.insert(varName, var);
  }
  assert(symtable.contains(varName));

  llvm::Value *var = symtable.get(varName);
  assert(var->getType()->isPointerTy());
  builder->CreateStore(value, var);
  value->setName(varName + VAL_SUFFIX);
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

  llvm::Value *iNum;
  switch (domain.getKind()) {
    case IndexSet::Range:
      iNum = llvmInt(domain.getSize());
      break;
    case IndexSet::Set: {
      llvm::Value *setValue = compile(domain.getSet());
      iNum = builder->CreateExtractValue(setValue, {0},
                                         setValue->getName()+LEN_SUFFIX);
      break;
    }
    case IndexSet::Dynamic:
      NOT_SUPPORTED_YET;
      break;
  }
  assert(iNum);

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

  llvm::Value *exitCond = builder->CreateICmpSLT(i_nxt, iNum, iName+"_cmp");
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
