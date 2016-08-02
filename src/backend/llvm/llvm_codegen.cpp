#include "llvm_codegen.h"

#include <utility>
#include "llvm_types.h"
#include "llvm_util.h"
#include "ir.h"

#include "llvm/IR/GlobalVariable.h"

using namespace std;
using namespace simit::ir;

namespace simit {
namespace backend {

/// SimitIRBuilder
llvm::Value *SimitIRBuilder::CreateComplex(
    llvm::Value *real, llvm::Value *imag) {
  llvm::Value *zero = llvmComplex(0, 0);
  llvm::Value *partial = CreateInsertValue(zero, real, 0);
  return CreateInsertValue(partial, imag, 1);
}

llvm::Value *SimitIRBuilder::ComplexGetReal(llvm::Value *c) {
  return CreateExtractValue(c, 0, "real");
}

llvm::Value *SimitIRBuilder::ComplexGetImag(llvm::Value *c) {
  return CreateExtractValue(c, 1, "imag");
}


llvm::ConstantInt *llvmInt(long long int val, unsigned bits) {
  return llvm::ConstantInt::get(LLVM_CTX, llvm::APInt(bits, val, true));
}

llvm::ConstantInt *llvmUInt(long long unsigned int val, unsigned bits) {
  return llvm::ConstantInt::get(LLVM_CTX, llvm::APInt(bits, val, false));
}

llvm::Constant *llvmFP(double val, unsigned bits) {
  return llvm::ConstantFP::get(llvmFloatType(), val);
}

llvm::Constant* llvmBool(bool val) {
  int intVal = (val) ? 1 : 0;
  return llvm::ConstantInt::get(LLVM_CTX, llvm::APInt(1, intVal, false));
}

llvm::Constant* llvmComplex(double real, double imag) {
  return llvm::ConstantStruct::get(llvmComplexType(),
                                   llvmFP(real), llvmFP(imag), nullptr);
}

llvm::Constant *llvmPtr(llvm::PointerType* type, const void* data) {
  llvm::Constant* c = (sizeof(void*) == 4)
      ? llvm::ConstantInt::get(llvm::Type::getInt32Ty(LLVM_CTX),
                               (int)(intptr_t)data)
      : llvm::ConstantInt::get(llvm::Type::getInt64Ty(LLVM_CTX),
                               (intptr_t)data);
  return llvm::ConstantExpr::getIntToPtr(c, type);
}

llvm::Constant *llvmPtr(const TensorType &type, const void *data,
                        unsigned addrspace) {
  return llvmPtr(llvmType(type, addrspace), data);
}

llvm::Constant* llvmPtr(const Literal& literal) {
  iassert(literal.type.isTensor());
  return llvmPtr(*literal.type.toTensor(), literal.data);
}

llvm::Constant* llvmVal(const TensorType& type, const void *data) {
  ScalarType componentType = type.getComponentType();
  switch (componentType.kind) {
    case ScalarType::Int:
      return llvmInt(static_cast<const int*>(data)[0]);
    case ScalarType::Float:
      if (ir::ScalarType::singleFloat()) {
        return llvmFP(static_cast<const float*>(data)[0], componentType.bytes());
      }
      else {
        return llvmFP(static_cast<const double*>(data)[0], componentType.bytes());
      }
    case ScalarType::Boolean:
      return llvmBool(static_cast<const bool*>(data)[0]);
    case ScalarType::Complex:
      if (ir::ScalarType::singleFloat()) {
        return llvmComplex(static_cast<const float*>(data)[0],
                           static_cast<const float*>(data)[1]);
      }
      else {
        return llvmComplex(static_cast<const double*>(data)[0],
                           static_cast<const double*>(data)[1]);
      }
    case ScalarType::String:
      break;
  }
  ierror;
  return nullptr;
}

llvm::Constant* llvmVal(const Literal& literal) {
  iassert(literal.type.isTensor());
  return llvmVal(*literal.type.toTensor(), literal.data);
}

llvm::Constant* initializer(llvm::Type* type);


llvm::Function *createPrototypeLLVM(const std::string& name,
                                    const vector<string>& argNames,
                                    const vector<llvm::Type*>& argTypes,
                                    llvm::Module* module,
                                    bool externalLinkage,
                                    bool doesNotThrow) {
  llvm::FunctionType *ft = llvm::FunctionType::get(LLVM_VOID, argTypes, false);
  auto linkage = externalLinkage ? llvm::Function::ExternalLinkage
                                 : llvm::Function::InternalLinkage;
  llvm::Function *f= llvm::Function::Create(ft, linkage, name, module);
  if (doesNotThrow) {
    f->setDoesNotThrow();
  }
  unsigned i = 0;
  for (llvm::Argument &arg : f->getArgumentList()) {
    arg.setName(argNames[i]);

    // TODO(gkanwar): Move noalias code here from GPU implementation
    if (arg.getType()->isPointerTy()) {
      f->setDoesNotCapture(i+1);  //  setDoesNotCapture(0) is the return value
    }
    ++i;
  }

  return f;
}

llvm::Constant* defaultInitializer(llvm::Type* type) {
  llvm::Constant* initializer = nullptr;
  if (type->isIntegerTy()) {
    return llvmInt(0);
  }
  else if (type->isFloatingPointTy()) {
    return llvmFP(0.0);
  }
  else if (type->isPointerTy()) {
    llvm::PointerType* ptrType = llvm::cast<llvm::PointerType>(type);
    return llvm::ConstantPointerNull::get(ptrType);
  }
  else if (type->isStructTy()) {
    llvm::StructType* structType = llvm::cast<llvm::StructType>(type);
    return llvm::ConstantAggregateZero::get(structType);
  }
  else {
    ierror << "creating an initializer not supported for this type";
  }
  return initializer;
}

llvm::Function* createPrototype(const std::string &name,
                                const vector<Var> &arguments,
                                const vector<Var> &results,
                                llvm::Module *module,
                                bool externalLinkage,
                                bool doesNotThrow,
                                bool scalarsByValue,
                                unsigned addrspace) {
  vector<string>      llvmArgNames;
  vector<llvm::Type*> llvmArgTypes;

  // We don't need two llvm arguments for aliased simit argument/results
  std::set<std::string> argNames;
  
  for (auto &arg : arguments) {
    argNames.insert(arg.getName());
    llvmArgNames.push_back(arg.getName());

    // Our convention is that scalars are passed to functions by value,
    // while everything else is passed through a pointer
    llvm::Type *type = (isScalar(arg.getType()) && scalarsByValue)
        ? llvmType(arg.getType().toTensor()->getComponentType())
        : llvmType(arg.getType(), addrspace);
    llvmArgTypes.push_back(type);
  }

  for (auto &res : results) {
    if (argNames.find(res.getName()) != argNames.end()) {
      continue;
    }
    llvmArgNames.push_back(res.getName());
    llvmArgTypes.push_back(llvmType(res.getType(), addrspace));
  }

  assert(llvmArgNames.size() == llvmArgTypes.size());

  return createPrototypeLLVM(name, llvmArgNames, llvmArgTypes,
                             module, externalLinkage, doesNotThrow);
}

llvm::GlobalVariable* createGlobal(llvm::Module *module, const Var& var,
                                   llvm::GlobalValue::LinkageTypes linkage,
                                   unsigned addrspace, bool packed) {
  bool isExtern = (linkage == llvm::GlobalValue::ExternalLinkage);

  Type type = var.getType();

  // Make sure extern structs are packed, so that we can correctly set them
  llvm::Type* externType = (type.isSet() && packed)
                             ? llvmType(type.toSet(), addrspace, isExtern)
                             : llvmType(type, addrspace);

  llvm::Constant* initializer = defaultInitializer(externType);
  llvm::GlobalVariable* globalPtr =
      new llvm::GlobalVariable(*module,
                               externType,
                               false,
                               linkage,
                               initializer,
                               var.getName(),
                               nullptr,
                               llvm::GlobalVariable::NotThreadLocal,
                               addrspace,
                               isExtern);
  globalPtr->setAlignment(8);
  return globalPtr;
}

}}
