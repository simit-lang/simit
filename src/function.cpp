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
    formals.push_back(arg.getName());
    actuals[arg.getName()] = Actual(arg.getType());
  }
  for (auto &res : simitFunc.getResults()) {
    // Skip results that alias an argument
    if (actuals.find(res.getName()) != actuals.end()) {
      continue;
    }
    formals.push_back(res.getName());
    actuals[res.getName()] = Actual(res.getType());
  }
}

Function::~Function() {
}

void Function::bind(const std::string &argName, ir::Expr *tensor) {
  uassert(actuals.find(argName) != actuals.end())
      << "no argument of this name in function";

  // Check that the tensor matches the argument type
  uassert(tensor->type() == actuals[argName].getType())
      << "tensor type does not match function argument type";

  uassert(dynamic_cast<const ir::Literal*>(tensor->expr()) != nullptr);

  actuals[argName].bind(tensor);
  initRequired = true;
}

void Function::bind(const std::string &argName, SetBase *set) {
  uassert(actuals.find(argName) != actuals.end())
      << "No argument of this name in function";

  // Check that the set matches the argument type
  ir::Type argType = actuals[argName].getType();
  uassert(argType.isSet()) << "Argument is not a set";
  const ir::SetType *argSetType = argType.toSet();
//  auto &argFieldsMap = argSetType->elementType.toElement()->fields;
  const ir::ElementType *elemType = argSetType->elementType.toElement();

  // Type check
  for (size_t i=0; i < set->fields.size(); ++i) {
    SetBase::FieldData *fieldData = set->fields[i];
    uassert(elemType->hasField(fieldData->name)) << "Field not found in set";

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

    uassert(setFieldTypeComponentType == elemFieldType->componentType)
        << "Set type does not match function argument type";
    uassert(setFieldType->getOrder() == elemFieldType->order())
        << "Set type does not match function argument type";

    const vector<ir::IndexDomain> &argFieldTypeDims = elemFieldType->dimensions;
    for (size_t i=0; i < elemFieldType->order(); ++i) {
      uassert(argFieldTypeDims[i].getIndexSets().size() == 1)
          << "Set type does not match function argument type";

      size_t argFieldRange = argFieldTypeDims[i].getIndexSets()[0].getSize();

      uassert(setFieldType->getDimension(i) == argFieldRange)
          << "Set type does not match function argument type";
    }
  }

  actuals[argName].bind(set);
  initRequired = true;
}

void *Function::getFieldPtr(const SetBase *set, const std::string &fieldName) {
  iassert(set->fieldNames.find(fieldName) != set->fieldNames.end());
  return set->fields[set->fieldNames.at(fieldName)]->data;
}

int *Function::getEndpointsPtr(const SetBase *set) {
  iassert(set->endpoints != nullptr);
  return set->endpoints;
}

} // namespace simit
