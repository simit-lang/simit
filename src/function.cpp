#include "function.h"

#include <iostream>
using namespace std;

#include "graph.h"
#include "types.h"
#include "ir.h"
#include "types.h"

namespace simit {

Function::Function(const simit::ir::Func &simitFunc)
    : funcPtr(NULL), initRequired(true) {
  for (auto &argument : simitFunc.getArguments()) {
    std::string argName = toVariable(argument)->name;
    formals.push_back(argName);
    actuals[argName] = Actual(argument.type());
  }
  for (auto &result : simitFunc.getResults()) {
    // Skip results that alias an argument
    std::string name = toVariable(result)->name;
    if (actuals.find(name) != actuals.end()) {
      continue;
    }
    formals.push_back(name);
    actuals[name] = Actual(result.type());
  }
}

Function::~Function() {
}

void Function::bind(const std::string &argName, ir::Expr *tensor) {
  assert(actuals.find(argName) != actuals.end() &&
         "no argument of this name in function");

  // Check that the tensor matches the argument type
  assert(tensor->type() == actuals[argName].getType() &&
         "tensor type does not match function argument type");

  assert(dynamic_cast<const ir::Literal*>(tensor->expr()) != nullptr);

  actuals[argName].bind(tensor);
  initRequired = true;
}

void Function::bind(const std::string &argName, SetBase *set) {
  assert(actuals.find(argName) != actuals.end() &&
         "no argument of this name in function");

  // Check that the set matches the argument type
  ir::Type argType = actuals[argName].getType();
  assert(argType.isSet() && "argument is not a set");
  const ir::SetType *argSetType = argType.toSet();
  auto &argFieldsMap = argSetType->elementType.toElement()->fields;

  for (const std::pair<std::string,int> &field : set->fieldNames) {
    assert(argFieldsMap.find(field.first) != argFieldsMap.end());

    SetBase::FieldData *fieldData = set->fields[field.second];

    const SetBase::FieldData::TensorType *setFieldType = fieldData->type;
    const ir::TensorType *argFieldType = argFieldsMap.at(field.first).toTensor();

    ir::Type setFieldTypeComponentType;
    switch (setFieldType->getComponentType()) {
      case ComponentType::INT:
        setFieldTypeComponentType = ir::Int(32);
        break;
      case ComponentType::FLOAT:
        setFieldTypeComponentType = ir::Float(32);
        break;
    }

    assert(setFieldTypeComponentType == argFieldType->componentType &&
           "set type does not match function argument type");
    assert(setFieldType->getOrder() == argFieldType->order() &&
           "set type does not match function argument type");

    const vector<ir::IndexDomain> &argFieldTypeDims = argFieldType->dimensions;
    for (size_t i=0; i < argFieldType->order(); ++i) {
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
