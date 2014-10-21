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

LLVMBackend::LLVMBackend() {
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
}

void LLVMBackend::visit(const Literal *op) {
  cout << "Literal" << endl;
}

void LLVMBackend::visit(const VarExpr *op) {
  assert(symtable.contains(op->var.name));
  val = symtable.get(op->var.name);
}

void LLVMBackend::visit(const Result *op) {
  cout << "Result" << endl;
}

void LLVMBackend::visit(const FieldRead *op) {
  cout << "FieldRead" << endl;
}

void LLVMBackend::visit(const TensorRead *op) {
  cout << "TensorRead" << endl;
}

void LLVMBackend::visit(const TupleRead *op) {
  cout << "TupleRead" << endl;
}

void LLVMBackend::visit(const Map *op) {
  cout << "Map" << endl;
}

void LLVMBackend::visit(const IndexedTensor *op) {
  cout << "IndexedTensor" << endl;
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
  if (op->function == "sin" && op->kind == Call::Intrinsic) {
    fun = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::sin, argTypes);
  }
  if (op->function == "cos" && op->kind == Call::Intrinsic) {
    fun = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::cos, argTypes);
  }
  if (op->function == "sqrt" && op->kind == Call::Intrinsic) {
    fun = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::sqrt, argTypes);
  }
  if (op->function == "log" && op->kind == Call::Intrinsic) {
    fun = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::log, argTypes);
  }
  if (op->function == "exp" && op->kind == Call::Intrinsic) {
    fun = llvm::Intrinsic::getDeclaration(module, llvm::Intrinsic::exp, argTypes);
  }
  if (op->function == "atan2" && op->kind == Call::Intrinsic) {
    // atan2 isn't an LLVM intrinsic
    auto ftype = llvm::FunctionType::get(LLVM_DOUBLE, argTypes, false);
    fun = llvm::cast<llvm::Function>(module->getOrInsertFunction("atan2", ftype));
  }
  
  // if not an intrinsic function, try to find it in the module
  if (!fun) {
    fun = module->getFunction(op->function);
  }
  
  if (fun)
    val = builder->CreateCall(fun, args);
  else
    assert(false && "Unsupported function call");
}

void LLVMBackend::visit(const Neg *op) {
  assert(isScalarTensor(op->type));

  cout << "Neg" << endl;
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

  cout << "Sub" << endl;
}

void LLVMBackend::visit(const Mul *op) {
  assert(isScalarTensor(op->type));

  cout << "Mul" << endl;
}

void LLVMBackend::visit(const Div *op) {
  assert(isScalarTensor(op->type));

  cout << "Div" << endl;
}


void LLVMBackend::visit(const AssignStmt *op) {
  llvm::Value *value = compile(op->value);
  string name = op->var.name;

  // Check if lhs already exist
  if (symtable.contains(name)) {
    llvm::Value *nameNode = symtable.get(name);

    // Check if the symbol is a function result
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
  }
}

void LLVMBackend::visit(const FieldWrite *op) {
  cout << "FieldWrite" << endl;
}

void LLVMBackend::visit(const TensorWrite *op) {
  cout << "TensorWrite" << endl;
}

void LLVMBackend::visit(const For *op) {
  cout << "For" << endl;
}

void LLVMBackend::visit(const IfThenElse *op) {
  cout << "IfThenElse" << endl;
}

void LLVMBackend::visit(const Block *op) {
  cout << "Block" << endl;
}

void LLVMBackend::visit(const Pass *op) {
  cout << "Pass" << endl;
}


}}  // namespace simit::internal
