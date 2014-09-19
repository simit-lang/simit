#include "llvm_codegen.h"

#include <iostream>
#include <utility>

using namespace std;
using namespace simit::internal;

namespace simit {
namespace internal {

llvm::ConstantInt* getInt32(int val) {
  return llvm::ConstantInt::get(LLVM_CONTEXT, llvm::APInt(32, val, true));
}

llvm::ConstantInt* getUInt32(unsigned val) {
  return llvm::ConstantInt::get(LLVM_CONTEXT, llvm::APInt(32, val, false));
}

llvm::Type *llvmType(const simit::ComponentType ctype) {
  assert(isValidComponentType(ctype));
  switch (ctype) {
    case simit::ComponentType::INT:
      return LLVM_INT;
    case simit::ComponentType::FLOAT:
      return LLVM_DOUBLE;
  }
}

llvm::Type *llvmType(const ir::TensorType *type){
  assert(isValidComponentType(type->getComponentType()));
  switch (type->getComponentType()) {
    case simit::ComponentType::INT:
      return LLVM_INTPTR;
    case simit::ComponentType::FLOAT:
      return LLVM_DOUBLEPTR;
  }
}

llvm::Constant *llvmPtr(ir::TensorType *type, void *data) {
  llvm::Constant *c = (sizeof(void*) == 4)
      ? llvm::ConstantInt::get(llvm::Type::getInt32Ty(LLVM_CONTEXT),
                               (int)(intptr_t)data)
      : llvm::ConstantInt::get(llvm::Type::getInt64Ty(LLVM_CONTEXT),
                               (intptr_t)data);
  return llvm::ConstantExpr::getIntToPtr(c, llvmType(type));
}

llvm::Constant *llvmPtr(simit::ir::Literal *literal) {
  assert(literal->getType()->isTensor());
  return llvmPtr(tensorTypePtr(literal->getType()), literal->getData());
}

simit::ComponentType simitType(const llvm::Type *type) {
  if (type->isPointerTy()) {
    type = type->getPointerElementType();
  }

  if (type->isDoubleTy()) {
    return simit::FLOAT;
  }
  else if (type->isIntegerTy()) {
    return simit::INT;
  }
  else {
    UNREACHABLE;
  }
}

namespace {
void llvmArgument(const std::shared_ptr<ir::Argument> &arg,
                  std::vector<std::string> *names,
                  std::vector<llvm::Type*> *types) {
  switch (arg->getType()->getKind()) {
    case ir::Type::Tensor:
      names->push_back(arg->getName());
      types->push_back(llvmType(tensorTypePtr(arg->getType())));
      break;
    case ir::Type::Set:
      names->push_back(arg->getName());
      types->push_back(LLVM_INT32);

      // Emit one function argument per set field
      ir::SetType *type = setTypePtr(arg->getType());
      for (auto &field : type->getElementType()->getFields()) {
        names->push_back(arg->getName() + "." + field.first);
        types->push_back(llvmType(field.second.get()));
      }

      break;
  }
}

void llvmArguments(const std::vector<std::shared_ptr<ir::Argument>> &arguments,
                   const std::vector<std::shared_ptr<ir::Result>> &results,
                   std::vector<std::string> *llvmArgNames,
                   std::vector<llvm::Type*> *llvmArgTypes) {
  // We don't need two llvm arguments for aliased simit argument/results
  std::set<std::string> argNames;

  for (auto &arg : arguments) {
    argNames.insert(arg->getName());
    llvmArgument(arg, llvmArgNames, llvmArgTypes);
  }

  for (auto &res : results) {
    if (argNames.find(res->getName()) != argNames.end()) {
      continue;
    }
    llvmArgument(res, llvmArgNames, llvmArgTypes);
  }
}

} // unnamed namespace


llvm::Function *createFunction(const std::string &name,
                               const vector<shared_ptr<ir::Argument>> &args,
                               const vector<shared_ptr<ir::Result>> &results,
                               llvm::GlobalValue::LinkageTypes linkage,
                               llvm::Module *module) {
  vector<string>      llvmArgNames;
  vector<llvm::Type*> llvmArgTypes;
  llvmArguments(args, results, &llvmArgNames, &llvmArgTypes);
  assert(llvmArgNames.size() == llvmArgTypes.size());

  llvm::FunctionType *ft = llvm::FunctionType::get(LLVM_VOID, llvmArgTypes,
                                                   false);

  llvm::Function *f = llvm::Function::Create(ft, linkage, name, module);
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
