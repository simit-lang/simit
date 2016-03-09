#include "types_convert.h"

using namespace std;

namespace simit {
namespace ir {

ScalarType convert(ComponentType componentType) {
  switch (componentType) {
    case ComponentType::Float:
      iassert(ir::ScalarType::floatBytes == sizeof(float));
      return ScalarType::Float;
    case ComponentType::Double:
      iassert(ir::ScalarType::floatBytes == sizeof(double));
      return ScalarType::Float;
    case ComponentType::Int:
      return ScalarType::Int;
    case ComponentType::Boolean:
      return ScalarType::Boolean;
    case ComponentType::FloatComplex:
      iassert(ir::ScalarType::floatBytes == sizeof(float));
      return ScalarType::Complex;
    case ComponentType::DoubleComplex:
      iassert(ir::ScalarType::floatBytes == sizeof(double));
      return ScalarType::Complex;
  }
}

ComponentType convert(ScalarType scalarType) {
  switch (scalarType.kind) {
    case ScalarType::Float:
      if (ir::ScalarType::floatBytes == sizeof(float)) {
        return ComponentType::Float;
      }
      else if (ir::ScalarType::floatBytes == sizeof(double)) {
        return ComponentType::Double;
      }
      else {
        not_supported_yet;
        return ComponentType::Double;
      }
    case ScalarType::Int:
      return ComponentType::Int;
    case ScalarType::Boolean:
      return ComponentType::Boolean;
    case ScalarType::Complex:
      if (ir::ScalarType::floatBytes == sizeof(float)) {
        return ComponentType::FloatComplex;
      }
      else if (ir::ScalarType::floatBytes == sizeof(double)) {
        return ComponentType::DoubleComplex;
      }
      else {
        not_supported_yet;
        return ComponentType::DoubleComplex;
      }
    case ScalarType::String:
      break;
  }
  unreachable;
  return ComponentType::Int;
}

vector<IndexDomain> getUnblockedDimensions(const simit::TensorType &tensorType){
  vector<IndexDomain> result;
  if (tensorType.isBlocked()) {
    result = getUnblockedDimensions(tensorType.getBlockType());
  }
  if (tensorType.getOrder() > result.size()) {
    result.resize(tensorType.getOrder());
  }
  for (size_t i=0; i < tensorType.getOrder(); ++i) {
    result[i] = result[i] * IndexSet(tensorType.getDimension(i));
  }
  return result;
}

Type convert(const simit::TensorType &tensorType) {
  vector<IndexDomain> dimensions = getUnblockedDimensions(tensorType);
  return TensorType::make(convert(tensorType.getComponentType()), dimensions);
}

static simit::TensorType convert(const TensorType &tensorType) {
  auto outerDimensions = tensorType.getOuterDimensions();
  std::vector<int> dimensions;
  dimensions.reserve(outerDimensions.size());
  for (const IndexSet &dim : tensorType.getOuterDimensions()) {
    iassert(dim.getKind() == IndexSet::Range);
    dimensions.push_back(dim.getSize());
  }

  Type blockType = tensorType.getBlockType();
  iassert(blockType.isTensor());
  auto blockTensorType = blockType.toTensor();
  if (blockTensorType->order() == 0) {
    return simit::TensorType(convert(blockTensorType->getComponentType()),
                             dimensions);
  }
  else {
    return simit::TensorType(convert(blockType), dimensions);;
  }
}

simit::TensorType convert(const Type &type) {
  iassert(type.isTensor());
  const TensorType* tensorType = type.toTensor();
  return convert(*tensorType);
}

}}
