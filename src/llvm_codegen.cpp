#include "llvm_codegen.h"

#include <iostream>
#include <utility>

using namespace std;
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

llvm::Type *llvmType(ir::ScalarType stype) {
  switch (stype.kind) {
    case ir::ScalarType::Int:
      return LLVM_INT;
    case ir::ScalarType::Float:
      return LLVM_DOUBLE;
  }
}

llvm::Type *llvmPtrType(ir::ScalarType stype) {
  switch (stype.kind) {
    case ir::ScalarType::Int:
      return LLVM_INTPTR;
    case ir::ScalarType::Float:
      return LLVM_DOUBLEPTR;
  }
}

llvm::Type *llvmPtrType(const ir::TensorType *ttype) {
  return llvmPtrType(ttype->componentType);
}

llvm::Type *llvmPtrType(const ir::Type &type){
  switch (type.kind()) {
    case ir::Type::Tensor:
      return llvmPtrType(type.toTensor());
      break;
    case ir::Type::Element:
      NOT_SUPPORTED_YET;
      break;
    case ir::Type::Set:
      NOT_SUPPORTED_YET;
      break;
    case ir::Type::Tuple:
      NOT_SUPPORTED_YET;
      break;
      
  }
}

llvm::Constant *llvmPtr(const ir::Type &type, void *data) {
  llvm::Constant *c = (sizeof(void*) == 4)
      ? llvm::ConstantInt::get(llvm::Type::getInt32Ty(LLVM_CONTEXT),
                               (int)(intptr_t)data)
      : llvm::ConstantInt::get(llvm::Type::getInt64Ty(LLVM_CONTEXT),
                               (intptr_t)data);
  return llvm::ConstantExpr::getIntToPtr(c, llvmPtrType(type));
}

llvm::Constant *llvmPtr(simit::ir::Literal *literal) {
  assert(literal->type.isTensor());
  return llvmPtr(literal->type, literal->data);
}

ir::Type simitType(const llvm::Type *type) {
  if (type->isPointerTy()) {
    type = type->getPointerElementType();
  }

  if (type->isDoubleTy()) {
    return ir::Float();
  }
  else if (type->isIntegerTy()) {
    return ir::Int();
  }
  else {
    UNREACHABLE;
  }
}

namespace {
void llvmArgument(ir::Var arg, std::vector<std::string> *names,
                  std::vector<llvm::Type*> *types) {
  switch (arg.type.kind()) {
    case ir::Type::Tensor: {
      names->push_back(arg.name);
      types->push_back(llvmPtrType(arg.type));
      break;
    }
    case ir::Type::Element: {
      NOT_SUPPORTED_YET;
      break;
    }
    case ir::Type::Set: {
      names->push_back(arg.name);
      types->push_back(LLVM_INT32);

      // Emit one function argument per set field
      const ir::SetType *type = arg.type.toSet();
      for (auto &field : type->elementType.toElement()->fields) {
        names->push_back(arg.name +"."+ field.first);
        types->push_back(llvmPtrType(field.second));
      }
      break;
    }
    case ir::Type::Tuple: {
      NOT_SUPPORTED_YET;
      break;
    }
  }
}

void llvmArguments(const std::vector<ir::Var> &arguments,
                   const std::vector<ir::Var> &results,
                   std::vector<std::string> *llvmArgNames,
                   std::vector<llvm::Type*> *llvmArgTypes) {
  // We don't need two llvm arguments for aliased simit argument/results
  std::set<std::string> argNames;

  for (auto &arg : arguments) {
    argNames.insert(arg.name);
    llvmArgument(arg, llvmArgNames, llvmArgTypes);
  }

  for (auto &res : results) {
    if (argNames.find(res.name) != argNames.end()) {
      continue;
    }
    llvmArgument(res, llvmArgNames, llvmArgTypes);
  }
}

} // unnamed namespace


llvm::Function *createFunction(const std::string &name,
                               const vector<ir::Var> &args,
                               const vector<ir::Var> &results,
                               llvm::Module *module) {
  vector<string>      llvmArgNames;
  vector<llvm::Type*> llvmArgTypes;
  llvmArguments(args, results, &llvmArgNames, &llvmArgTypes);
  assert(llvmArgNames.size() == llvmArgTypes.size());

  llvm::FunctionType *ft= llvm::FunctionType::get(LLVM_VOID,llvmArgTypes,false);

  llvm::Function *f= llvm::Function::Create(ft, llvm::Function::InternalLinkage,
                                            name, module);
  f->setDoesNotThrow();
  unsigned i = 0;
  for (llvm::Argument &arg : f->getArgumentList()) {
    arg.setName(llvmArgNames[i]);

    if (arg.getType()->isPointerTy()) {
      f->setDoesNotCapture(i+1);  // setDoesNotCapture(0) is return value
    }
    ++i;
  }

  return f;
}

}} // namespace simit::internal
