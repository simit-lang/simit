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
  simit_uassert(defined()) << "undefined function";
  simit_uassert(impl->hasBindable(name))
      << "no argument or global " << util::quote(name) << " in function";
  // Check that the set matches the argument type
  ir::Type argType = impl->getBindableType(name);
  simit_uassert(argType.isSet()) << "Argument is not a set";
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

    const string fieldTypeErrorString =
        name + "." + fieldData->name + " type does not match Simit type " +
        util::quote(*elemFieldType);

    simit_uassert(setFieldTypeComponentType == elemFieldType->getComponentType() &&
            setFieldType->getOrder() == elemFieldType->order())
        << fieldTypeErrorString;

    const vector<ir::IndexDomain> &fieldDims = elemFieldType->getDimensions();
    for (size_t i=0; i < elemFieldType->order(); ++i) {
      simit_uassert(fieldDims[i].getIndexSets().size() == 1)
          << fieldTypeErrorString;

      size_t argFieldRange = fieldDims[i].getIndexSets()[0].getSize();

      simit_uassert(setFieldType->getDimension(i) == argFieldRange)
          << fieldTypeErrorString;
    }
  }
#endif

  impl->bind(name, set);
}

void Function::bind(const string& name, const TensorType& ttype, void* data) {
#ifdef SIMIT_ASSERTS
  simit_uassert(defined()) << "undefined function";
  simit_uassert(impl->hasBindable(name))
      << "no argument or global of this name in the function";
  ir::Type type = ir::convert(ttype);
  ir::Type argType = impl->getBindableType(name);
  simit_uassert(type == argType)
      << "tensor type " << type
      << " does not match function argument type " << argType;
#endif
  return bind(name, data);
}

void Function::bind(const std::string& name, void* data) {
  simit_uassert(defined()) << "undefined function";
  simit_uassert(impl->hasBindable(name))
      << "no argument or global of this name in the function";
  impl->bind(name, data);
}

void Function::bind(const string& name, TensorData& data) {
  impl->bind(name, data);
}

void Function::init() {
  simit_uassert(defined()) << "undefined function";
  funcPtr = impl->init();
}

void Function::runSafe() {
  simit_uassert(defined()) << "undefined function";
  if (!impl->isInitialized()) {
    init();
  }
  unmapArgs();
  funcPtr();
  mapArgs();
}

void Function::mapArgs() {
  simit_uassert(defined()) << "undefined function";
  impl->mapArgs();
}

void Function::unmapArgs(bool updated) {
  simit_uassert(defined()) << "undefined function";
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
