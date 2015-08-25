#include "function.h"

#include "backend/backend_function.h"
#include "types_convert.h"
#include "graph.h"  // TODO: should not need this include

using namespace std;

namespace simit {

// class Function
Function::Function() : Function(nullptr) {
}

Function::Function(backend::Function *func) : impl(func), funcPtr(nullptr) {
}

void Function::bind(string argument, const TensorType& ttype, void* data) {
#ifdef SIMIT_ASSERTS
  ir::Type type = ir::convert(ttype);
  ir::Type argType = impl->getArgType(argument);
  uassert(defined()) << "undefined function";
  uassert(impl->hasArg(argument)) << "no argument of this name in the function";
  uassert(type == argType)
      << "tensor type " << type
      << " does not match function argument type " << argType;
#endif

  return bind(argument, data);
}

void Function::bind(string argument, void* data) {
  uassert(defined()) << "undefined function";
  uassert(impl->hasArg(argument)) << "no argument of this name in the function";

  impl->bindTensor(argument, data);
}

void Function::bind(string argument, simit::Set *set) {
#ifdef SIMIT_ASSERTS
  uassert(defined()) << "undefined function";
  uassert(impl->hasArg(argument)) << "no argument of this name in the function";
  // Check that the set matches the argument type
  ir::Type argType = impl->getArgType(argument);
  uassert(argType.isSet()) << "Argument is not a set";
  const ir::SetType *argSetType = argType.toSet();
  const ir::ElementType *elemType = argSetType->elementType.toElement();

  // Type check
  for (size_t i=0; i < set->fields.size(); ++i) {
    Set::FieldData *fieldData = set->fields[i];
    uassert(elemType->hasField(fieldData->name))
        << "Field " << fieldData->name << " not found in set";

    const Set::FieldData::TensorType *setFieldType = fieldData->type;
    const ir::TensorType *elemFieldType =
        elemType->field(fieldData->name).type.toTensor();

    ir::ScalarType setFieldTypeComponentType;
    switch (setFieldType->getComponentType()) {
      case ComponentType::Float:
        setFieldTypeComponentType = ir::ScalarType(ir::ScalarType::Float);
        break;
      case ComponentType::Double:
        iassert(ir::ScalarType::floatBytes == sizeof(double));
        setFieldTypeComponentType = ir::ScalarType(ir::ScalarType::Float);
        break;
      case ComponentType::Int:
        setFieldTypeComponentType = ir::ScalarType(ir::ScalarType::Int);
        break;
      case ComponentType::Boolean:
        setFieldTypeComponentType = ir::ScalarType(ir::ScalarType::Boolean);
        break;
    }

    uassert(setFieldTypeComponentType == elemFieldType->componentType)
        << "field type"
        << "does not match function argument type" << *elemFieldType;

    uassert(setFieldType->getOrder() == elemFieldType->order())
        << "field type"
        << "does not match function argument type" << *elemFieldType;

    const vector<ir::IndexDomain> &argFieldTypeDims = elemFieldType->getDimensions();
    for (size_t i=0; i < elemFieldType->order(); ++i) {
      uassert(argFieldTypeDims[i].getIndexSets().size() == 1)
          << "field type"
          << "does not match function argument type" << *elemFieldType;

      size_t argFieldRange = argFieldTypeDims[i].getIndexSets()[0].getSize();

      uassert(setFieldType->getDimension(i) == argFieldRange)
          << "field type"
          << "does not match function argument type" << *elemFieldType;
    }
  }
#endif

  impl->bindSet(argument, set);
}

void Function::init() {
  uassert(defined()) << "undefined function";
  funcPtr = impl->init();
}

void Function::runSafe() {
  uassert(defined()) << "undefined function";
  if (!impl->isInitialized()) {
    init();
  }
  unmapArgs();
  funcPtr();
  mapArgs();
}

void Function::mapArgs() {
  uassert(defined()) << "undefined function";
  impl->mapArgs();
}

void Function::unmapArgs(bool updated) {
  uassert(defined()) << "undefined function";
  impl->unmapArgs(updated);
}

void Function::printMachine(std::ostream &os) {
  if (defined()) {
    impl->printMachine(os);
  }
}

std::ostream &operator<<(std::ostream &os, const Function &f) {
  if (f.defined()) {
    os << *f.impl;
  }
  return os;
}

}
