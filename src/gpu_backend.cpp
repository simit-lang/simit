#include "gpu_backend.h"

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Type.h"

#include "error.h"
#include "gpu_function.h"
#include "types.h"

namespace simit {
namespace internal {

namespace {

llvm::Type *getTypeFromTensor(const simit::ir::TensorType *tensor,
                              llvm::LLVMContext& ctx) {
  if (tensor->componentType.isInt()) {
    return llvm::ArrayType::get(llvm::Type::getInt32Ty(ctx), tensor->size());
  }
  else if (tensor->componentType.isFloat()) {
    // TODO(gkanwar): Is this float or double?
    return llvm::ArrayType::get(llvm::Type::getFloatTy(ctx), tensor->size());
  }
  else {
    // ERROR! Should not fall through.
    ierror << "Tensor type not int or float";
  }
}

llvm::StructType *getStructFromSet(const std::string& name,
                                   const simit::ir::SetType *setType,
                                   llvm::LLVMContext& ctx) {
  std::vector<llvm::Type*> fields;
  for (auto &field : setType->elementType.toElement()->fields) {
    iassert(field.type.isTensor());
    fields.push_back(getTypeFromTensor(field.type.toTensor(), ctx));
  }
  return llvm::StructType::create(llvm::ArrayRef<llvm::Type*>(fields), name);
}

}  // namespace

GPUBackend::GPUBackend() {}
GPUBackend::~GPUBackend() {}

simit::Function *GPUBackend::compile(simit::ir::Func irFunc) {
  llvm::LLVMContext& context = llvm::getGlobalContext();
  llvm::Module *mod = new llvm::Module("nvvm-module", context);

  // Set appropriate data layout
  if (sizeof(void*) == 8)
    mod->setDataLayout("e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
                       "i64:64:64-f32:32:32-f64:64:64-v16:16:16-v32:32:32-"
                       "v64:64:64-v128:128:128-n16:32:64");
  else
    mod->setDataLayout("e-p:32:32:32-i1:8:8-i8:8:8-i16:16:16-i32:32:32-"
                       "i64:64:64-f32:32:32-f64:64:64-v16:16:16-v32:32:32-"
                       "v64:64:64-v128:128:128-n16:32:64");

  std::vector<llvm::Type*> paramTys;
  // Pass 1: Build param type list
  for (const simit::ir::Var &arg : irFunc.getArguments()) {
    switch (arg.getType().kind()) {
      case ir::Type::Tensor: {
        paramTys.push_back(getTypeFromTensor(arg.getType().toTensor(), context));
        break;
      }
      case ir::Type::Element: {
        not_supported_yet;
        break;
      }
      case ir::Type::Set: {
        paramTys.push_back(getStructFromSet(arg.getName(), arg.getType().toSet(), context));
        break;
      }
      case ir::Type::Tuple: {
        not_supported_yet;
        break;
      }
    }
  }

  llvm::Type *voidTy = llvm::Type::getVoidTy(context);
  llvm::FunctionType *funcTy = llvm::FunctionType::get(voidTy, paramTys, false);
  llvm::Function *func = llvm::Function::Create(funcTy, llvm::GlobalValue::ExternalLinkage, "kernel", mod);

  // Pass 2: Name LLVM arguments
  auto arg = func->arg_begin();
  auto irArg = irFunc.getArguments().begin();
  while (arg != func->arg_end()) {
    arg->setName(irArg->getName());
    arg++;
    irArg++;
  }

  // Build 'entry' basic block
  llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entry", func);
  builder.reset(new llvm::IRBuilder<>(entry));

  irFunc.getBody().accept(this);

  return new GPUFunction(irFunc, func, mod);
}

}
}
