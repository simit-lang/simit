#include "llvm_codegen.h"

#include <iostream>
#include <utility>

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

llvm::Type *createLLVMType(ScalarType stype) {
  switch (stype.kind) {
    case ScalarType::Int:
      return LLVM_INT;
    case ScalarType::Float:
      return LLVM_DOUBLE;
  }
}

llvm::Type *llvmPtrType(ScalarType stype) {
  switch (stype.kind) {
    case ScalarType::Int:
      return LLVM_INTPTR;
    case ScalarType::Float:
      return LLVM_DOUBLEPTR;
  }
}

llvm::Constant *llvmPtr(llvm::Type *type, void *data) {
  llvm::Constant *c = (sizeof(void*) == 4)
      ? llvm::ConstantInt::get(llvm::Type::getInt32Ty(LLVM_CONTEXT),
                               (int)(intptr_t)data)
      : llvm::ConstantInt::get(llvm::Type::getInt64Ty(LLVM_CONTEXT),
                               (intptr_t)data);
  return llvm::ConstantExpr::getIntToPtr(c, type);
}

llvm::Constant *llvmPtr(const Type &type, void *data) {
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

llvm::StructType *createLLVMType(const ir::SetType *setType) {
  const ElementType *elemType = setType->elementType.toElement();
  vector<llvm::Type*> llvmFieldTypes;

  // Set size
  llvmFieldTypes.push_back(LLVM_INT);

  // Edge indices (if the set is an edge set)
  if (setType->endpointSets.size() > 0) {
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
