#include "function.h"

#include "backend/backend_function.h"
#include "types_convert.h"
#include "graph.h"  // TODO: should not need this include

using namespace std;

namespace simit {

// class Function
Function::Function() : Function(nullptr) {
}

Function::Function(backend::Function* func) : impl(func), funcPtr(nullptr) {
}

void Function::clear() {
  impl = nullptr;
}

void Function::bind(const std::string& name, simit::Set *set) {
#ifdef SIMIT_ASSERTS
  uassert(defined()) << "undefined function";
  uassert(impl->hasBindable(name))
      << "no argument or global " << util::quote(name) << " in function";
  // Check that the set matches the argument type
  ir::Type argType = impl->getBindableType(name);
  uassert(argType.isSet()) << "Argument is not a set";
  const ir::SetType *argSetType = argType.toSet();
  const ir::ElementType *elemType = argSetType->elementType.toElement();

  // Type check
  for (size_t i=0; i < set->fields.size(); ++i) {
    Set::FieldData *fieldData = set->fields[i];

    // Skip fields that are not defined in the Simit program
    if (!elemType->hasField(fieldData->name)) continue;

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
      case ComponentType::FloatComplex:
        setFieldTypeComponentType = ir::ScalarType(ir::ScalarType::Complex);
        break;
      case ComponentType::DoubleComplex:
        setFieldTypeComponentType = ir::ScalarType(ir::ScalarType::Complex);
        break;
    }

    uassert(setFieldTypeComponentType == elemFieldType->getComponentType())
        << "field type does not match function argument type "
        << util::quote(*elemFieldType)
        << "(" << name << "." << fieldData->name << ")";

    uassert(setFieldType->getOrder() == elemFieldType->order())
        << "field type does not match function argument type "
        << util::quote(*elemFieldType)
        << "(" << name << "." << fieldData->name << ")";

    const vector<ir::IndexDomain> &fieldDims = elemFieldType->getDimensions();
    for (size_t i=0; i < elemFieldType->order(); ++i) {
      uassert(fieldDims[i].getIndexSets().size() == 1)
          << "field type does not match function argument type "
          << util::quote(*elemFieldType);

      size_t argFieldRange = fieldDims[i].getIndexSets()[0].getSize();

      uassert(setFieldType->getDimension(i) == argFieldRange)
          << "field type does not match function argument type "
          << util::quote(*elemFieldType);
    }
  }
#endif

  impl->bind(name, set);
}

void Function::bind(const string& name, const TensorType& ttype, void* data) {
#ifdef SIMIT_ASSERTS
  uassert(defined()) << "undefined function";
  uassert(impl->hasBindable(name))
      << "no argument or global of this name in the function";
  ir::Type type = ir::convert(ttype);
  ir::Type argType = impl->getBindableType(name);
  uassert(type == argType)
      << "tensor type " << type
      << " does not match function argument type " << argType;
#endif
  return bind(name, data);
}

void Function::bind(const std::string& name, void* data) {
  uassert(defined()) << "undefined function";
  uassert(impl->hasBindable(name))
      << "no argument or global of this name in the function";
  impl->bind(name, data);
}

void Function::bind(const string& name, TensorData& data) {
  impl->bind(name, data);
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

void Function::print(std::ostream& os) const {
  if (defined()) {
    os << *impl;
  }
}

void Function::printMachine(std::ostream& os) const {
  if (defined()) {
    impl->printMachine(os);
  }
}

std::ostream& operator<<(std::ostream& os, const Function& f) {
  f.print(os);
  return os;
}

}
