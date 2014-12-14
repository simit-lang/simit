#include "llvm_codegen.h"

#include <iostream>
#include <utility>

#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"

#include "llvm/Support/raw_ostream.h"

using namespace std;
using namespace simit::ir;
using namespace simit::internal;

namespace simit {
namespace internal {

llvm::ConstantInt *llvmInt(long long int val, unsigned bits) {
  return llvm::ConstantInt::get(LLVM_CONTEXT, llvm::APInt(bits, val, true));
}

llvm::ConstantInt *llvmUInt(long long unsigned int val, unsigned bits) {
  return llvm::ConstantInt::get(LLVM_CONTEXT, llvm::APInt(bits, val, false));
}

llvm::ConstantFP *llvmFP(double val, unsigned bits) {
  return llvm::ConstantFP::get(LLVM_CONTEXT, llvm::APFloat(val));
}

void print(const std::string &format, std::initializer_list<llvm::Value*> args,
           LLVMIRBuilder *builder, llvm::Module *module) {
  auto int32Type = llvm::IntegerType::getInt32Ty(LLVM_CONTEXT);
  llvm::Function *printfFunc = module->getFunction("printf");
  if (printfFunc == nullptr) {
    std::vector<llvm::Type*> printfArgTypes;
    printfArgTypes.push_back(llvm::Type::getInt8PtrTy(LLVM_CONTEXT));
    llvm::FunctionType* printfType = llvm::FunctionType::get(int32Type,
                                                             printfArgTypes,
                                                             true);
    printfFunc = llvm::Function::Create(printfType,
                                        llvm::Function::ExternalLinkage,
                                        llvm::Twine("printf"), module);
    printfFunc->setCallingConv(llvm::CallingConv::C);
  }

  auto formatValue = llvm::ConstantDataArray::getString(LLVM_CONTEXT, format);

  auto intType = llvm::IntegerType::get(LLVM_CONTEXT,8);
  auto formatStrType = llvm::ArrayType::get(intType, format.size()+1);

  llvm::GlobalVariable *formatStr =
      new llvm::GlobalVariable(*module, formatStrType, true,
                               llvm::GlobalValue::PrivateLinkage, formatValue,
                               ".str");
  llvm::Constant *zero = llvm::Constant::getNullValue(int32Type);

  std::vector<llvm::Constant*> idx;
  idx.push_back(zero);
  idx.push_back(zero);
  llvm::Constant *str = llvm::ConstantExpr::getGetElementPtr(formatStr, idx);

  std::vector<llvm::Value*> printfArgs;
  printfArgs.push_back(str);
  printfArgs.insert(printfArgs.end(), args.begin(), args.end());

  builder->CreateCall(printfFunc, printfArgs);
}

llvm::Type *createLLVMType(ScalarType stype) {
  switch (stype.kind) {
    case ScalarType::Int:
      return LLVM_INT;
    case ScalarType::Float:
      return LLVM_DOUBLE;
    case ScalarType::Boolean:
      return LLVM_BOOL;
  }
}

llvm::Type *llvmPtrType(ScalarType stype) {
  switch (stype.kind) {
    case ScalarType::Int:
      return LLVM_INTPTR;
    case ScalarType::Float:
      return LLVM_DOUBLEPTR;
    case ScalarType::Boolean:
      return LLVM_BOOL;
  }
}

llvm::Constant *llvmPtr(llvm::Type *type, const void *data) {
  llvm::Constant *c = (sizeof(void*) == 4)
      ? llvm::ConstantInt::get(llvm::Type::getInt32Ty(LLVM_CONTEXT),
                               (int)(intptr_t)data)
      : llvm::ConstantInt::get(llvm::Type::getInt64Ty(LLVM_CONTEXT),
                               (intptr_t)data);
  return llvm::ConstantExpr::getIntToPtr(c, type);
}

llvm::Constant *llvmPtr(const Type &type, const void *data) {
  return llvmPtr(createLLVMType(type), data);
}

llvm::Constant *llvmPtr(const Literal *literal) {
  assert(literal->type.isTensor());
  return llvmPtr(literal->type, literal->data);
}

Type simitType(const llvm::Type *type) {
  if (type->isPointerTy()) {
    type = type->getPointerElementType();
  }

  if (type->isDoubleTy()) {
    return Float;
  }
  else if (type->isIntegerTy()) {
    return Int;
  }

  unreachable;
  return Type();
}

llvm::Type *createLLVMType(const TensorType *ttype) {
  return llvmPtrType(ttype->componentType);
}

/// One for endpoints, two for neighbor index
extern const int NUM_EDGE_INDEX_ELEMENTS = 3;

// TODO: replace anonymous struct with one struct per element and set type
llvm::StructType *createLLVMType(const ir::SetType *setType) {
  const ElementType *elemType = setType->elementType.toElement();
  vector<llvm::Type*> llvmFieldTypes;

  // Set size
  llvmFieldTypes.push_back(LLVM_INT);

  // Edge indices (if the set is an edge set)
  if (setType->endpointSets.size() > 0) {
    // Endpoints
    llvmFieldTypes.push_back(LLVM_INTPTR);

    // Neighbor Index
    llvmFieldTypes.push_back(LLVM_INTPTR);
    llvmFieldTypes.push_back(LLVM_INTPTR);
  }

  // Fields
  for (const Field &field : elemType->fields) {
    llvmFieldTypes.push_back(createLLVMType(field.type));
  }
  return llvm::StructType::get(LLVM_CONTEXT, llvmFieldTypes);
}

llvm::Type *createLLVMType(const Type &type) {
  switch (type.kind()) {
    case Type::Tensor:
      return createLLVMType(type.toTensor());
    case Type::Element:
      not_supported_yet;
      break;
    case Type::Set:
      return createLLVMType(type.toSet());
    case Type::Tuple:
      not_supported_yet;
      break;
  }
  unreachable;
  return nullptr;
}

static llvm::Function *createFunction(const std::string &name,
                                      const std::vector<std::string> &argNames,
                                      const std::vector<llvm::Type*> &argTypes,
                                      llvm::Module *module) {
  llvm::FunctionType *ft= llvm::FunctionType::get(LLVM_VOID,argTypes,false);
  llvm::Function *f= llvm::Function::Create(ft, llvm::Function::InternalLinkage,
                                            name, module);
  f->setDoesNotThrow();
  unsigned i = 0;
  for (llvm::Argument &arg : f->getArgumentList()) {
    arg.setName(argNames[i]);

    if (arg.getType()->isPointerTy()) {
      f->setDoesNotCapture(i+1);  //  setDoesNotCapture(0) is the return value
    }
    ++i;
  }

  return f;
}

llvm::Function *createFunction(const std::string &name,
                               const vector<Var> &arguments,
                               const vector<Var> &results,
                               llvm::Module *module) {
  vector<string>      llvmArgNames;
  vector<llvm::Type*> llvmArgTypes;

  // We don't need two llvm arguments for aliased simit argument/results
  std::set<std::string> argNames;
  
  for (auto &arg : arguments) {
    argNames.insert(arg.getName());
    llvmArgNames.push_back(arg.getName());
    llvmArgTypes.push_back(createLLVMType(arg.getType()));
  }
  for (auto &res : results) {
    if (argNames.find(res.getName()) != argNames.end()) {
      continue;
    }
    llvmArgNames.push_back(res.getName());
    llvmArgTypes.push_back(createLLVMType(res.getType()));
  }

  assert(llvmArgNames.size() == llvmArgTypes.size());

  return createFunction(name, llvmArgNames, llvmArgTypes, module);
}

std::ostream &operator<<(std::ostream &os, const llvm::Value &value) {
  std::string str;
  llvm::raw_string_ostream ss(str);
  value.print(ss);
  return os << ss.str();
}

std::ostream &operator<<(std::ostream &os, const llvm::Type &type) {
  std::string str;
  llvm::raw_string_ostream ss(str);
  type.print(ss);
  return os << ss.str();
}

}} // namespace simit::internal
