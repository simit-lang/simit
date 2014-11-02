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
  for (auto &arg : simitFunc.getArguments()) {
    formals.push_back(arg.name);
    actuals[arg.name] = Actual(arg.type);
  }
  for (auto &res : simitFunc.getResults()) {
    // Skip results that alias an argument
    if (actuals.find(res.name) != actuals.end()) {
      continue;
    }
    formals.push_back(res.name);
    actuals[res.name] = Actual(res.type);
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
//  auto &argFieldsMap = argSetType->elementType.toElement()->fields;
  const ir::ElementType *elemType = argSetType->elementType.toElement();

  // Type check
  for (size_t i=0; i < set->fields.size(); ++i) {
    SetBase::FieldData *fieldData = set->fields[i];
    assert(elemType->hasField(fieldData->name) && "Field not found in set");


    const SetBase::FieldData::TensorType *setFieldType = fieldData->type;
    const ir::TensorType *elemFieldType =
        elemType->field(fieldData->name).type.toTensor();

    ir::ScalarType setFieldTypeComponentType;
    switch (setFieldType->getComponentType()) {
      case ComponentType::INT:
        setFieldTypeComponentType = ir::ScalarType(ir::ScalarType::Int);
        break;
      case ComponentType::FLOAT:
        setFieldTypeComponentType = ir::ScalarType(ir::ScalarType::Float);
        break;
    }

    assert(setFieldTypeComponentType == elemFieldType->componentType &&
           "set type does not match function argument type");
    assert(setFieldType->getOrder() == elemFieldType->order() &&
           "set type does not match function argument type");

    const vector<ir::IndexDomain> &argFieldTypeDims = elemFieldType->dimensions;
    for (size_t i=0; i < elemFieldType->order(); ++i) {
      assert(argFieldTypeDims[i].getIndexSets().size() == 1 &&
             "set type does not match function argument type");

      size_t argFieldRange = argFieldTypeDims[i].getIndexSets()[0].getSize();

      assert(setFieldType->getDimension(i) == argFieldRange &&
             "set type does not match function argument type");
    }
  }

  actuals[argName].bind(set);
  initRequired = true;
}

void *Function::getFieldPtr(const SetBase *set, const std::string &fieldName) {
  assert(set->fieldNames.find(fieldName) != set->fieldNames.end());
  return set->fields[set->fieldNames.at(fieldName)]->data;
}

int *Function::getEndpointsPtr(const SetBase *set) {
  assert(set->endpoints != nullptr);
  return set->endpoints;
}

} // namespace simit
