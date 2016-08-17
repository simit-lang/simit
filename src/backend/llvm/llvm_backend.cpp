#include "llvm_backend.h"

#include <cstdint>
#include <iostream>
#include <stack>
#include <algorithm>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Value.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallVector.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Analysis/Passes.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"

#include "llvm_types.h"
#include "llvm_codegen.h"
#include "llvm_util.h"
#include "llvm_data_layouts.h"

#include "macros.h"
#include "types.h"
#include "func.h"
#include "ir.h"
#include "intrinsics.h"
#include "ir_printer.h"
#include "ir_queries.h"
#include "ir_transforms.h"
#include "ir_rewriter.h" // TODO: Remove this header
#include "environment.h"
#include "tensor_index.h"
#include "llvm_function.h"
#include "macros.h"
#include "path_expressions.h"
#include "util/collections.h"

using namespace std;
using namespace simit::ir;

namespace simit {
namespace backend {

const std::string VAL_SUFFIX(".val");
const std::string PTR_SUFFIX(".ptr");
const std::string LEN_SUFFIX(".len");

// class LLVMBackend
bool LLVMBackend::llvmInitialized = false;

shared_ptr<llvm::EngineBuilder> createEngineBuilder(llvm::Module *module) {
  shared_ptr<llvm::EngineBuilder> engineBuilder(
      new llvm::EngineBuilder(std::unique_ptr<llvm::Module>(module)));
  return engineBuilder;
}

LLVMBackend::LLVMBackend() : builder(new SimitIRBuilder(LLVM_CTX)) {
  if (!llvmInitialized) {
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmPrinters();
    llvm::InitializeAllAsmParsers();
    llvmInitialized = true;
  }
}

LLVMBackend::~LLVMBackend() {}

// TODO: Remove this function, once the old init system has been removed
Func LLVMBackend::makeSystemTensorsGlobal(Func func) {
  class MakeSystemTensorsGlobalRewriter : public ir::IRRewriter {
  public:
    MakeSystemTensorsGlobalRewriter() {}

  private:
    Environment environment;

    void visit(const Func* f) {
      environment = f->getEnvironment();

      Stmt body = rewrite(f->getBody());
      if (body != f->getBody()) {
        func = Func(f->getName(), f->getArguments(), f->getResults(), body,
                    environment);
        func.setStorage(f->getStorage());
      }
      else {
        func = *f;
      }
    }

    void visit(const VarDecl* op) {
      const Var& var = op->var;
      if (isSystemTensorType(var.getType()) && environment.hasTensorIndex(var)){
        environment.addTemporary(var);
      }
      else {
        stmt = op;
      }
    }
  };
  return MakeSystemTensorsGlobalRewriter().rewrite(func);
}

Function* LLVMBackend::compile(ir::Func func, const ir::Storage& storage) {
  this->module = new llvm::Module("simit", LLVM_CTX);

  iassert(func.getBody().defined()) << "cannot compile an undefined function";

  this->dataLayout.reset(new llvm::DataLayout(module));

  this->symtable.clear();
  this->buffers.clear();
  this->globals.clear();
  this->storage = storage;

  // This backend stores dense tensors and sparse tensors with path expressions
  // as globals.
  func = makeSystemTensorsGlobal(func);

  this->environment = &func.getEnvironment();
  emitGlobals(*this->environment);

  // Create compute functions
  vector<Func> callTree = getCallTree(func);
  std::reverse(callTree.begin(), callTree.end());

  llvm::Function *llvmFunc = nullptr;
  for (auto &f : callTree) {
    if (f.getKind() != Func::Internal) {
      continue;
    }
    iassert(f.getBody().defined());

    this->storage.add(f.getStorage());

    // Emit function
    symtable.scope(); // put function arguments in new scope

    bool exported = (f == func);
    llvmFunc = emitEmptyFunction(f.getName(), f.getArguments(),
                                 f.getResults(), exported);

    // Add constants to symbol table
    for (auto &global : f.getEnvironment().getConstants()) {
      symtable.insert(global.first, compile(global.second));
    }

    // LLVM does not de-allocate any stack memory until a function returns, so
    // we must make sure to not allocate stack memory inside a loop. To do this
    // we move all the var decls to the front of the function body
    Stmt body = moveVarDeclsToFront(f.getBody());

    compile(body);
    builder->CreateRetVoid();

    symtable.unscope();
  }
  iassert(llvmFunc);

  // Declare malloc and free if necessary
  llvm::FunctionType *m =
      llvm::FunctionType::get(LLVM_INT8_PTR, {LLVM_INT}, false);
  llvm::Function *malloc =
      llvm::cast<llvm::Function>(module->getOrInsertFunction("malloc", m));
  llvm::FunctionType *f =
      llvm::FunctionType::get(LLVM_VOID, {LLVM_INT8_PTR}, false);
  llvm::Function *free =
      llvm::cast<llvm::Function>(module->getOrInsertFunction("free", f));

  // Create initialization function
  emitEmptyFunction(func.getName()+"_init", func.getArguments(),
                    func.getResults(), true);
  for (auto &buffer : buffers) {
    const Var&   bufferVar = buffer.first;
    llvm::Value* bufferVal = buffer.second;

    Type type = bufferVar.getType();
    llvm::Type *ltype = llvmType(type);

    iassert(type.isTensor());
    const TensorType *ttype = type.toTensor();
    llvm::Value *len= emitComputeLen(ttype,this->storage.getStorage(bufferVar));
    unsigned compSize = ttype->getComponentType().bytes();
    llvm::Value *size = builder->CreateMul(len, llvmInt(compSize));
    llvm::Value *mem = builder->CreateCall(malloc, size);

    mem = builder->CreateCast(llvm::Instruction::CastOps::BitCast, mem, ltype);
    builder->CreateStore(mem, bufferVal);
  }
  builder->CreateRetVoid();
  symtable.clear();


  // Create de-initialization function
  emitEmptyFunction(func.getName()+"_deinit", func.getArguments(),
                    func.getResults(), true);
  for (auto &buffer : buffers) {
    Var var = buffer.first;
    llvm::Value *bufferVal = buffer.second;

    llvm::Value *tmpPtr = builder->CreateLoad(bufferVal);
    tmpPtr = builder->CreateCast(llvm::Instruction::CastOps::BitCast,
                                 tmpPtr, LLVM_INT8_PTR);
    builder->CreateCall(free, tmpPtr);
  }
  builder->CreateRetVoid();
  symtable.clear();

  iassert(!llvm::verifyModule(*module))
      << "LLVM module does not pass verification";

  auto engineBuilder = createEngineBuilder(module);

#ifndef SIMIT_DEBUG
  // Run LLVM optimization passes on the function
  // We use the built-in PassManagerBuilder to build
  // the set of passes that are similar to clang's -O3
  llvm::legacy::FunctionPassManager fpm(module);
  llvm::legacy::PassManager mpm;
  llvm::PassManagerBuilder pmBuilder;
  
  pmBuilder.OptLevel = 3;

  pmBuilder.BBVectorize = 1;
  pmBuilder.LoopVectorize = 1;
//  pmBuilder.LoadCombine = 1;
  pmBuilder.SLPVectorize = 1;

  llvm::DataLayout dataLayout(module);
  setDataLayout(fpm, dataLayout, module);

  pmBuilder.populateFunctionPassManager(fpm);
  pmBuilder.populateModulePassManager(mpm);

  fpm.doInitialization();
  fpm.run(*llvmFunc);
  fpm.doFinalization();
  
  mpm.run(*module);
#endif

  return new LLVMFunction(func, storage, llvmFunc, module, engineBuilder);
}

void LLVMBackend::compile(const ir::Literal& literal) {
  iassert(literal.type.isTensor()) << "Only tensor literals supported for now";
  const TensorType *type = literal.type.toTensor();

  if (type->order() == 0) {
    ScalarType ctype = type->getComponentType();
    switch (ctype.kind) {
      case ScalarType::Int: {
        iassert(ctype.bytes() == 4) << "Only 4-byte ints currently supported";
        val = llvmInt(((int*)literal.data)[0]);
        break;
      }
      case ScalarType::Float: {
        iassert(ctype.bytes() == ScalarType::floatBytes)
            << "Only " << ScalarType::floatBytes
            << "-byte float mode allowed by current float setting";
        val = llvmFP(literal.getFloatVal(0));
        break;
      }
      case ScalarType::Boolean: {
        iassert(ctype.bytes() == sizeof(bool));
        bool data = ((bool*)literal.data)[0];
        val = llvm::ConstantInt::get(LLVM_BOOL, llvm::APInt(1, data, false));
        break;
      }
      case ScalarType::Complex: {
        iassert(ctype.bytes() == ScalarType::floatBytes*2)
          << "Only " << ScalarType::floatBytes
          << "-byte float mode allowed by current float setting";
        val = llvmComplex(literal.getFloatVal(0), literal.getFloatVal(1));
        break;
      }
      case ScalarType::String: {
        iassert(ctype.bytes() == sizeof(char));
        std::string data((const char *)literal.data);
        val = emitGlobalString(data);
        break;
      }
      default: unreachable;
    }
  }
  else {
    // TODO: This should become a reference to a global literal
    // (unify with GPUBackend).
    val = llvmPtr(literal);
  }
  iassert(val);
}

void LLVMBackend::compile(const ir::VarExpr& varExpr) {
  iassert(symtable.contains(varExpr.var))
      << varExpr.var << " not found in symbol table:\n\n" << symtable;

  val = symtable.get(varExpr.var);

  string ptrName = string(val->getName());
  string valName = string(val->getName()) + VAL_SUFFIX;

  // Globals are stored as pointer-pointers so we must load them
  if (util::contains(globals, varExpr.var)) {
    val = builder->CreateLoad(val, ptrName);
    // Cast non-generic address spaces into generic
    if (val->getType()->isPointerTy() &&
        val->getType()->getPointerAddressSpace() != 0) {
      llvm::Type* eltTy = val->getType()->getPointerElementType();
      val = builder->CreateAddrSpaceCast(val, eltTy->getPointerTo(0));
    }
  }

  // Special case: check if the symbol is a scalar and the llvm value is a ptr,
  // in which case we must load the value.  This case arises because we keep
  // many scalars on the stack.  An exceptions to this are loop variables,
  // which is why we can't assume Simit scalars are always kept on the stack.
  //if (isScalar(varExpr.type) && val->getType()->isPointerTy()) {
  if (isScalar(varExpr.type)) {
    iassert(!isString(varExpr.type) || val->getType()->isPointerTy());
    if (val->getType()->isPointerTy() && (!isString(varExpr.type) || 
        val->getType()->getContainedType(0)->isPointerTy())) {
      val = builder->CreateLoad(val, valName);
    }
  }
}

void LLVMBackend::compile(const ir::Load& load) {
  llvm::Value *buffer = compile(load.buffer);
  llvm::Value *index = compile(load.index);

  string locName = string(buffer->getName()) + PTR_SUFFIX;
  llvm::Value *bufferLoc = builder->CreateInBoundsGEP(buffer, index, locName);

  string valName = string(buffer->getName()) + VAL_SUFFIX;
  val = builder->CreateLoad(bufferLoc, valName);
}

void LLVMBackend::compile(const ir::FieldRead& fieldRead) {
  val = emitFieldRead(fieldRead.elementOrSet, fieldRead.fieldName);
}

void LLVMBackend::compile(const ir::Length& length) {
  val = emitComputeLen(length.indexSet);
}

void LLVMBackend::compile(const ir::IndexRead& indexRead) {
  iassert(indexRead.edgeSet.type().isSet());
  llvm::Value *edgesValue = compile(indexRead.edgeSet);
  std::shared_ptr<SetLayout> layout =
      getSetLayout(indexRead.edgeSet, edgesValue, builder.get());
  switch (indexRead.kind) {
    case ir::IndexRead::Endpoints:
      val = layout->getEpsArray();
      break;
    case ir::IndexRead::LatticeDim:
      iassert(indexRead.edgeSet.type().isLatticeLinkSet());
      val = layout->getSize(indexRead.index);
      break;
    default:
      unreachable;
  }
}

void LLVMBackend::compile(const ir::Neg& negExpr) {
  iassert(isScalar(negExpr.type));
  llvm::Value *a = compile(negExpr.a);

  switch (negExpr.type.toTensor()->getComponentType().kind) {
    case ScalarType::Int:
      val = builder->CreateNeg(a);
      break;
    case ScalarType::Float:
      val = builder->CreateFNeg(a);
      break;
    case ScalarType::Complex: {
      llvm::Value *realNeg = builder->CreateFNeg(builder->ComplexGetReal(a));
      llvm::Value *imagNeg = builder->CreateFNeg(builder->ComplexGetImag(a));
      val = builder->CreateComplex(realNeg, imagNeg);
      break;
    }
    case ScalarType::Boolean:
    case ScalarType::String:
      ierror << "Cannot negate a boolean or string value.";
      break;
  }
}

void LLVMBackend::compile(const ir::Add& addExpr) {
  iassert(isScalar(addExpr.type));

  llvm::Value *a = compile(addExpr.a);
  llvm::Value *b = compile(addExpr.b);

  switch (addExpr.type.toTensor()->getComponentType().kind) {
    case ScalarType::Int:
      val = builder->CreateAdd(a, b);
      break;
    case ScalarType::Float:
      val = builder->CreateFAdd(a, b);
      break;
    case ScalarType::Complex: {
      llvm::Value *realA = builder->ComplexGetReal(a);
      llvm::Value *imagA = builder->ComplexGetImag(a);
      llvm::Value *realB = builder->ComplexGetReal(b);
      llvm::Value *imagB = builder->ComplexGetImag(b);
      llvm::Value *real = builder->CreateFAdd(realA, realB);
      llvm::Value *imag = builder->CreateFAdd(imagA, imagB);
      val = builder->CreateComplex(real, imag);
      break;
    }
    case ScalarType::Boolean:
    case ScalarType::String:
      ierror << "Cannot add boolean or string values.";
      break;
  }
}

void LLVMBackend::compile(const ir::Sub& subExpr) {
  iassert(isScalar(subExpr.type));

  llvm::Value *a = compile(subExpr.a);
  llvm::Value *b = compile(subExpr.b);

  switch (subExpr.type.toTensor()->getComponentType().kind) {
    case ScalarType::Int:
      val = builder->CreateSub(a, b);
      break;
    case ScalarType::Float:
      val = builder->CreateFSub(a, b);
      break;
    case ScalarType::Complex: {
      llvm::Value *realA = builder->ComplexGetReal(a);
      llvm::Value *imagA = builder->ComplexGetImag(a);
      llvm::Value *realB = builder->ComplexGetReal(b);
      llvm::Value *imagB = builder->ComplexGetImag(b);
      llvm::Value *real = builder->CreateFSub(realA, realB);
      llvm::Value *imag = builder->CreateFSub(imagA, imagB);
      val = builder->CreateComplex(real, imag);
      break;
    }
    case ScalarType::Boolean:
    case ScalarType::String:
      ierror << "Cannot subtract boolean or string values.";
      break;
  }
}

void LLVMBackend::compile(const ir::Mul& mulExpr) {
  iassert(isScalar(mulExpr.type));

  llvm::Value *a = compile(mulExpr.a);
  llvm::Value *b = compile(mulExpr.b);

  switch (mulExpr.type.toTensor()->getComponentType().kind) {
    case ScalarType::Int:
      val = builder->CreateMul(a, b);
      break;
    case ScalarType::Float:
      val = builder->CreateFMul(a, b);
      break;
    case ScalarType::Complex: {
      llvm::Value *realA = builder->ComplexGetReal(a);
      llvm::Value *imagA = builder->ComplexGetImag(a);
      llvm::Value *realB = builder->ComplexGetReal(b);
      llvm::Value *imagB = builder->ComplexGetImag(b);
      llvm::Value *real = builder->CreateFSub(
          builder->CreateFMul(realA, realB),
          builder->CreateFMul(imagA, imagB));
      llvm::Value *imag = builder->CreateFAdd(
          builder->CreateFMul(realA, imagB),
          builder->CreateFMul(imagA, realB));
      val = builder->CreateComplex(real, imag);
      break;
    }
    case ScalarType::Boolean:
    case ScalarType::String:
      ierror << "Cannot multiply boolean or string values.";
      break;
  }
}

void LLVMBackend::compile(const ir::Div& divExpr) {
  iassert(isScalar(divExpr.type));

  llvm::Value *a = compile(divExpr.a);
  llvm::Value *b = compile(divExpr.b);

  switch (divExpr.type.toTensor()->getComponentType().kind) {
    case ScalarType::Int:
      // TODO: Figure out what's the deal with integer div. Cast to fp, div and
      // truncate?
      not_supported_yet;
      break;
    case ScalarType::Float:
      val = builder->CreateFDiv(a, b);
      break;
    case ScalarType::Complex: {
      // Computed using the naive ((ac+bd)/(c^2+d^2), (bc-ad)/(c^2+d^2))
      llvm::Value *realA = builder->ComplexGetReal(a);
      llvm::Value *imagA = builder->ComplexGetImag(a);
      llvm::Value *realB = builder->ComplexGetReal(b);
      llvm::Value *imagB = builder->ComplexGetImag(b);
      llvm::Value *denom = builder->CreateFAdd(
          builder->CreateFMul(realB, realB),
          builder->CreateFMul(imagB, imagB));
      llvm::Value *num1 = builder->CreateFAdd(
          builder->CreateFMul(realA, realB),
          builder->CreateFMul(imagA, imagB));
      llvm::Value *num2 = builder->CreateFSub(
          builder->CreateFMul(imagA, realB),
          builder->CreateFMul(realA, imagB));
      llvm::Value *real = builder->CreateFDiv(num1, denom);
      llvm::Value *imag = builder->CreateFDiv(num2, denom);
      val = builder->CreateComplex(real, imag);
      break;
    }
    case ScalarType::Boolean:
    case ScalarType::String:
      ierror << "Cannot divide boolean or string values.";
      break;
  }
}

void LLVMBackend::compile(const ir::Rem& remExpr) {
  iassert(isScalar(remExpr.type));
  iassert(isInt(remExpr.type));
  iassert(isInt(remExpr.a.type()));
  iassert(isInt(remExpr.b.type()));

  llvm::Value *a = compile(remExpr.a);
  llvm::Value *b = compile(remExpr.b);

  // Use SRem to match the truncated semantics
  val = builder->CreateSRem(a, b);
}

void LLVMBackend::compile(const ir::Not& notExpr) {
  iassert(isBoolean(notExpr.type));
  iassert(isBoolean(notExpr.a.type()));

  llvm::Value *a = compile(notExpr.a);

  val = builder->CreateNot(a);
}

#define LLVMBACKEND_VISIT_COMPARE_OP(Type, op, float_cmp, int_cmp)             \
void LLVMBackend::compile(Type op) {                                           \
  iassert(isBoolean(op.type));                                                 \
  iassert(isScalar(op.a.type()));                                              \
  iassert(isScalar(op.b.type()));                                              \
                                                                               \
  llvm::Value *a = compile(op.a);                                              \
  llvm::Value *b = compile(op.b);                                              \
                                                                               \
  const TensorType *ttype = op.a.type().toTensor();                            \
  switch (ttype->getComponentType().kind) {                                    \
    case ScalarType::Float:                                                    \
      val = builder->float_cmp(a, b);                                          \
      break;                                                                   \
    case ScalarType::Int:                                                      \
    case ScalarType::Boolean:                                                  \
      val = builder->int_cmp(a, b);                                            \
      break;                                                                   \
    default:                                                                   \
      not_supported_yet;                                                       \
  }                                                                            \
}

LLVMBACKEND_VISIT_COMPARE_OP(const Eq&, op, CreateFCmpOEQ, CreateICmpEQ)
LLVMBACKEND_VISIT_COMPARE_OP(const Ne&, op, CreateFCmpONE, CreateICmpNE)
LLVMBACKEND_VISIT_COMPARE_OP(const Gt&, op, CreateFCmpOGT, CreateICmpSGT)
LLVMBACKEND_VISIT_COMPARE_OP(const Lt&, op, CreateFCmpOLT, CreateICmpSLT)
LLVMBACKEND_VISIT_COMPARE_OP(const Ge&, op, CreateFCmpOGE, CreateICmpSGE)
LLVMBACKEND_VISIT_COMPARE_OP(const Le&, op, CreateFCmpOLE, CreateICmpSLE)

void LLVMBackend::compile(const ir::And& andExpr) {
  iassert(isBoolean(andExpr.type));
  iassert(isBoolean(andExpr.a.type()));
  iassert(isBoolean(andExpr.b.type()));

  llvm::Value *a = compile(andExpr.a);
  llvm::Value *b = compile(andExpr.b);

  val = builder->CreateAnd(a, b);
}

void LLVMBackend::compile(const ir::Or& orExpr) {
  iassert(isBoolean(orExpr.type));
  iassert(isBoolean(orExpr.a.type()));
  iassert(isBoolean(orExpr.b.type()));

  llvm::Value *a = compile(orExpr.a);
  llvm::Value *b = compile(orExpr.b);

  val = builder->CreateOr(a, b);
}

void LLVMBackend::compile(const ir::Xor& xorExpr) {
  iassert(isBoolean(xorExpr.type));
  iassert(isBoolean(xorExpr.a.type()));
  iassert(isBoolean(xorExpr.b.type()));

  llvm::Value *a = compile(xorExpr.a);
  llvm::Value *b = compile(xorExpr.b);

  val = builder->CreateXor(a, b);
}

void LLVMBackend::compile(const ir::VarDecl& varDecl) {
  Var var = varDecl.var;
  Type type = var.getType();

  // Do not duplicate variable storage, even on a duplicated declaration
  if (symtable.contains(var)) {
    return;
  }

  llvm::Value *llvmVar = nullptr;
  if (type.isTensor()) {
    if (isScalar(type)) {
      ScalarType stype = type.toTensor()->getComponentType();
      llvmVar = builder->CreateAlloca(llvmType(stype), nullptr, var.getName());

      if (isString(type)) {
        // Initialize pointer to null.
        llvm::Value *valuePtr = defaultInitializer(llvmType(ScalarType::String));
        builder->CreateStore(valuePtr, llvmVar);
      }
    }
    else {
      auto tensorStorage = storage.getStorage(varDecl.var);

      // Sparse matrices with path expressions are stored globally
      if (tensorStorage.getKind() != TensorStorage::Indexed ||
          tensorStorage.getTensorIndex().getPathExpression().defined()) {
        llvmVar = makeGlobalTensor(varDecl.var);
      }
      // Sparse matrices without path expressions are managed locally
      else {
        auto index = tensorStorage.getTensorIndex();
        auto rowptr = index.getRowptrArray();
        auto rowptrPtr = builder->CreateAlloca(LLVM_INT_PTR, llvmInt(1),
                                               rowptr.getName()+PTR_SUFFIX);
        auto colidx = index.getColidxArray();
        auto colidxPtr = builder->CreateAlloca(LLVM_INT_PTR, llvmInt(1),
                                               colidx.getName()+PTR_SUFFIX);
        symtable.insert(rowptr, rowptrPtr);
        symtable.insert(colidx, colidxPtr);

        llvmVar = builder->CreateAlloca(llvmType(type), llvmInt(1),
                                        var.getName()+PTR_SUFFIX);
      }
    }
  } else if (type.isOpaque()) {
    llvmVar = builder->CreateAlloca(llvmType(type), nullptr, var.getName());
  }
  else {
    terror << type << " declarations not supported yet";
  }

  iassert(llvmVar);
  symtable.insert(var, llvmVar);
}

void LLVMBackend::compile(const ir::AssignStmt& assignStmt) {
  switch (assignStmt.cop) {
    case ir::CompoundOperator::None: {
      emitAssign(assignStmt.var, assignStmt.value);
      return;
    }
    case ir::CompoundOperator::Add: {
      emitAssign(assignStmt.var, Add::make(assignStmt.var, assignStmt.value));
      return;
    }
    default: ierror << "Unknown compound operator type";
  }
}

std::vector<llvm::Value*>
LLVMBackend::emitArgument(ir::Expr argument, bool excludeStaticTypes) {
  std::vector<llvm::Value*> argumentValues;

  if (argument.type().isTensor()) {
    auto type = argument.type().toTensor();
    vector<IndexDomain> dimensions = type->getDimensions();

    // Sparse matrices
    if (type->order() == 2 && type->hasSystemDimensions()) {
      // we need to add additional arguments: rowPtr and colIdx pointers,
      // as well as the number of rows, columns and blocksize.
      tassert(isa<VarExpr>(argument));

      auto tensorStorage = storage.getStorage(to<VarExpr>(argument)->var);
      auto tensorIndex = tensorStorage.getTensorIndex();

      llvm::Value *rowptr = compile(tensorIndex.getRowptrArray());
      llvm::Value *colidx = compile(tensorIndex.getColidxArray());

      auto n = emitComputeLen(dimensions[0]);
      auto m = emitComputeLen(dimensions[1]);

      // get block sizes.
      llvm::Value* nn;
      llvm::Value* mm;
      Type blockType = type->getBlockType();
      if (isScalar(blockType)) {
        nn = llvmInt(1);
        mm = llvmInt(1);
      }
      else {
        auto blockDimensions = blockType.toTensor()->getDimensions();
        nn = emitComputeLen(blockDimensions[0]);
        mm = emitComputeLen(blockDimensions[1]);
      }

      // Argument list:
      // - Top-level type:    n, m
      // - Top-level indices: rowPtr, colIdx,
      // - Block type:        nn, mm
      // - Values:  vals
      argumentValues.push_back(n);
      argumentValues.push_back(m);
      argumentValues.push_back(rowptr);
      argumentValues.push_back(colidx);
      argumentValues.push_back(nn);
      argumentValues.push_back(mm);
    }
    // Dense vectors and matrices
    else if (type->order() == 1 || type->order() == 2) {
      if (!excludeStaticTypes) {
        auto N = emitComputeLen(dimensions[0]);
        argumentValues.push_back(N);
      }
    }
  }
  argumentValues.push_back(compile(argument));

  return argumentValues;
}

std::vector<llvm::Value*>
LLVMBackend::emitArguments(std::vector<ir::Expr> arguments,
                           bool excludeStaticTypes) {
  std::vector<llvm::Value*> args;
  for (auto argument : arguments) {
    auto arg = emitArgument(argument, excludeStaticTypes);
    args.insert(args.end(), arg.begin(), arg.end());
  }
  return args;
}

vector<llvm::Value*>
LLVMBackend::emitResult(ir::Var result, bool excludeStaticTypes,
                        vector<std::pair<ir::Var, llvm::Value*>>* resVals) {
  std::vector<llvm::Value*> resultValues;
  Type type = result.getType();

  if (type.isTensor()) {
    auto tensorType = result.getType().toTensor();
    vector<IndexDomain> dimensions = tensorType->getDimensions();

    // Emit type arguments
    // Sparse Matrices
    if (tensorType->order() == 2 && tensorType->hasSystemDimensions()) {
      // we need to add additional arguments: rowPtr and colIdx pointers,
      // as well as the number of rows, columns and blocksize.
      tassert(isa<VarExpr>(result));

      auto tensorStorage = storage.getStorage(to<VarExpr>(result)->var);
      auto tensorIndex = tensorStorage.getTensorIndex();

      auto rowptr = tensorIndex.getRowptrArray();
      auto rowptrPtr = symtable.get(rowptr);
      resVals->push_back(make_pair(rowptr, rowptrPtr));

      auto colidx = tensorIndex.getColidxArray();
      auto colidxPtr = symtable.get(colidx);
      resVals->push_back(make_pair(colidx, colidxPtr));

      auto n = emitComputeLen(dimensions[0]);
      auto m = emitComputeLen(dimensions[1]);

      // get block sizes
      llvm::Value* nn;
      llvm::Value* mm;
      Type blockType = tensorType->getBlockType();
      if (isScalar(blockType)) {
        nn = llvmInt(1);
        mm = llvmInt(1);
      }
      else {
        auto blockDimensions = blockType.toTensor()->getDimensions();
        nn = emitComputeLen(blockDimensions[0]);
        mm = emitComputeLen(blockDimensions[1]);
      }

      auto resultPtr = symtable.get(result);
      resVals->push_back(make_pair(result, resultPtr));

      // Argument list::
      // - Type:    N, M, Nb, Mb,
      // - Indices: rowptr, colidx,
      // - Values:  vals
      resultValues.push_back(n);
      resultValues.push_back(m);
      resultValues.push_back(rowptrPtr);
      resultValues.push_back(colidxPtr);
      resultValues.push_back(nn);
      resultValues.push_back(mm);
      resultValues.push_back(resultPtr);
    }
    // Dense vectors and matrices
    else if (tensorType->order() == 1 || tensorType->order() == 2) {
      if (!excludeStaticTypes) {
        auto N = emitComputeLen(dimensions[0]);
        resultValues.push_back(N);
      }
      resultValues.push_back(compile(result));
    }
  }
  else if (type.isOpaque()) {
    resultValues.push_back(compile(result));
  }
  else {
    terror << type << " type return values not supported yet";
  }

  return resultValues;
}

vector<llvm::Value*>
LLVMBackend::emitResults(vector<ir::Var> results, bool excludeStaticTypes,
                         vector<std::pair<ir::Var, llvm::Value*>>* resVals) {
  std::vector<llvm::Value*> resultValues;
  for (auto result : results) {
    auto res = emitResult(result, excludeStaticTypes, resVals);
    resultValues.insert(resultValues.end(), res.begin(), res.end());
  }
  return resultValues;
}

void LLVMBackend::emitInternalCall(const ir::CallStmt& callStmt) {
  auto args = emitArguments(callStmt.actuals, true);

  if (module->getFunction(callStmt.callee.getName())) {
    for (Var r : callStmt.results) {
      args.push_back(symtable.get(r));
    }

    llvm::Function* fun = module->getFunction(callStmt.callee.getName());
    builder->CreateCall(fun, args);
  }
  else {
    ierror << "function " << callStmt.callee.getName()
           << " not found in module";
  }
}

void LLVMBackend::emitExternCall(const ir::CallStmt& callStmt) {
  // ensure it is called with the correct number of arguments.
  uassert(callStmt.actuals.size() == callStmt.callee.getArguments().size()) <<
      "External function '" << callStmt.callee.getName() << "' called with " <<
      callStmt.actuals.size() << " arguments, but expected " <<
      callStmt.callee.getArguments().size() << " arguments.";

  // Arguments
  auto args = emitArguments(callStmt.actuals, false);

  // Results
  vector<std::pair<ir::Var, llvm::Value*>> resultVals;
  if (callStmt.results.size() > 0) {
    auto results = emitResults(callStmt.results, false, &resultVals);
    args.insert(args.end(), results.begin(), results.end());
  }

  // Function name
  std::string name = callStmt.callee.getName();
  name = name.substr(0, name.find("@"));
  std::string floatType = ir::ScalarType::singleFloat() ? "s" : "d";
  name = floatType + name;

  auto errorCode = emitCall(name, args, LLVM_INT);
  UNUSED(errorCode);  // TODO: Accept and handle error code from extern func

  // Load the results into llvm variables
  for (auto resultVal : resultVals) {
    auto var = resultVal.first;
    auto llvmPtr = resultVal.second;
    auto llvmVar = builder->CreateLoad(llvmPtr, var.getName());
    symtable.insert(var, llvmVar);
  }
}

void LLVMBackend::emitIntrinsicCall(const ir::CallStmt& callStmt) {
  auto args = emitArguments(callStmt.actuals, true);

  llvm::Function *fun = nullptr;
  Func callee = callStmt.callee;

  iassert(callee != ir::intrinsics::norm() && callee != ir::intrinsics::dot())
      << "norm and dot should have been lowered";

  std::map<Func, llvm::Intrinsic::ID> llvmIntrinsicByName =
      {{ir::intrinsics::sin(), llvm::Intrinsic::sin},
       {ir::intrinsics::cos(), llvm::Intrinsic::cos},
       {ir::intrinsics::sqrt(),llvm::Intrinsic::sqrt},
       {ir::intrinsics::log(), llvm::Intrinsic::log},
       {ir::intrinsics::exp(), llvm::Intrinsic::exp},
       {ir::intrinsics::pow(), llvm::Intrinsic::pow}};

  std::string floatTypeName = ir::ScalarType::singleFloat() ? "_f32" : "_f64";

  llvm::Value *call = nullptr;

  // is it an LLVM intrinsic?
  auto foundIntrinsic = llvmIntrinsicByName.find(callStmt.callee);
  if (foundIntrinsic != llvmIntrinsicByName.end()) {
    iassert(callStmt.results.size() == 1);
    auto ctype = callStmt.results[0].getType().toTensor()->getComponentType();
    llvm::Type *overloadType = llvmType(ctype);
    fun = llvm::Intrinsic::getDeclaration(module, foundIntrinsic->second,
                                          {overloadType});
    call = builder->CreateCall(fun, args);
  }
  // is it an intrinsic from libm?
  else if (callStmt.callee == ir::intrinsics::atan2() ||
           callStmt.callee == ir::intrinsics::tan()   ||
           callStmt.callee == ir::intrinsics::asin()  ||
           callStmt.callee == ir::intrinsics::acos()) {
    std::string fname = callStmt.callee.getName() + floatTypeName;
    call = emitCall(fname, args, llvmFloatType());
  }
  else if (callStmt.callee == ir::intrinsics::mod()) {
    iassert(callStmt.actuals.size() == 2) << "mod takes two inputs, got"
        << callStmt.actuals.size();
    call = builder->CreateSRem(compile(callStmt.actuals[0]),
                               compile(callStmt.actuals[1]));
  }
  else if (callStmt.callee == ir::intrinsics::loc()) {
    call = emitCall("loc", args, LLVM_INT);
  }
  else if (callStmt.callee == ir::intrinsics::free()) {
    auto arg = args[args.size()-1];
    arg = builder->CreateCast(llvm::Instruction::CastOps::BitCast, arg, LLVM_INT8_PTR);
    call = emitCall("free", {arg}, LLVM_VOID);
  }
  else if (callStmt.callee == ir::intrinsics::malloc()) {
    call = emitCall("malloc", args, LLVM_INT8_PTR);
  }
  else if (callStmt.callee == ir::intrinsics::strcmp()) {
    call = emitCall("strcmp", args, LLVM_INT);
  }
  else if (callStmt.callee == ir::intrinsics::strlen()) {
    call = emitCall("strlen", args, LLVM_INT);
  }
  else if (callStmt.callee == ir::intrinsics::strcpy()) {
    call = emitCall("strcpy", args, LLVM_INT8_PTR);
  }
  else if (callStmt.callee == ir::intrinsics::strcat()) {
    call = emitCall("strcat", args, LLVM_INT8_PTR);
  }
  else if (callStmt.callee == ir::intrinsics::clock()) {
    call = emitCall("clock", args, llvmFloatType());
  }
  else if (callStmt.callee == ir::intrinsics::storeTime()) {
    call = emitCall("storeTime", args);
  }
  else if (callee == ir::intrinsics::det()) {
    iassert(args.size() == 1);
    std::string fname = callStmt.callee.getName() + "3" + floatTypeName;
    call = emitCall(fname, args, llvmFloatType());
  }
  else if (callee == ir::intrinsics::inv()) {
    iassert(args.size() == 1);

    Var result = callStmt.results[0];
    llvm::Value *llvmResult = symtable.get(result);
    args.push_back(llvmResult);

    std::string fname = callStmt.callee.getName() + "3" + floatTypeName;
    call = emitCall(fname, args);
  }
  else if (callStmt.callee == ir::intrinsics::solve()) {
    std::string fname = "cMatSolve" + floatTypeName;
    call = emitCall(fname, args);
  }
  else if (callStmt.callee == ir::intrinsics::complexNorm()) {
    std::string fname = "complexNorm" + floatTypeName;
    call = emitCall(fname, {builder->ComplexGetReal(args[0]),
      builder->ComplexGetImag(args[0])}, llvmFloatType());
  }
  else if (callStmt.callee == ir::intrinsics::createComplex()) {
    call = builder->CreateComplex(args[0], args[1]);
  }
  else if (callStmt.callee == ir::intrinsics::complexGetReal()) {
    call = builder->ComplexGetReal(args[0]);
  }
  else if (callStmt.callee == ir::intrinsics::complexGetImag()) {
    call = builder->ComplexGetImag(args[0]);
  }
  else if (callStmt.callee == ir::intrinsics::complexConj()) {
    auto real = builder->ComplexGetReal(args[0]);
    auto imag = builder->CreateFNeg(builder->ComplexGetImag(args[0]));
    call = builder->CreateComplex(real, imag);
  }
  else {
    ierror << "intrinsic " << callStmt.callee.getName() << " not found";
  }
  iassert(call);

  if (!call->getType()->isVoidTy()) {
    iassert(callStmt.results.size() == 1);
    Var var = callStmt.results[0];
    llvm::Value *llvmVar = symtable.get(var);
    builder->CreateStore(call, llvmVar);
  }
}

void LLVMBackend::compile(const ir::CallStmt& callStmt) {
  switch (callStmt.callee.getKind()) {
    case Func::Internal:
      emitInternalCall(callStmt);
      break;
    case Func::Intrinsic:
      emitIntrinsicCall(callStmt);
      break;
    case Func::External:
      emitExternCall(callStmt);
      break;
  }
}

void LLVMBackend::compile(const ir::Store& store) {
  llvm::Value *buffer = compile(store.buffer);
  llvm::Value *index = compile(store.index);
  llvm::Value *value;
  switch (store.cop) {
    case CompoundOperator::None: {
      value = compile(store.value);
      break;
    }
    case CompoundOperator::Add: {
      value = compile(Add::make(Load::make(store.buffer, store.index),
                                store.value));
      break;
    }
  }
  iassert(value != nullptr);

  string locName = string(buffer->getName()) + PTR_SUFFIX;
  llvm::Value *bufferLoc = builder->CreateInBoundsGEP(buffer, index, locName);
  builder->CreateStore(value, bufferLoc);
}

void LLVMBackend::compile(const ir::FieldWrite& fieldWrite) {
  iassert(fieldWrite.value.type().isTensor());
  iassert(getFieldType(fieldWrite.elementOrSet,
                       fieldWrite.fieldName).isTensor());
  iassert(fieldWrite.elementOrSet.type().isSet() ||
          fieldWrite.elementOrSet.type().isElement());

  Type fieldType = getFieldType(fieldWrite.elementOrSet, fieldWrite.fieldName);
  Type valueType = fieldWrite.value.type();

  // Assigning a scalar to an n-order tensor
  if (fieldType.toTensor()->order() > 0 && valueType.toTensor()->order() == 0) {
    iassert(fieldWrite.cop == CompoundOperator::None)
        << "Compound write when assigning scalar to n-order tensor";
    if (isa<Literal>(fieldWrite.value) &&
        to<Literal>(fieldWrite.value)->getFloatVal(0) == 0.0) {
      // emit memset 0
      llvm::Value *fieldPtr = emitFieldRead(fieldWrite.elementOrSet,
                                            fieldWrite.fieldName);

      const TensorType *tensorFieldType = fieldType.toTensor();

      // For now we'll assume fields are always dense row major
      llvm::Value *fieldLen =
          emitComputeLen(tensorFieldType, TensorStorage::Dense);
      unsigned compSize = tensorFieldType->getComponentType().bytes();
      llvm::Value *fieldSize = builder->CreateMul(fieldLen,llvmInt(compSize));

      emitMemSet(fieldPtr, llvmInt(0,8), fieldSize, compSize);
    }
    else {
      not_supported_yet;
    }
  }
  else {
    // emit memcpy
    llvm::Value *fieldPtr = emitFieldRead(fieldWrite.elementOrSet,
                                          fieldWrite.fieldName);
    llvm::Value *valuePtr;
    switch (fieldWrite.cop) {
      case ir::CompoundOperator::None: {
        valuePtr = compile(fieldWrite.value);
        break;
      }
      case ir::CompoundOperator::Add: {
        valuePtr = compile(Add::make(
            FieldRead::make(fieldWrite.elementOrSet, fieldWrite.fieldName),
                                     fieldWrite.value));
        break;
      }
    }
    iassert(valuePtr != nullptr);

    const TensorType *tensorFieldType = fieldType.toTensor();

    // For now we'll assume fields are always dense row major
    llvm::Value *fieldLen =
        emitComputeLen(tensorFieldType, TensorStorage::Dense);
    unsigned elemSize = tensorFieldType->getComponentType().bytes();
    llvm::Value *fieldSize = builder->CreateMul(fieldLen, llvmInt(elemSize));

    emitMemCpy(fieldPtr, valuePtr, fieldSize, elemSize);
  }
}

void LLVMBackend::compile(const ir::Scope& scope) {
  symtable.scope();
  compile(scope.scopedStmt);
  symtable.unscope();
}

void LLVMBackend::compile(const ir::IfThenElse& ifThenElse) {
  llvm::Function *llvmFunc = builder->GetInsertBlock()->getParent();

  llvm::Value *cond = compile(ifThenElse.condition);
  llvm::Value *condEval = builder->CreateICmpEQ(builder->getTrue(), cond);


  llvm::BasicBlock *thenBlock = llvm::BasicBlock::Create(LLVM_CTX, "then",
                                                         llvmFunc);
  llvm::BasicBlock *elseBlock = llvm::BasicBlock::Create(LLVM_CTX, "else");
  llvm::BasicBlock *exitBlock = llvm::BasicBlock::Create(LLVM_CTX, "exit");
  builder->CreateCondBr(condEval, thenBlock, elseBlock);

  builder->SetInsertPoint(thenBlock);
  compile(ifThenElse.thenBody);
  builder->CreateBr(exitBlock);
  thenBlock = builder->GetInsertBlock();

  llvmFunc->getBasicBlockList().push_back(elseBlock);

  builder->SetInsertPoint(elseBlock);
  if (ifThenElse.elseBody.defined()) {
    compile(ifThenElse.elseBody);
  }
  builder->CreateBr(exitBlock);
  elseBlock = builder->GetInsertBlock();

  llvmFunc->getBasicBlockList().push_back(exitBlock);
  builder->SetInsertPoint(exitBlock);
}

void LLVMBackend::compile(const ir::ForRange& forLoop) {
  std::string iName = forLoop.var.getName();
  
  llvm::Function *llvmFunc = builder->GetInsertBlock()->getParent();
  
  // Loop Header
  llvm::BasicBlock *entryBlock = builder->GetInsertBlock();

  llvm::Value *rangeStart = compile(forLoop.start);
  llvm::Value *rangeEnd = compile(forLoop.end);

  llvm::BasicBlock *loopBodyStart =
    llvm::BasicBlock::Create(LLVM_CTX, iName+"_loop_body", llvmFunc);
  llvm::BasicBlock *loopEnd = llvm::BasicBlock::Create(LLVM_CTX,
                                                       iName+"_loop_end",
                                                       llvmFunc);
  llvm::Value *firstCmp = builder->CreateICmpSLT(rangeStart, rangeEnd);
  builder->CreateCondBr(firstCmp, loopBodyStart, loopEnd);
  builder->SetInsertPoint(loopBodyStart);

  llvm::PHINode *i = builder->CreatePHI(LLVM_INT32, 2, iName);
  i->addIncoming(rangeStart, entryBlock);

  // Loop Body
  symtable.insert(forLoop.var, i);
  compile(forLoop.body);

  // Loop Footer
  llvm::BasicBlock *loopBodyEnd = builder->GetInsertBlock();
  llvm::Value *i_nxt = builder->CreateAdd(i, builder->getInt32(1),
                                          iName+"_nxt", false, true);
  i->addIncoming(i_nxt, loopBodyEnd);

  llvm::Value *exitCond = builder->CreateICmpSLT(i_nxt, rangeEnd,
                                                 iName+"_cmp");
  builder->CreateCondBr(exitCond, loopBodyStart, loopEnd);
  builder->SetInsertPoint(loopEnd);

}

void LLVMBackend::compile(const ir::For& forLoop) {
  std::string iName = forLoop.var.getName();
  ForDomain domain = forLoop.domain;

  llvm::Value *iNum = nullptr;
  switch (domain.kind) {
    case ForDomain::IndexSet: {
      iNum = emitComputeLen(domain.indexSet);
      break;
    }
    case ForDomain::Endpoints:
      not_supported_yet;
      break;
    case ForDomain::Edges:
      not_supported_yet;
      break;
    case ForDomain::Lattice:
      not_supported_yet;
      break;
    case ForDomain::NeighborsOf:
    case ForDomain::Neighbors:
      not_supported_yet;
      break;
    case ForDomain::Diagonal:
      not_supported_yet;
      break;
  }
  iassert(iNum);

  llvm::Function *llvmFunc = builder->GetInsertBlock()->getParent();

  // Loop Header
  llvm::BasicBlock *entryBlock = builder->GetInsertBlock();

  llvm::BasicBlock *loopBodyStart =
      llvm::BasicBlock::Create(LLVM_CTX, iName+"_loop_body", llvmFunc);
  llvm::BasicBlock *loopEnd = llvm::BasicBlock::Create(LLVM_CTX,
                                                       iName+"_loop_end", llvmFunc);
  llvm::Value *firstCmp = builder->CreateICmpSLT(llvmInt(0), iNum);
  builder->CreateCondBr(firstCmp, loopBodyStart, loopEnd);
  builder->SetInsertPoint(loopBodyStart);

  llvm::PHINode *i = builder->CreatePHI(LLVM_INT32, 2, iName);
  i->addIncoming(builder->getInt32(0), entryBlock);

  // Loop Body
  symtable.insert(forLoop.var, i);
  compile(forLoop.body);

  // Loop Footer
  llvm::BasicBlock *loopBodyEnd = builder->GetInsertBlock();
  llvm::Value *i_nxt = builder->CreateAdd(i, builder->getInt32(1),
                                          iName+"_nxt", false, true);
  i->addIncoming(i_nxt, loopBodyEnd);

  llvm::Value *exitCond = builder->CreateICmpSLT(i_nxt, iNum, iName+"_cmp");
  builder->CreateCondBr(exitCond, loopBodyStart, loopEnd);
  builder->SetInsertPoint(loopEnd);
}

void LLVMBackend::compile(const ir::While& whileLoop) {
  llvm::Function *llvmFunc = builder->GetInsertBlock()->getParent();

  llvm::Value *cond = compile(whileLoop.condition);
  llvm::Value *condEval = builder->CreateICmpEQ(builder->getTrue(), cond);


  llvm::BasicBlock *bodyBlock = llvm::BasicBlock::Create(LLVM_CTX, "body",
                                                         llvmFunc);
  llvm::BasicBlock *checkBlock = llvm::BasicBlock::Create(LLVM_CTX,"check");
  llvm::BasicBlock *exitBlock = llvm::BasicBlock::Create(LLVM_CTX, "exit");
  builder->CreateCondBr(condEval, bodyBlock, exitBlock);

  builder->SetInsertPoint(bodyBlock);
  compile(whileLoop.body);
  builder->CreateBr(checkBlock);
  
  // We actually need to save the original body block, because the
  // current block could be different (e.g. if another loop was added
  // as part of the while's body).
  auto priorBodyBlock = bodyBlock;
  bodyBlock = builder->GetInsertBlock();
  
  llvmFunc->getBasicBlockList().push_back(checkBlock);
  builder->SetInsertPoint(checkBlock);
  llvm::Value *cond2 = compile(whileLoop.condition);
  llvm::Value *condEval2 = builder->CreateICmpEQ(builder->getTrue(), cond2);

  builder->CreateCondBr(condEval2, priorBodyBlock, exitBlock);
  
  llvmFunc->getBasicBlockList().push_back(exitBlock);
  builder->SetInsertPoint(exitBlock);
}

void LLVMBackend::compile(const ir::Print& print) {
  std::vector<llvm::Value*> args;
  
  llvm::Value *result = compile(print.expr);
  Type type = print.expr.type();

  iassert(isScalar(type)) << "Backend can only compile scalars";

  const TensorType *tensor = type.toTensor();
  ScalarType scalarType = tensor->getComponentType();

  if (scalarType == ScalarType::String) {
    emitPrintf(result, {});
  } else {
    std::string specifier;
    switch (scalarType.kind) {
      case ScalarType::Float:
        specifier = std::string("%") + print.format + "g";
        break;
      case ScalarType::Complex:
        specifier = std::string("%") + print.format + "<%g,%g>";
        break;
      case ScalarType::Boolean:
      case ScalarType::Int:
        specifier = std::string("%") + print.format + "d";
        break;
      case ScalarType::String:
        unreachable;
        break;
    }

    args.push_back(result);
    emitPrintf(specifier, args);
  }
}


// helper methods
llvm::Function *LLVMBackend::getBuiltIn(std::string name,
                                        llvm::Type *retTy,
                                        std::vector<llvm::Type*> argTys) {
  llvm::FunctionType *funcTy;
  if (argTys.size() > 0) {
    funcTy = llvm::FunctionType::get(retTy, argTys, false);
  }
  else {
    funcTy = llvm::FunctionType::get(retTy, false);
  }
  module->getOrInsertFunction(name, funcTy);
  return module->getFunction(name);
}

llvm::Value *LLVMBackend::emitFieldRead(const Expr &elemOrSet,
                                        std::string fieldName) {
  assert(elemOrSet.type().isElement() || elemOrSet.type().isSet());
  llvm::Value *setOrElemValue = compile(elemOrSet);
  const ElementType *elemType = nullptr;
  int fieldsOffset = -1;
  if (elemOrSet.type().isElement()) {
    elemType = elemOrSet.type().toElement();
    fieldsOffset = 0;
  }
  else {
    const SetType *setType = elemOrSet.type().toSet();
    elemType = setType->elementType.toElement();
    std::shared_ptr<SetLayout> layout =
        getSetLayout(elemOrSet, setOrElemValue, builder.get());
    fieldsOffset = layout->getFieldsOffset();
  }
  assert(fieldsOffset >= 0);
  
  assert(elemType->hasField(fieldName));
  unsigned fieldLoc = fieldsOffset + elemType->fieldNames.at(fieldName);
  return builder->CreateExtractValue(setOrElemValue, {fieldLoc},
                                     setOrElemValue->getName()+"."+fieldName);
}

llvm::Value *LLVMBackend::emitComputeLen(const TensorType *tensorType,
                                         const TensorStorage &tensorStorage) {
  if (tensorType->order() == 0) {
    return llvmInt(1);
  }

  vector<IndexDomain> dimensions = tensorType->getDimensions();

  llvm::Value *len = nullptr;
  switch (tensorStorage.getKind()) {
    case TensorStorage::Dense: {
      auto it = dimensions.begin();
      len = emitComputeLen(*it++);
      for (; it != dimensions.end(); ++it) {
        len = builder->CreateMul(len, emitComputeLen(*it));
      }
      break;
    }
    case TensorStorage::Diagonal: {
      iassert(dimensions.size() > 0);

      // Just need one outer dimensions because diagonal implies square
      len = emitComputeLen(tensorType->getOuterDimensions()[0]);

      Type blockType = tensorType->getBlockType();
      llvm::Value *blockLen = emitComputeLen(blockType.toTensor(),
                                             TensorStorage::Dense);
      len = builder->CreateMul(len, blockLen);
      break;
    }
    case TensorStorage::Indexed: {
      // We retrieve the number of non-zero blocks in the index, which is stored
      // in the last (sentinel) entry of the coords/rowptr index array.

      // We assume the sparse matrix is stored using the BCSR format, which
      // means the size of the coords array is the size of the first matrix
      // dimension + 1.
      auto dimSize = emitComputeLen(tensorType->getOuterDimensions()[0]);

      // Retrieve the index of this tensor from the environment
      auto tensorIndex = tensorStorage.getTensorIndex();

      llvm::Value* coordArrayPtr = symtable.get(tensorIndex.getRowptrArray());
      llvm::Value* coordArray = builder->CreateAlignedLoad(coordArrayPtr, 8);
      llvm::Value* coordArrayLast
          = builder->CreateInBoundsGEP(coordArray, dimSize,
                                       "coords"+LEN_SUFFIX+PTR_SUFFIX);
      len = builder->CreateAlignedLoad(coordArrayLast, 8);

      // Multiply by block size
      Type blockType = tensorType->getBlockType();
      if (!isScalar(blockType)) {
        // TODO: The following assumes all blocks are dense row major. The right
        //       way to assign a storage order for every block in the tensor
        //       represented by a TensorStorage
        llvm::Value *blockSize = emitComputeLen(blockType.toTensor(),
                                                TensorStorage::Dense);
        len = builder->CreateMul(len, blockSize);
      }
      break;
    }
    case TensorStorage::Undefined:
      ierror << "Can't compute the size of tensor with undefined storage";
      break;
    default:
      unreachable;
      break;
  }
  iassert(len != nullptr);
  return len;
}

llvm::Value *LLVMBackend::emitComputeLen(const IndexDomain &dom) {
  assert(dom.getIndexSets().size() > 0);

  auto it = dom.getIndexSets().begin();
  llvm::Value *result = emitComputeLen(*it++);
  for (; it != dom.getIndexSets().end(); ++it) {
    result = builder->CreateMul(result, emitComputeLen(*it));
  }
  return result;
}

llvm::Value *LLVMBackend::emitComputeLen(const IndexSet &is) {
  switch (is.getKind()) {
    case IndexSet::Range:
      return llvmInt(is.getSize());
      break;
    case IndexSet::Set: {
      llvm::Value *setValue = compile(is.getSet());
      return builder->CreateExtractValue(setValue, {0},
                                         setValue->getName()+LEN_SUFFIX);
    }
    case IndexSet::Single:
      unreachable;
    case IndexSet::Dynamic:
      not_supported_yet;
      break;
  }
  unreachable;
  return nullptr;
}

llvm::Value *LLVMBackend::loadFromArray(llvm::Value *array, llvm::Value *index){
  llvm::Value *loc = builder->CreateGEP(array, index);
  return builder->CreateLoad(loc);
}

llvm::Value *LLVMBackend::emitCall(string name, vector<llvm::Value*> args) {
  return emitCall(name, args, LLVM_VOID);
}

llvm::Value *LLVMBackend::emitCall(string name, vector<llvm::Value*> args,
                                   llvm::Type *returnType) {
  std::vector<llvm::Type*> argTypes;
  for (auto &arg : args) {
    argTypes.push_back(arg->getType());
  }

  llvm::FunctionType *ftype =
      llvm::FunctionType::get(returnType, argTypes, false);
  llvm::Function *fun =
      llvm::cast<llvm::Function>(module->getOrInsertFunction(name, ftype));
  
  iassert(fun != nullptr)
      << "could not find" << fun << "with the given signature";

  return builder->CreateCall(fun, std::vector<llvm::Value*>(args));
}

llvm::Constant *LLVMBackend::emitGlobalString(const std::string& str) {
  auto strValue = llvm::ConstantDataArray::getString(LLVM_CTX, str);
  auto strType = llvm::ArrayType::get(LLVM_INT8, str.size()+1);

  llvm::GlobalVariable *strGlobal =
      new llvm::GlobalVariable(*module, strType, true,
                               llvm::GlobalValue::PrivateLinkage, strValue,
                               "_str");
  llvm::Constant *zero = llvm::Constant::getNullValue(LLVM_INT);

  std::vector<llvm::Constant*> idx;
  idx.push_back(zero);
  idx.push_back(zero);
  return llvm::ConstantExpr::getGetElementPtr(nullptr, strGlobal, idx);
}

llvm::Function *LLVMBackend::emitEmptyFunction(const string &name,
                                               const vector<ir::Var> &arguments,
                                               const vector<ir::Var> &results,
                                               bool externalLinkage,
                                               bool doesNotThrow,
                                               bool scalarsByValue) {
  llvm::Function *llvmFunc = createPrototype(name, arguments, results, module,
                                             externalLinkage, doesNotThrow,
                                             scalarsByValue);
  auto entry = llvm::BasicBlock::Create(LLVM_CTX, "entry", llvmFunc);
  builder->SetInsertPoint(entry);

  iassert(llvmFunc->getArgumentList().size() == arguments.size()+results.size())
      << "Number of arguments to llvm func does not match simit func arguments";

  // Add arguments and results to symbol table
  auto llvmArgIt = llvmFunc->getArgumentList().begin();
  auto simitArgIt = arguments.begin();
  for (; simitArgIt < arguments.end(); ++simitArgIt, ++llvmArgIt) {
    symtable.insert(*simitArgIt, llvmArgIt);
  }

  auto simitResIt = results.begin();
  for (; simitResIt < results.end(); ++simitResIt, ++llvmArgIt) {
    symtable.insert(*simitResIt, llvmArgIt);
  }

  return llvmFunc;
}

void LLVMBackend::emitPrintf(llvm::Value *str, std::vector<llvm::Value*> args) {
  llvm::Function *printfFunc = module->getFunction("printf");
  if (printfFunc == nullptr) {
    std::vector<llvm::Type*> printfArgTypes;
    printfArgTypes.push_back(llvm::Type::getInt8PtrTy(LLVM_CTX));
    llvm::FunctionType* printfType = llvm::FunctionType::get(LLVM_INT,
                                                             printfArgTypes,
                                                             true);
    printfFunc = llvm::Function::Create(printfType,
                                        llvm::Function::ExternalLinkage,
                                        llvm::Twine("printf"), module);
    printfFunc->setCallingConv(llvm::CallingConv::C);
  }

  // Split any complex structs into two doubles
  for (size_t i = 0; i < args.size(); ++i) {
    if (args[i]->getType()->isStructTy()) {
      llvm::Value *real = builder->ComplexGetReal(args[i]);
      llvm::Value *imag = builder->ComplexGetImag(args[i]);
      args[i] = real;
      args.insert(args.begin()+i+1, imag);
    }
  }

  std::vector<llvm::Value*> printfArgs;
  for (size_t i = 0; i < args.size(); i++) {
    // printf requires float varargs be promoted to doubles!
    if (args[i]->getType()->isFloatTy()) {
      args[i] = builder->CreateFPCast(args[i], LLVM_DOUBLE);
    }
  }
  printfArgs.push_back(str);
  printfArgs.insert(printfArgs.end(), args.begin(), args.end());

  builder->CreateCall(printfFunc, printfArgs);
}

void LLVMBackend::emitPrintf(std::string format,
                             std::vector<llvm::Value*> args) {
  emitPrintf(emitGlobalString(format), args);
}

void LLVMBackend::emitGlobals(const ir::Environment& env, bool packed) {
  // Emit global constants
  // TODO

  // Emit global variables (externs and temporaries)
  for (const Var& ext : env.getExternVars()) {
    llvm::GlobalVariable* ptr = createGlobal(module, ext,
                                             llvm::GlobalValue::ExternalLinkage,
                                             globalAddrspace(), packed);
    this->symtable.insert(ext, ptr);
    this->globals.insert(ext);
  }

  // Emit global temporaries
  for (const Var& tmp : env.getTemporaries()) {
    llvm::GlobalVariable* ptr = createGlobal(module, tmp,
                                             llvm::GlobalValue::ExternalLinkage,
                                             globalAddrspace(), packed);
    this->symtable.insert(tmp, ptr);
    this->globals.insert(tmp);
  }

  // Emit global tensor indices
  for (const TensorIndex& tensorIndex : env.getTensorIndices()) {
    if (tensorIndex.getKind() == TensorIndex::PExpr) {
      const Var& rowptr = tensorIndex.getRowptrArray();
      llvm::GlobalVariable* rowptrPtr =
          createGlobal(module, rowptr, llvm::GlobalValue::ExternalLinkage,
                       globalAddrspace(), packed);
      this->symtable.insert(rowptr, rowptrPtr);
      this->globals.insert(rowptr);

      const Var& colidx  = tensorIndex.getColidxArray();
      llvm::GlobalVariable* colidxPtr =
          createGlobal(module, colidx, llvm::GlobalValue::ExternalLinkage,
                       globalAddrspace(), packed);
      this->symtable.insert(colidx, colidxPtr);
      this->globals.insert(colidx);
    }
  }
}

void LLVMBackend::emitAssign(Var var, const Expr& value) {
  /// \todo assignment of scalars to tensors and tensors to tensors should be
  ///       handled by the lowering so that we only assign scalars to scalars
  ///       in the backend. Probably requires copy and memset intrinsics.
//  iassert(isScalar(value.type()) &&
//         "assignment non-scalars should have been lowered by now");
  llvm::Value *valuePtr = compile(value);

  iassert(var.getType().isTensor() && value.type().isTensor());
  std::string varName = var.getName();
  iassert(symtable.contains(var)) << var << " has not been declared in:\n"
                                  << var << " = " << value << ";";

  llvm::Value *varPtr = symtable.get(var);
  iassert(varPtr->getType()->isPointerTy());

  // Globals are stored as pointer-pointers so we must load them
  if (util::contains(globals, var)) {
    varPtr = builder->CreateLoad(varPtr, var.getName());
  }

  const TensorType *varType = var.getType().toTensor();
  const TensorType *valType = value.type().toTensor();

  // Assigning a scalar to a scalar
  if (varType->order() == 0 && valType->order() == 0) {
    builder->CreateStore(valuePtr, varPtr);
    valuePtr->setName(varName + VAL_SUFFIX);
  }
  // Assign to n-order tensors
  else {
    iassert(storage.hasStorage(var)) << var << " has no storage";
    llvm::Value *len = emitComputeLen(varType, storage.getStorage(var));
    unsigned componentSize = varType->getComponentType().bytes();
    llvm::Value *size = builder->CreateMul(len, llvmInt(componentSize));

    // Assigning a scalar to an n-order tensor
    if (varType->order() > 0 && valType->order() == 0) {
      if (isa<Literal>(value)) {
        const ScalarType& sType = valType->getComponentType();
        // Assigning 0 to a tensor (memset)
        if ((sType.kind == ScalarType::Float &&
             to<Literal>(value)->getFloatVal(0) == 0.0) ||
            (sType.kind == ScalarType::Complex &&
             to<Literal>(value)->getComplexVal(0) == double_complex(0,0)) ||
            (sType.kind == ScalarType::Int &&
             ((int*)to<Literal>(value)->data)[0] == 0)) {
          emitMemSet(varPtr, llvmInt(0,8), size, componentSize);
        }
        else {
          not_supported_yet << "Cannot assign non-zero value to tensor:"
                            << std::endl
                            << var.getName() << " = " << value;
        }
      }
      // Assigning general scalar to a tensor
      else {
        not_supported_yet << "you can only currently assign a scalar to a"
                          << "tensor if the scalar is a literal 0:" << std::endl
                          << var.getName() << " = " << value;
      }
    }
    // Assign tensor to conforming tensor
    else {
      iassert(var.getType() == value.type())
          << "variable and value types don't match";
      emitMemCpy(varPtr, valuePtr, size, componentSize);
    }
  }
}

void LLVMBackend::emitMemCpy(llvm::Value *dst, llvm::Value *src,
                             llvm::Value *size, unsigned align) {
  builder->CreateMemCpy(dst, src, size, align);
}

void LLVMBackend::emitMemSet(llvm::Value *dst, llvm::Value *val,
                             llvm::Value *size, unsigned align) {
  builder->CreateMemSet(dst, val, size, align);
}

llvm::Value *LLVMBackend::makeGlobalTensor(ir::Var var) {
  // Allocate buffer for local variable in global storage.
  // TODO: We should allocate small local dense tensors on the stack
  iassert(var.getType().isTensor());
  llvm::Type *ctype = llvmType(var.getType().toTensor()->getComponentType());
  llvm::PointerType *globalType = llvm::PointerType::get(ctype, globalAddrspace());

  llvm::GlobalVariable* buffer =
      new llvm::GlobalVariable(*module, globalType,
                               false, llvm::GlobalValue::ExternalLinkage,
                               llvm::ConstantPointerNull::get(globalType),
                               var.getName(), nullptr,
                               llvm::GlobalVariable::NotThreadLocal,
                               globalAddrspace());
  buffer->setAlignment(8);
  buffers.insert(pair<Var, llvm::Value*>(var, buffer));

  // Add load to symtable
  return builder->CreateLoad(buffer, buffer->getName());
}

}}
