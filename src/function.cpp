#include "function.h"

#include <iostream>
using namespace std;

#include "graph.h"
#include "types.h"
#include "ir.h"
#include "types.h"

namespace simit {

Function::Function(const simit::ir::Function &simitFunc)
    : funcPtr(NULL), initRequired(true) {
  for (auto &argument : simitFunc.getArguments()) {
    formals.push_back(argument->getName());
    actuals[argument->getName()] = Actual(argument->getType());
  }
  for (auto &result : simitFunc.getResults()) {
    // Skip results that alias an argument
    if (actuals.find(result->getName()) != actuals.end()) {
      continue;
    }
    formals.push_back(result->getName());
    actuals[result->getName()] = Actual(result->getType());
  }
}

Function::~Function() {
}

void Function::bind(const std::string &argName, Tensor *tensor) {
  assert(actuals.find(argName) != actuals.end() &&
         "no argument of this name in function");

  // Check that the tensor matches the argument type
  assert(*tensor->getType() == *actuals[argName].getType() &&
         "tensor type does not match function argument type");

  actuals[argName].bind(tensor);
  initRequired = true;
}

void Function::bind(const std::string &argName, SetBase *set) {
  assert(actuals.find(argName) != actuals.end() &&
         "no argument of this name in function");

  // Check that the set matches the argument type
  const ir::Type *argType = actuals[argName].getType();
  assert(argType->isSet() && "argument is not a set");
  const ir::SetType *argSetType = setTypePtr(argType);
  auto &argFieldsMap = argSetType->getElementType()->getFields();

  for (const std::pair<std::string,int> &field : set->fieldNames) {
    assert(argFieldsMap.find(field.first) != argFieldsMap.end());

    SetBase::FieldData *fieldData = set->fields[field.second];

    const SetBase::FieldData::TensorType *setFieldType = fieldData->type;
    std::shared_ptr<ir::TensorType> argFieldType = argFieldsMap.at(field.first);

    assert(setFieldType->getComponentType()==argFieldType->getComponentType() &&
           "set type does not match function argument type");
    assert(setFieldType->getOrder() == argFieldType->getOrder() &&
           "set type does not match function argument type");

    const vector<ir::IndexDomain> &argFieldTypeDims =
        argFieldType->getDimensions();
    for (size_t i=0; i<argFieldType->getOrder(); ++i) {
      assert(argFieldTypeDims[i].getFactors().size() == 1 &&
             "set type does not match function argument type");

      size_t argFieldRange = argFieldTypeDims[i].getFactors()[0].getSize();

      assert(setFieldType->getDimension(i) == argFieldRange &&
             "set type does not match function argument type");
    }
  }

  actuals[argName].bind(set);
  initRequired = true;
}

void *Function::getFieldPtr(const SetBase *base, const std::string &fieldName) {
  assert(base->fieldNames.find(fieldName) != base->fieldNames.end());
  return base->fields[base->fieldNames.at(fieldName)]->data;
}

} // namespace simit
