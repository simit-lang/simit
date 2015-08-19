#include "types_convert.h"

using namespace std;

namespace simit {
namespace ir {

ScalarType convert(ComponentType componentType) {
  switch (componentType) {
    case ComponentType::Float:
      return ScalarType::Float;
    case ComponentType::Int:
      return ScalarType::Int;
    case ComponentType::Boolean:
      return ScalarType::Boolean;
  }
}

ComponentType convert(ScalarType scalarType) {
  switch (scalarType.kind) {
    case ScalarType::Float:
      return ComponentType::Float;
    case ScalarType::Int:
      return ComponentType::Int;
    case ScalarType::Boolean:
      return ComponentType::Boolean;
  }
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
    return simit::TensorType(convert(blockTensorType->componentType),
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
