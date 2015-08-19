#ifndef SIMIT_TENSOR_TYPE_H
#define SIMIT_TENSOR_TYPE_H

#include <climits>
#include <string>

#include "error.h"
#include "types.h"

namespace simit {

/** The types of supported tensor components. */
enum class ComponentType {Float, Int, Boolean };

/** Helper to convert from C++ type to Simit Type. */
template<typename T> inline ComponentType typeOf() {
  ierror << "Unsupported type";
  return ComponentType::Int; // TODO XXX gcc warning suppression
}

template<> inline ComponentType typeOf<int>() {
  return ComponentType::Int;
}

template<> inline ComponentType typeOf<float>() {
  iassert(ir::ScalarType::floatBytes == 4);
  return ComponentType::Float;
}

template<> inline ComponentType typeOf<double>() {
  iassert(ir::ScalarType::floatBytes == 8);
  return ComponentType::Float;
}

template<> inline ComponentType typeOf<bool>() {
  return ComponentType::Boolean;
}

inline std::size_t componentSize(ComponentType ct) {
  switch (ct) {
    case ComponentType::Int:
      return sizeof(int);
    case ComponentType::Float:
      return ir::ScalarType::floatBytes;
    case ComponentType::Boolean:
      return sizeof(bool);
  }
  unreachable;
  return 0;
}

std::ostream& operator<<(std::ostream& os, ComponentType componentType);


class TensorType {
public:
  TensorType(ComponentType componentType,
             std::vector<int> dimensions={})
      : blocked(false), componentType(componentType), dimensions(dimensions) {}

  TensorType(const TensorType &blockType, std::vector<int> dimensions)
      : blocked(true), blockType(new TensorType(blockType)),
        dimensions(dimensions) {}

  bool isBlocked() const {return blocked;}

  TensorType getBlockType() const;
  ComponentType getComponentType() const;

  size_t getOrder() const { return dimensions.size(); }
  size_t getDimension(size_t i) const {
    iassert(i<getOrder());
    return dimensions[i];
  }

  size_t getSize() const;

private:
  bool blocked;
  union {
    TensorType*   blockType;
    ComponentType componentType;
  };

  std::vector<int> dimensions;
};

std::ostream& operator<<(std::ostream& os, const TensorType& tensorType);

/// Template function to statically compute a TensorType from a template tensor
/// type descriptor.
template <typename ComponentType, int... Dimensions>
inline TensorType computeType() {
  TensorType subType = computeType<ComponentType>();
  return (subType.isBlocked())
          ? TensorType(subType, {Dimensions...})
          : TensorType(subType.getComponentType(), {Dimensions...});
}
template<> inline TensorType computeType<double>() {
  return simit::ComponentType::Float;
}
template<> inline TensorType computeType<float>() {
  return simit::ComponentType::Float;
}
template<> inline TensorType computeType<int>() {
  return simit::ComponentType::Int;
}
template<> inline TensorType computeType<bool>() {
  return simit::ComponentType::Boolean;
}


bool operator==(const TensorType& l, const TensorType& r);
bool operator!=(const TensorType& l, const TensorType& r);

}

#endif
