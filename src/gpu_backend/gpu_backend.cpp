#include "gpu_backend.h"

#include "llvm/ADT/StringMap.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"
#include "llvm/PassManager.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Transforms/IPO.h"
#include "llvm/Transforms/Scalar.h"

#include <fstream>

#include "error.h"
#include "gpu_codegen.h"
#include "gpu_function.h"
#include "ir.h"
#include "ir_queries.h"
#include "llvm_codegen.h"
#include "types.h"

#ifndef NASSERT
#define ASSERT(cond) \
  do { \
    if (!(cond)) { \
      std::cerr << "Assert error: " << __FILE__ << ":" << __LINE__ << std::endl; \
      exit(1); \
    } \
  } while (0)
#else
#define ASSERT(cond) do { (void)sizeof(cond); } while (0)
#endif

namespace simit {
namespace internal {

simit::Function *GPUBackend::compile(simit::ir::Func irFunc) {
  std::ofstream irFile("simit.sim", std::ofstream::trunc);
  irFile << irFunc;
  irFile.close();

  this->irFunc = irFunc;
  this->module = createNVVMModule("kernels-module");
  this->dataLayout.reset(new llvm::DataLayout(module));

  std::vector<ir::Func> callTree = ir::getCallTree(irFunc);
  std::reverse(callTree.begin(), callTree.end());

  this->storage = ir::Storage();
  symtable.clear();
  buffers.clear();

  for (auto &f : callTree) {
    // If we're not compiling the top-level func, then we do regular stack
    // allocations.
    inKernel = (f.getName() != irFunc.getName());
    if (f.getKind() != ir::Func::Internal) continue;
    iassert(f.getBody().defined());

    this->storage.add(f.getStorage());

    // Emit function
    func = emitEmptyFunction(f.getName(), f.getArguments(), f.getResults(),
                             false, false);

    // Add constants to symbol table
    for (auto &global : f.getEnvironment().globals) {
      symtable.insert(global.first, compile(global.second));
    }

    f.getBody().accept(this);
    builder->CreateRetVoid();
    symtable.clear();
  }
  iassert(func);
  exported.push_back(func->getName().data());

  // Run LLVM/NVVM optimization passes on the function
  llvm::FunctionPassManager fpm(module);
  fpm.add(new llvm::DataLayout(*dataLayout));

  // Internalize pass
  fpm.add(llvm::createInternalizePass(llvm::ArrayRef<const char*>(exported)));
  fpm.add(llvm::createGVNPass());
  fpm.add(llvm::createPromoteMemoryToRegisterPass());

  return new GPUFunction(irFunc, func, module, buffers, storage);
}

llvm::Value *GPUBackend::compile(const ir::Expr &expr) {
  expr.accept(this);
  llvm::Value *tmp = val;
  val = nullptr;
  return tmp;
}

void GPUBackend::visit(const ir::FieldRead *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::TensorRead *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::TupleRead *op) {
  std::cerr << "GPUBackend::visit unsupported node:\n\n" << *op << "\n";
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::IndexRead *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Length *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Map *op) {
  std::cerr << "GPUBackend::visit unsupported node:\n\n" << *op << "\n";
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::IndexedTensor *op) {
  std::cerr << "GPUBackend::visit unsupported node:\n\n" << *op << "\n";
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::IndexExpr *op) {
  std::cerr << "GPUBackend::visit unsupported node:\n\n" << *op << "\n";
  ASSERT(false && "No code generation for this type");
}
void GPUBackend::visit(const ir::TensorWrite *op) {
  LLVMBackend::visit(op);
}

void GPUBackend::visit(const ir::Literal *op) {
  const ir::TensorType *type = op->type.toTensor();
  if (type->order() == 0) {
    // Delegate scalar literals to generic LLVM backend
    LLVMBackend::visit(op);
  }
  else {
    // Put the data in global memory and generate a pointer
    ir::ScalarType ctype = type->componentType;
    llvm::Constant *dataConstant;
    switch (ctype.kind) {
      case ir::ScalarType::Int: {
        iassert(ctype.bytes() == sizeof(uint32_t))
            << "Incorrect native types used for constant data array";
        iassert(op->size % sizeof(uint32_t) == 0)
            << "Literal data size not a multiple of element size";
        dataConstant = llvm::ConstantDataArray::get(
            LLVM_CONTEXT, llvm::ArrayRef<uint32_t>(
                reinterpret_cast<uint32_t*>(op->data),
                op->size/sizeof(uint32_t)));
        break;
      }
      case ir::ScalarType::Float: {
        if (ir::ScalarType::floatBytes == sizeof(float)) {
          iassert(op->size % sizeof(float) == 0)
              << "Literal data size not a multiple of element size";
          dataConstant = llvm::ConstantDataArray::get(
              LLVM_CONTEXT, llvm::ArrayRef<float>(
                  reinterpret_cast<float*>(op->data),
                  op->size/sizeof(float)));
        }
        else if (ir::ScalarType::floatBytes == sizeof(double)) {
          iassert(op->size % sizeof(double) == 0)
              << "Literal data size not a multiple of element size";
          dataConstant = llvm::ConstantDataArray::get(
              LLVM_CONTEXT, llvm::ArrayRef<double>(
                  reinterpret_cast<double*>(op->data),
                  op->size/sizeof(double)));
        }
        else {
          unreachable;
        }
        break;
      }
      case ir::ScalarType::Boolean: {
        not_supported_yet;
        // This code is untested, but likely correct
        iassert(op->size % sizeof(bool) == 0)
            << "Literal data size not a multiple of element size";
        iassert(sizeof(bool) == sizeof(uint32_t))
            << "Boolean literal assumes 32-bit data format";
        dataConstant = llvm::ConstantDataArray::get(
            LLVM_CONTEXT, llvm::ArrayRef<uint32_t>(
                reinterpret_cast<uint32_t*>(op->data),
                op->size/sizeof(uint32_t)));
        break;
      }
      default: unreachable;
    }
    llvm::GlobalVariable *globalData =
        new llvm::GlobalVariable(*module, dataConstant->getType(), true,
                                 llvm::GlobalVariable::InternalLinkage,
                                 dataConstant, "global_const", nullptr,
                                 llvm::GlobalVariable::NotThreadLocal,
                                 CUDA_GLOBAL_ADDRSPACE);
    llvm::Type *finalType = createLLVMType(type, CUDA_GLOBAL_ADDRSPACE);
    val = builder->CreateBitCast(globalData, finalType);
  }
  iassert(val);
}
void GPUBackend::visit(const ir::VarExpr *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Load *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::CallStmt *op) {
  std::map<ir::Func, std::string> nvvmIntrinsicByName =
                                  {{ir::Intrinsics::sin,    std::string("__nv_sinf")},
                                   {ir::Intrinsics::cos,    std::string("__nv_cosf")},
                                   {ir::Intrinsics::sqrt,   std::string("__nv_sqrtf")},
                                   {ir::Intrinsics::log,    std::string("__nv_logf")},
                                   {ir::Intrinsics::exp,    std::string("__nv_fast_expf")},
                                   {ir::Intrinsics::pow,    std::string("__nv_fast_powf")},
                                   {ir::Intrinsics::atan2,  std::string("__nv_atan2f")},
                                   {ir::Intrinsics::tan,    std::string("__nv_tanf")},
                                   {ir::Intrinsics::asin,   std::string("__nv_asinf")},
                                   {ir::Intrinsics::acos,   std::string("__nv_acosf")}};
  
  std::vector<llvm::Type*> argTypes;
  std::vector<llvm::Value*> args;
  llvm::Function *fun = nullptr;
  llvm::Value *call = nullptr;

  // compile arguments first
  for (auto a: op->actuals) {
    //FIX: remove once solve() is no longer needed
    //iassert(isScalar(a.type()));
    argTypes.push_back(createLLVMType(a.type().toTensor()->componentType));
    args.push_back(compile(a));
  }

  ir::Func callee = op->callee;

  if (callee.getKind() == ir::Func::Intrinsic) {
    std::string floatTypeName = ir::ScalarType::singleFloat() ? "_f32" : "_f64";

    // first, see if this is an LLVM intrinsic
    auto foundIntrinsic = nvvmIntrinsicByName.find(callee);
    if (foundIntrinsic != nvvmIntrinsicByName.end()) {
      fun = getBuiltIn(foundIntrinsic->second, getLLVMFloatType(), argTypes);
      call = builder->CreateCall(fun, args);
      // Export the intrinsic, so it's not optimized out of the module
      exported.push_back(fun->getName().data());
    }
    else if (callee == ir::Intrinsics::mod) {
      iassert(op->actuals.size() == 2) << "mod takes two inputs, got"
                                       << op->actuals.size();
      call = builder->CreateSRem(compile(op->actuals[0]),
                                 compile(op->actuals[1]));
    }
    else if (callee == ir::Intrinsics::det) {
      iassert(args.size() == 1);
      std::string fname = callee.getName() + "3" + floatTypeName;
      call = emitCall(fname, args, getLLVMFloatType());
    }
    else if (callee == ir::Intrinsics::norm) {
      iassert(args.size() == 1);
      auto type = op->actuals[0].type().toTensor();
      
      // Dense operation
      if (!type->isSparse()) {
        args.push_back(emitComputeLen(type->dimensions[0]));
        std::string funcName = callee.getName() + floatTypeName;
        call = emitCall(funcName, args, getLLVMFloatType());
      }
      else {
        // Fire off kernel for sparse operation
        llvm::Value *llvmResult = symtable.get(op->results[0]);
        llvm::Value *size = emitComputeLen(type->dimensions[0]);
        emitShardedDot(op->actuals[0].type(), op->actuals[0].type(),
                       op->results[0].getType(), args[0], args[0],
                       size, llvmResult);
        llvm::Value *sqrt = getBuiltIn(
            nvvmIntrinsicByName.at(ir::Intrinsics::sqrt),
            getLLVMFloatType(), { getLLVMFloatType() });
        call = builder->CreateCall(sqrt, builder->CreateLoad(llvmResult));
      }
    }
    else if (callee == ir::Intrinsics::inv) {
      iassert(args.size() == 1);

      ir::Var result = op->results[0];
      llvm::Value *llvmResult = symtable.get(result);
      args.push_back(llvmResult);

      std::string fname = callee.getName() + "3" + floatTypeName;
      call = emitCall(fname, args);
    }
    else if (op->callee == ir::Intrinsics::loc) {
      call = emitCall("loc", args, LLVM_INT);
    }
    else if (callee == ir::Intrinsics::dot) {
      iassert(args.size() == 2);
      // we need to add the vector length to the args
      auto type1 = op->actuals[0].type().toTensor();
      auto type2 = op->actuals[1].type().toTensor();
      uassert(type1->dimensions[0] == type2->dimensions[0]) <<
          "dimension mismatch in dot product";

      // Dense operation
      if (!type1->isSparse() && !type2->isSparse()) {
        args.push_back(emitComputeLen(type1->dimensions[0]));
        std::string funcName = callee.getName() + floatTypeName;
        call = emitCall(funcName, args, getLLVMFloatType());
      }
      else {
        // Fire off kernel for sparse operation
        iassert(type1->isSparse() && type2->isSparse());

        llvm::Value *llvmResult = symtable.get(op->results[0]);
        llvm::Value *size = emitComputeLen(type1->dimensions[0]);
        emitShardedDot(op->actuals[0].type(), op->actuals[1].type(),
                       op->results[0].getType(), args[0], args[1],
                       size, llvmResult);
        call = builder->CreateLoad(llvmResult);
      }
    }
    else {
      ierror << "intrinsic " << op->callee.getName() << " not found";
    }
  
    iassert(call);
    if (!call->getType()->isVoidTy()) {
      iassert(op->results.size() == 1);
      ir::Var var = op->results[0];
      llvm::Value *llvmVar = symtable.get(var);
      builder->CreateStore(call, llvmVar);
    }
  }
  // if not an intrinsic function, try to find it in the module
  else {
    if (module->getFunction(callee.getName())) {
      for (ir::Var r : op->results) {
        argTypes.push_back(createLLVMType(r.getType().toTensor()->componentType));

        llvm::Value *llvmResult = symtable.get(r);
        args.push_back(llvmResult);
        symtable.insert(r, llvmResult);
      }
      fun = module->getFunction(op->callee.getName());
      call = builder->CreateCall(fun, args);
    }
    else {
      ierror << "function " << op->callee.getName() << " not found in module";
    }
  }
}
void GPUBackend::visit(const ir::Call *op) {
  std::map<ir::Func, std::string> nvvmIntrinsicByName =
                                  {{ir::Intrinsics::sin,    std::string("__nv_sinf")},
                                   {ir::Intrinsics::cos,    std::string("__nv_cosf")},
                                   {ir::Intrinsics::sqrt,   std::string("__nv_sqrtf")},
                                   {ir::Intrinsics::log,    std::string("__nv_logf")},
                                   {ir::Intrinsics::exp,    std::string("__nv_fast_expf")},
                                   {ir::Intrinsics::pow,    std::string("__nv_fast_powf")},
                                   {ir::Intrinsics::atan2,  std::string("__nv_atan2f")},
                                   {ir::Intrinsics::tan,    std::string("__nv_tanf")},
                                   {ir::Intrinsics::asin,   std::string("__nv_asinf")},
                                   {ir::Intrinsics::acos,   std::string("__nv_acosf")}};
  
  std::vector<llvm::Type*> argTypes;
  std::vector<llvm::Value*> args;
  llvm::Function *fun = nullptr;

  // compile arguments first
  for (auto a: op->actuals) {
    //FIX: remove once solve() is no longer needed
    //iassert(isScalar(a.type()));
    argTypes.push_back(createLLVMType(a.type().toTensor()->componentType));
    args.push_back(compile(a));
  }

  auto foundIntrinsic = nvvmIntrinsicByName.find(op->func);
  if (foundIntrinsic != nvvmIntrinsicByName.end()) {
    auto ftype = llvm::FunctionType::get(getLLVMFloatType(), argTypes, false);
    module->getOrInsertFunction(foundIntrinsic->second, ftype);
    fun = module->getFunction(foundIntrinsic->second);
    // Export the intrinsic, so it's not optimized out of the module
    exported.push_back(fun->getName().data());
  }
  else if (op->func == ir::Intrinsics::norm) {
    iassert(args.size() == 1);
    auto type = op->actuals[0].type().toTensor();

    // Dense operation
    if (!type->isSparse()) {
      args.push_back(emitComputeLen(type->dimensions[0]));
      std::string funcName = ir::ScalarType::singleFloat() ?
          "norm_f32" : "norm_f64";
      val = emitCall(funcName, args, getLLVMFloatType());
    }
    else {
      // Fire off kernel for sparse computation
      llvm::Value *result = builder->CreateAlloca(
          getLLVMFloatType(), llvmInt(1));
      llvm::Value *size = emitComputeLen(type->dimensions[0]);
      ir::Type resultType = ir::TensorType::make(type->componentType);
      emitShardedDot(op->actuals[0].type(), op->actuals[0].type(),
                     resultType, args[0], args[0], size, result);
      llvm::Value *sqrt = getBuiltIn(
          nvvmIntrinsicByName.at(ir::Intrinsics::sqrt),
          getLLVMFloatType(), { getLLVMFloatType() });
      val = builder->CreateCall(sqrt, builder->CreateLoad(result));
    }
    return;
  }
  else if (op->func == ir::Intrinsics::loc) {
    val = emitCall("loc", args, LLVM_INT);
    return;
  }
  else if (op->func == ir::Intrinsics::dot) {
    iassert(args.size() == 2);
    // we need to add the vector length to the args
    auto type1 = op->actuals[0].type().toTensor();
    auto type2 = op->actuals[1].type().toTensor();
    uassert(type1->dimensions[0] == type2->dimensions[0]) <<
      "dimension mismatch in dot product";

    // Dense operation
    if (!type1->isSparse() && !type2->isSparse()) {
      std::string funcName = ir::ScalarType::singleFloat() ?
          "dot_f32" : "dot_f64";
      args.push_back(emitComputeLen(type1->dimensions[0]));
      val = emitCall(funcName, args, getLLVMFloatType());
      return;
    }

    // Fallthrough: fire off a kernel for sparse operation
    iassert(type1->isSparse() && type2->isSparse());

    llvm::Value *result = builder->CreateAlloca(getLLVMFloatType(), llvmInt(1));
    llvm::Value *size = emitComputeLen(type1->dimensions[0]);
    ir::Type resultType = ir::TensorType::make(type1->componentType);
    emitShardedDot(op->actuals[0].type(), op->actuals[1].type(),
                   resultType, args[0], args[1],
                   size, result);
    val = result;

    return;
  }
  // if not an intrinsic function, try to find it in the module
  else if (module->getFunction(op->func.getName())) {
    fun = module->getFunction(op->func.getName());
  }
  else {
    std::cerr << "GPUBackend::visit unsupported node:\n\n" << *op << "\n\n";
    ASSERT(false && "No code generation for this type");
  }
  
  val = builder->CreateCall(fun, args);
}
void GPUBackend::visit(const ir::Neg *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Add *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Sub *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Mul *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Div *op) {
  LLVMBackend::visit(op);
}

void GPUBackend::visit(const ir::VarDecl *op) {
  tassert(op->var.getType().isTensor()) << "Only tensor decls supported";

  if (inKernel) {
    ir::Var var = op->var;
    if (isScalar(var.getType())) {
      // Allow LLVMBackend to emit a local alloca
      LLVMBackend::visit(op);
    }
    else {
      const ir::TensorType *ttype = var.getType().toTensor();
      ir::ScalarType ctype = ttype->componentType;
      llvm::Value *llvmVar = builder->CreateAlloca(
          createLLVMType(ctype), llvmInt(ttype->size()), var.getName());
      symtable.insert(var, llvmVar);
    }
  }
  else { // Root scope
    // Always global, to be accessible to all kernels
    makeGlobalTensor(op->var);
  }
}

void GPUBackend::visit(const ir::AssignStmt *op) {
  // Only atomic for a compound scalar-scalar assign
  const ir::TensorType *varType = op->var.getType().toTensor();
  const ir::TensorType *valType = op->value.type().toTensor();
  if (op->cop.kind != ir::CompoundOperator::None &&
      varType->order() == 0) {
    iassert(symtable.contains(op->var)) << op->var << " has not been declared";
    switch (op->cop.kind) {
      case ir::CompoundOperator::Add: {
        llvm::Value *value = compile(op->value);
        llvm::Value *varPtr = symtable.get(op->var);
        // Guard against non-pointer
        iassert(varPtr->getType()->isPointerTy());
        if (buffers.find(op->var) != buffers.end()) {
          // Global or argument which might be accessed in parallel
          emitAtomicLoadAdd(varPtr, value);
        }
        else {
          // Local, will not be accessed in parallel
          LLVMBackend::visit(op);
        }
        break;
      }
      default: ierror << "Unknown compound operator type: " << op->cop.kind;
    }
  }
  else if (varType->order() > 0 && valType->order() == 0 &&
           ir::isa<ir::Literal>(op->value) &&
           (ir::to<ir::Literal>(op->value)->getFloatVal(0) == 0.0 ||
            ((int*)ir::to<ir::Literal>(op->value))[0] == 0) &&
           !inKernel) {
    llvm::Value *varPtr = symtable.get(op->var);
    llvm::Value *len = emitComputeLen(varType, storage.get(op->var));
    emitShardedMemSet(op->var.getType(), varPtr, len);
  }
  else {
    LLVMBackend::visit(op);
  }
}
void GPUBackend::visit(const ir::FieldWrite *op) {
  // Sparse memset 0 should be emitted as a kernel
  ir::Type fieldType = getFieldType(op->elementOrSet, op->fieldName);
  ir::Type valueType = op->value.type();
  if (fieldType.toTensor()->order() > 0 &&
      valueType.toTensor()->order() == 0 &&
      ir::isa<ir::Literal>(op->value) &&
      ir::to<ir::Literal>(op->value)->getFloatVal(0) == 0.0) {
    llvm::Value *fieldPtr = emitFieldRead(op->elementOrSet, op->fieldName);
    // For now we'll assume fields are always dense row major
    emitShardedMemSet(fieldType, fieldPtr,
                      emitComputeLen(fieldType.toTensor(), ir::TensorStorage::DenseRowMajor));
  }
  else {
    LLVMBackend::visit(op);
  }
}
void GPUBackend::visit(const ir::Store *op) {
  if (op->cop.kind != ir::CompoundOperator::None) {
    llvm::Value *buffer = compile(op->buffer);
    llvm::Value *index = compile(op->index);
    llvm::Value *value = compile(op->value);
    std::string locName = std::string(buffer->getName()) + PTR_SUFFIX;
    llvm::Value *bufferLoc = builder->CreateInBoundsGEP(buffer, index, locName);
    switch (op->cop.kind) {
      case ir::CompoundOperator::Add: {
        emitAtomicLoadAdd(bufferLoc, value);
        break;
      }
      default: ierror << "Unknown compound operator type";
    }
  }
  else {
    LLVMBackend::visit(op);
  }
}
void GPUBackend::visit(const ir::ForRange *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::For *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::GPUKernel *op) {
  // Stash the symtable
  ScopedMap<simit::ir::Var, llvm::Value*> oldSymtable = symtable;
  symtable.clear();
  // Stash the current basic block
  llvm::BasicBlock *prevBB = builder->GetInsertBlock();

  // Pass all globals as arguments
  std::vector<ir::Var> kernelArgs = irFunc.getArguments();
  for (auto &kv : buffers) {
    kernelArgs.push_back(kv.first);
  }

  // Create LLVM func
  llvm::Function *kernel = emitEmptyFunction(
      irFunc.getName() + "_nested_kernel", kernelArgs,
      irFunc.getResults(), true, false, false);
  builder->SetInsertPoint(&kernel->getEntryBlock());
  
  // Parameter attributes
  llvm::AttributeSet attrSet = kernel->getAttributes();
  for (int slot = 0; slot < attrSet.getNumSlots(); ++slot) {
    int index = attrSet.getSlotIndex(slot);
    attrSet = attrSet.addAttribute(LLVM_CONTEXT, index, llvm::Attribute::NoAlias);
  }
  kernel->setAttributes(attrSet);
  
  llvm::BasicBlock *bodyStart = llvm::BasicBlock::Create(
    LLVM_CONTEXT, "bodyStart", kernel);
  llvm::BasicBlock *earlyExit = llvm::BasicBlock::Create(
    LLVM_CONTEXT, "earlyExit", kernel);
  
  // Guard: check if we're outside the intended range of the kernel loop and 
  // early-exit if so.
  GPUSharding kernelSharding = op->sharding;
  llvm::Value *cond = builder->CreateICmpULT(getTidX(),
    emitComputeLen(kernelSharding.xDomain));
  builder->CreateCondBr(cond, bodyStart, earlyExit);

  builder->SetInsertPoint(earlyExit);
  builder->CreateRetVoid();

  // Continue with kernel body
  builder->SetInsertPoint(bodyStart);

  // Kernel metadata
  addNVVMAnnotation(kernel, "kernel", llvmInt(1), module);

  // Code generate for the kernel
  if (kernelSharding.xSharded) {
    symtable.insert(kernelSharding.xVar, getTidX());
  }
  if (kernelSharding.ySharded) {
    symtable.insert(kernelSharding.yVar, getTidY());
  }
  if (kernelSharding.zSharded) {
    symtable.insert(kernelSharding.zVar, getTidZ());
  }
  
  inKernel = true;
  LLVMBackend::compile(op->body);
  inKernel = false;

  // NVVM kernel should always return void
  builder->CreateRetVoid();

  // Unstash symtable
  symtable = oldSymtable;

  // Emit a dynamic kernel launch
  builder->SetInsertPoint(prevBB);
  std::vector<llvm::Value*> args;
  for (auto &irArg : kernelArgs) {
    args.push_back(symtable.get(irArg));
  }
  for (auto &irRes : irFunc.getResults()) {
    // TODO(gkanwar): Figure out inouts
    args.push_back(symtable.get(irRes));
  }
  emitKernelLaunch(kernel, args, kernelSharding);
}

void GPUBackend::visit(const ir::IfThenElse *op) {
  // ASSERT(false && "No code generation for this type");
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Block *op) {
  LLVMBackend::visit(op);
}
void GPUBackend::visit(const ir::Pass *op) {
  // ASSERT(false && "No code generation for this type");
  LLVMBackend::visit(op);
}

namespace {

// TODO(gkanwar): Do we need to clean attrs now that we are passing in BC?
void cleanFuncAttrs(llvm::Function *func) {
  // Clean attributes off of params
  llvm::AttributeSet funcAttrs = func->getAttributes();
  llvm::AttributeSet cleanAttrs;
  for (unsigned slot = 0; slot < funcAttrs.getNumSlots(); ++slot) {
    // Never add func attributes, because attribute groups are
    // disallowed in NVVM. If left on, they trip up the parser
    if (slot == 0) continue;
    // Remove readonly from param attrs
    int index = funcAttrs.getSlotIndex(slot);
    llvm::AttributeSet cleanSlot = funcAttrs.removeAttribute(
        LLVM_CONTEXT, index, llvm::Attribute::ReadOnly);
    cleanAttrs.addAttributes(LLVM_CONTEXT, index, cleanSlot);
  }

  func->setAttributes(cleanAttrs);
}

}

llvm::Value *GPUBackend::emitBarrier() {
  llvm::Function *func = getBuiltIn("llvm.nvvm.barrier0", LLVM_VOID, {});
  cleanFuncAttrs(func);
  return builder->CreateCall(func);
}

llvm::Value *GPUBackend::emitCheckRoot() {
  not_supported_yet;
  assert(false && "unreachable");
  return NULL;
}

llvm::Value *GPUBackend::getTidX() {
  llvm::Function *tidFunc = getBuiltIn(
      "llvm.nvvm.read.ptx.sreg.tid.x", LLVM_INT, {});
  cleanFuncAttrs(tidFunc);
  llvm::Function *bidFunc = getBuiltIn(
      "llvm.nvvm.read.ptx.sreg.ctaid.x", LLVM_INT, {});
  cleanFuncAttrs(bidFunc);
  auto tid = builder->CreateCall(tidFunc);
  auto bid = builder->CreateCall(bidFunc);
  auto blockOffset = builder->CreateMul(bid, llvmInt(blockSize));
  return builder->CreateAdd(tid, blockOffset);
}

llvm::Value *GPUBackend::getTidY() {
  not_supported_yet; // these should never be emitted at this point
}

llvm::Value *GPUBackend::getTidZ() {
  not_supported_yet; // these should never be emitted at this point
}

llvm::Value *GPUBackend::emitCastGlobalToGen(llvm::Value *src) {
  iassert(src->getType()->isPointerTy());
  llvm::PointerType *srcPtrTy = llvm::cast<llvm::PointerType>(src->getType());
  iassert(srcPtrTy->getAddressSpace() ==
          CUDA_GLOBAL_ADDRSPACE);
  llvm::Value *srcCast = builder->CreateBitCast(src, LLVM_INT8PTR_GLOBAL);
  llvm::Function *castFunc = getBuiltIn(
      "llvm.nvvm.ptr.global.to.gen.p0i8.p1i8",
      LLVM_INT8PTR, { LLVM_INT8PTR_GLOBAL });
  cleanFuncAttrs(castFunc);
  llvm::Value *out = builder->CreateCall(castFunc, srcCast);
  llvm::Type *genTy = llvm::PointerType::getUnqual(srcPtrTy->getElementType());
  return builder->CreateBitCast(out, genTy);
}

void GPUBackend::emitThreadBarrier() {
  llvm::Function *func = getBuiltIn("llvm.nvvm.barrier0", LLVM_VOID, {});
  cleanFuncAttrs(func);
  builder->CreateCall(func);
}

void GPUBackend::emitDeviceSync() {
  llvm::Function *syncFunc = getBuiltIn("cudaDeviceSynchronize", LLVM_INT, {});
  builder->CreateCall(syncFunc);
}

void GPUBackend::emitAtomicLoadAdd(llvm::Value *ptr, llvm::Value *value) {
  if (value->getType()->isIntegerTy()) {
    builder->CreateAtomicRMW(llvm::AtomicRMWInst::Add, ptr, value,
                             llvm::AtomicOrdering::Monotonic);
  }
  else if (value->getType()->isFloatTy()) {
    emitAtomicFLoadAdd(ptr, value);
  }
  else {
    ierror << "Unknown LLVM value type for atomic load add";
  }
}

void GPUBackend::emitAtomicFLoadAdd(llvm::Value *ptr, llvm::Value *value) {
  llvm::Type *ptrGenTy = ptr->getType();
  iassert(ptrGenTy->isPointerTy())
      << "Atomic float load add requires pointer type for ptr";
  llvm::PointerType *ptrTy = reinterpret_cast<llvm::PointerType*>(ptrGenTy);
  unsigned addrspace = ptrTy->getAddressSpace();
  std::vector<llvm::Type*> argTys;
  std::string funcName;
  switch (addrspace) {
    case CUDA_GENERIC_ADDRSPACE: {
      argTys.push_back(LLVM_FLOATPTR);
      argTys.push_back(LLVM_FLOAT);
      funcName = "llvm.nvvm.atomic.load.add.f32.p0f32";
      break;
    }
    case CUDA_GLOBAL_ADDRSPACE: {
      argTys.push_back(LLVM_FLOATPTR_GLOBAL);
      argTys.push_back(LLVM_FLOAT);
      funcName = "llvm.nvvm.atomic.load.add.f32.p1f32";
      break;
    }
    case CUDA_SHARED_ADDRSPACE: {
      argTys.push_back(llvm::Type::getFloatPtrTy(LLVM_CONTEXT, addrspace));
      argTys.push_back(LLVM_FLOAT);
      funcName = "llvm.nvvm.atomic.load.add.f32.p3f32";
      break;
    }
    default:
      ierror << "Unsupported addrspace for float load/add: " << addrspace;
  }
  llvm::Function *func = getBuiltIn(funcName, LLVM_FLOAT, argTys);
  cleanFuncAttrs(func);
  builder->CreateCall2(func, ptr, value);
}

void GPUBackend::emitKernelLaunch(llvm::Function *kernel,
                                  std::vector<llvm::Value*> args,
                                  GPUSharding sharding) {
  iassert(sharding.xSharded && !sharding.ySharded && !sharding.zSharded);
  emitKernelLaunch(kernel, args,
                   emitComputeLen(sharding.xDomain), nullptr, nullptr);
}

void GPUBackend::emitKernelLaunch(llvm::Function *kernel,
                                  std::vector<llvm::Value*> args,
                                  llvm::Value *xSize,
                                  llvm::Value *ySize,
                                  llvm::Value *zSize) {
  iassert(xSize) << "x dimension must be non-null";
  iassert(!ySize && !zSize) << "y and z dimensions not currently supported";

  // LLVM types
  // struct dim3
  llvm::StructType *dim3Ty = getOrCreateDim3Ty();

  // cudaGetParamBufferV2
  std::vector<llvm::Type*> getParamArgTys = {
    LLVM_INT8PTR, dim3Ty, dim3Ty, LLVM_INT
  };
  llvm::Function *getParamFunc = getBuiltIn(
      "cudaGetParameterBufferV2", LLVM_INT8PTR, getParamArgTys);

  // CUstream_st
  llvm::PointerType *cuStreamPtrTy = getOrCreateCUStreamPtrTy();

  // cudaLaunchDeviceV2
  std::vector<llvm::Type*> launchDevArgTys = {
    LLVM_INT8PTR, cuStreamPtrTy
  };
  llvm::Function *cudaLaunchFunc = getBuiltIn(
      "cudaLaunchDeviceV2", LLVM_INT, launchDevArgTys);

  // Build dimensions
  std::vector<llvm::Constant*> gridDimsVec = {
    llvmInt(1), llvmInt(1), llvmInt(1)
  };
  llvm::Value *gridDims =
      llvm::ConstantStruct::get(dim3Ty, gridDimsVec);
  
  // numBlocks = 1 + ( (len-1) / blockSize )
  llvm::Value *numBlocks =  builder->CreateAdd(
                              builder->CreateUDiv(
                                builder->CreateSub(
                                  xSize,
                                  llvmInt(1)),
                                llvmInt(blockSize)
                              ), llvmInt(1)
                            );
  gridDims = builder->CreateInsertValue(
      gridDims,
      numBlocks,
      llvm::ArrayRef<unsigned>({0}));

  std::vector<llvm::Constant*> initBlockDims = {
    llvmInt(blockSize), llvmInt(1), llvmInt(1)
  };
  llvm::Constant *blockDims =
      llvm::ConstantStruct::get(dim3Ty, initBlockDims);

  // Build param buffer
  llvm::Value *kernelBitCast = builder->CreateBitCast(kernel, LLVM_INT8PTR);
  llvm::Value *paramBuf = builder->CreateCall4(
      getParamFunc, kernelBitCast, gridDims, blockDims, llvmInt(0));

  // Insert args into param buffer
  emitFillBuf(paramBuf, args);

  builder->CreateCall2(cudaLaunchFunc, paramBuf,
                       llvm::ConstantPointerNull::get(cuStreamPtrTy));

  // Synchronize memory after the call
  emitDeviceSync();
}

void GPUBackend::emitPrintf(std::string format,
                            std::vector<llvm::Value*> args) {
  format = "(%d) " + format; // add thread ID
  llvm::Value *formatPtr = emitGlobalString(format);
  
  // add thread ID to beginning:
  std::reverse(args.begin(), args.end());
  args.push_back(getTidX());
  std::reverse(args.begin(), args.end());
  
  // Convert any args that need to be extended
  for (size_t i = 0; i < args.size(); ++i) {
    auto &arg = args[i];
    if (arg->getType()->isFloatTy()) {
      args[i] = builder->CreateFPExt(arg, LLVM_DOUBLE);
    }
    else if (arg->getType()->isIntegerTy()) {
      unsigned width = arg->getType()->getIntegerBitWidth();
      if (width < 32) {
        args[i] = builder->CreateSExt(arg, LLVM_INT);
      }
    }
  }

  // Alloc args buf
  size_t size = 0;
  for (auto &arg : args) {
    size += dataLayout->getTypeAllocSize(arg->getType());
  }

  llvm::AllocaInst *argBuf =
      builder->CreateAlloca(LLVM_INT8, llvmInt(size), "buffer");
  // Align 8, so vprintf will be happy
  argBuf->setAlignment(8);
  emitFillBuf(argBuf, args);

  // Create and call vprintf syscall
  llvm::Function *vprintf = getBuiltIn(
      "vprintf", LLVM_INT, {LLVM_INT8PTR, LLVM_INT8PTR});

  builder->CreateCall2(vprintf, formatPtr, argBuf);
}

void GPUBackend::emitMemCpy(llvm::Value *dst, llvm::Value *src,
                            llvm::Value *size, unsigned align) {
  iassert(dst->getType()->isPointerTy());
  iassert(src->getType()->isPointerTy());


  unsigned dstAddrspace = llvm::cast<llvm::PointerType>(
      dst->getType())->getAddressSpace();
  llvm::Type *dstCastTy;
  std::string dstTyStr;
  if (dstAddrspace == CUDA_GLOBAL_ADDRSPACE) {
    dstCastTy = LLVM_INT8PTR_GLOBAL;
    dstTyStr = "p1i8";
  }
  else if (dstAddrspace == CUDA_GENERIC_ADDRSPACE) {
    dstCastTy = LLVM_INT8PTR;
    dstTyStr = "p0i8";
  }
  else {
    not_supported_yet;
  }

  unsigned srcAddrspace = llvm::cast<llvm::PointerType>(
      src->getType())->getAddressSpace();
  llvm::Type *srcCastTy;
  std::string srcTyStr;
  if (srcAddrspace == CUDA_GLOBAL_ADDRSPACE) {
    srcCastTy = LLVM_INT8PTR_GLOBAL;
    srcTyStr = "p1i8";
  }
  else if (srcAddrspace == CUDA_GENERIC_ADDRSPACE) {
    srcCastTy = LLVM_INT8PTR;
    srcTyStr = "p0i8";
  }
  else {
    not_supported_yet;
  }

  // Emit our own memcpy decl, since the built-in has attributes which
  // are not handled by NVVM
  std::string memcpyName = "llvm.memcpy."+dstTyStr+"."+srcTyStr+".i32";
  llvm::Function *func = getBuiltIn(
      memcpyName, LLVM_VOID,
      {dstCastTy, srcCastTy, LLVM_INT, LLVM_INT, LLVM_BOOL});
  cleanFuncAttrs(func);

  llvm::Value *llvmAlign = llvmInt(align);
  llvm::Value *castDst = builder->CreateBitCast(dst, dstCastTy);
  llvm::Value *castSrc = builder->CreateBitCast(src, srcCastTy);
  llvm::Constant *isVolatile = llvmBool(true);
  builder->CreateCall5(func, castDst, castSrc, size, llvmAlign, isVolatile);
}

void GPUBackend::emitMemSet(llvm::Value *dst, llvm::Value *val,
                            llvm::Value *size, unsigned align) {
  iassert(dst->getType()->isPointerTy());

  unsigned dstAddrspace = llvm::cast<llvm::PointerType>(
      dst->getType())->getAddressSpace();
  llvm::Type *dstCastTy;
  std::string dstTyStr;
  if (dstAddrspace == CUDA_GLOBAL_ADDRSPACE) {
    dstCastTy = LLVM_INT8PTR_GLOBAL;
    dstTyStr = "p1i8";
  }
  else if (dstAddrspace == CUDA_GENERIC_ADDRSPACE) {
    dstCastTy = LLVM_INT8PTR;
    dstTyStr = "p0i8";
  }
  else {
    not_supported_yet;
  }

  // Emit our own memset decl, since the built-in has attributes which
  // are not handled by NVVM
  std::string memsetName = "llvm.memset."+dstTyStr+".i32";
  llvm::Function *func = getBuiltIn(
      memsetName, LLVM_VOID,
      { dstCastTy, LLVM_INT8, LLVM_INT, LLVM_INT, LLVM_BOOL });
  cleanFuncAttrs(func);

  llvm::Value *llvmAlign = llvmInt(align);
  llvm::Value *castDst = builder->CreateBitCast(dst, dstCastTy);
  llvm::Constant *isVolatile = llvmBool(true);
  builder->CreateCall5(func, castDst, val, size, llvmAlign, isVolatile);
}


void GPUBackend::emitShardedMemSet(ir::Type targetType, llvm::Value *target,
                                   llvm::Value *length) {
  iassert(!inKernel);
  iassert(targetType.isTensor());

  // Stash the symtable
  ScopedMap<ir::Var, llvm::Value*> oldSymtable = symtable;
  symtable.clear();
  // Stash the current basic block
  llvm::BasicBlock *prevBB = builder->GetInsertBlock();

  // Create LLVM func
  ir::Var targetArg("target", targetType);
  ir::Var lengthArg("length", ir::Int);
  llvm::Function *kernel = emitEmptyFunction(
      "memset_kernel", {targetArg, lengthArg}, {},  true, false);
  builder->SetInsertPoint(&kernel->getEntryBlock());

  // Kernel metadata
  addNVVMAnnotation(kernel, "kernel", llvmInt(1), module);
  
  llvm::BasicBlock *bodyStart = llvm::BasicBlock::Create(
    LLVM_CONTEXT, "bodyStart", kernel);
  llvm::BasicBlock *earlyExit = llvm::BasicBlock::Create(
    LLVM_CONTEXT, "earlyExit", kernel);
  
  // Guard: check if we're outside the intended range of the kernel loop and 
  // early-exit if so.
  llvm::Value *cond = builder->CreateICmpULT(getTidX(), symtable.get(lengthArg));
  builder->CreateCondBr(cond, bodyStart, earlyExit);

  builder->SetInsertPoint(earlyExit);
  builder->CreateRetVoid();

  // Continue with kernel body
  builder->SetInsertPoint(bodyStart);

  // Actual assign
  llvm::Value *value;
  if (targetType.toTensor()->componentType.kind == ir::ScalarType::Float) {
    value = llvmFP(0);
  }
  else if (targetType.toTensor()->componentType.kind == ir::ScalarType::Int) {
    value = llvmInt(0);
  }
  else {
    not_supported_yet;
  }
  llvm::Value *ptr = builder->CreateGEP(symtable.get(targetArg), getTidX());
  builder->CreateStore(value, ptr);

  // Kernel should always return void
  builder->CreateRetVoid();

  // Unstash symtable
  symtable = oldSymtable;

  // Emit kernel launch
  builder->SetInsertPoint(prevBB);
  emitKernelLaunch(kernel, {target, length}, length, nullptr, nullptr);
}

void GPUBackend::emitShardedDot(ir::Type vec1Type, ir::Type vec2Type,
                                ir::Type resType,
                                llvm::Value *vec1, llvm::Value *vec2,
                                llvm::Value *size, llvm::Value *result) {
  // Clear result first
  iassert(resType.toTensor()->componentType.kind == ir::ScalarType::Float);
  builder->CreateStore(llvmFP(0), result);

  // Stash the symtable
  ScopedMap<ir::Var, llvm::Value*> oldSymtable = symtable;
  symtable.clear();
  // Stash the current basic block
  llvm::BasicBlock *prevBB = builder->GetInsertBlock();

  // Create LLVM func
  ir::Var resVar("result", resType);
  ir::Var vec1Var("vec1", vec1Type);
  ir::Var vec2Var("vec2", vec2Type);
  ir::Var sizeVar("size", ir::Int);
  llvm::Function *kernel = emitEmptyFunction(
      "dot_kernel", {vec1Var, vec2Var, sizeVar}, {resVar}, true, false);
  builder->SetInsertPoint(&kernel->getEntryBlock());

  // Kernel metadata
  addNVVMAnnotation(kernel, "kernel", llvmInt(1), module);
  
  llvm::BasicBlock *bodyStart = llvm::BasicBlock::Create(
      LLVM_CONTEXT, "bodyStart", kernel);
  llvm::BasicBlock *earlyExit = llvm::BasicBlock::Create(
      LLVM_CONTEXT, "earlyExit", kernel);
  
  // Guard: check if we're outside the intended range of the kernel loop and 
  // early-exit if so.
  llvm::Value *cond = builder->CreateICmpULT(getTidX(), symtable.get(sizeVar));
  builder->CreateCondBr(cond, bodyStart, earlyExit);

  builder->SetInsertPoint(earlyExit);
  builder->CreateRetVoid();

  // Continue with kernel body
  builder->SetInsertPoint(bodyStart);

  // Perform multiply and add
  llvm::Value *val1 = builder->CreateLoad(
      builder->CreateGEP(symtable.get(vec1Var), getTidX()));
  llvm::Value *val2 = builder->CreateLoad(
      builder->CreateGEP(symtable.get(vec2Var), getTidX()));
  llvm::Value *mul;
  iassert(val1->getType()->isFloatTy());
  mul = builder->CreateFMul(val1, val2);
  emitAtomicLoadAdd(symtable.get(resVar), mul);

  // Kernel should always return void
  builder->CreateRetVoid();

  // Unstash symtable
  symtable = oldSymtable;

  // Emit kernel launch
  builder->SetInsertPoint(prevBB);
  emitKernelLaunch(kernel, {vec1, vec2, size, result}, size, nullptr, nullptr);
}

void GPUBackend::emitFillBuf(llvm::Value *buffer,
                             std::vector<llvm::Value*> vals) {
  uint64_t bufIndex = 0;
  for (auto &val : vals) {
    llvm::Value *bufPtr = builder->CreateGEP(buffer, llvmInt(bufIndex));
    llvm::Value *valPtr = builder->CreateBitCast(
        bufPtr,
        // Pointer to arg type, addrspace 0
        llvm::PointerType::get(val->getType(), 0));
    // Aligned store required to ensure that each parameter remains 8-byte aligned
    builder->CreateAlignedStore(val, valPtr, 8);
    bufIndex += dataLayout->getTypeAllocSize(val->getType());
    if (bufIndex % 8 != 0) {
      iassert(bufIndex % 8 == 4)
          << "Cannot accept non 4-byte aligned params";
      bufIndex += 4;
    }
  }
}

void GPUBackend::makeGlobalTensor(ir::Var var) {
  LLVMBackend::makeGlobalTensor(var);

  // Annotate the global as managed memory to allow us to write its
  // value from the CUDA setup
  llvm::Value *global = buffers[var];
  addNVVMAnnotation(global, "managed", llvmInt(1), module);
  exported.push_back(global->getName().data());

  // Replace the load in the symtable with an appropriately casted version
  llvm::Value *llvmTmp = symtable.get(var);
  symtable.insert(var, emitCastGlobalToGen(llvmTmp));
}

}
}
