#ifndef SIMIT_TENSOR_H
#define SIMIT_TENSOR_H

#include <iostream>
#include <ostream>
#include <iomanip>

#include "error.h"
#include "tensor_type.h"
#include "util/compare.h"
#include "util/variadic.h"

namespace simit {

class Dynamic_Tensor {};

/// Non-templated Tensor base class that is stored in the IR
template <typename ComponentType=Dynamic_Tensor, int... dimensions>
class Tensor {
public:
  Tensor() : data(new ComponentType[getSize()]) {
    std::fill(&data[0], &data[getSize()], ComponentType());
  }

  Tensor(ComponentType val) : data(new ComponentType(val)) {
    static_assert(getOrder() == 0, "Using scalar constructor with non-scalar");
  }

  Tensor(const std::initializer_list<ComponentType>& vals) : Tensor(){
    std::copy(vals.begin(), vals.end(), data);
  }

  Tensor(const Tensor& other) : Tensor() {
    std::copy(&other.data[0], &other.data[getSize()], data);
  }

  Tensor(Tensor&& other) noexcept : data(other.data) {
    other.data = nullptr;
  }

  Tensor& operator=(const Tensor& other) {
    if (&other == this) {
      return *this;
    }
    Tensor tmp(other);
    *this = std::move(tmp);
    return *this;
  }

  Tensor& operator=(Tensor&& other) noexcept {
    std::swap(data, other.data);
    return *this;
  }
  
  ~Tensor() {
    delete[] data;
  }

  static constexpr size_t getOrder() {return sizeof...(dimensions);}

  /// Convert scalar tensors to their component type
  inline operator ComponentType() const {
    static_assert(getOrder()==0,
                  "Can only convert scalar tensors to scalar values.");
    return data[0];
  }

  /// Methods to index into the tensor
  template <typename... Indices> inline
  ComponentType& operator()(Indices... indices) {
    static_assert(getOrder() > 0, "Indexing is not defined for scalars.");
    static_assert(sizeof...(indices) == sizeof...(dimensions),
                  "Incorrect number of indices used to index tensor.");
    return data[util::computeOffset(util::seq<dimensions...>(), indices...)];
  }

  template <typename... Indices> inline
  const ComponentType& operator()(Indices... indices) const {
    static_assert(getOrder() > 0, "Indexing is not defined for scalars.");
    static_assert(sizeof...(indices) == getOrder(),
                  "Incorrect number of indices used to index tensor.");
    return data[util::computeOffset(util::seq<dimensions...>(), indices...)];
  }

  ComponentType& operator()(const std::vector<size_t>& indices) {
    simit_iassert(indices.size() == getOrder())
        << "Incorrect number of indices used to index tensor.";
    return data[util::computeOffset(util::seq<dimensions...>(), indices)];
  }

  const ComponentType& operator()(const std::vector<size_t>& indices) const {
    simit_iassert(indices.size() == getOrder())
        << "Incorrect number of indices used to index tensor.";
    return data[util::computeOffset(util::seq<dimensions...>(), indices)];
  }

  static constexpr size_t getSize() {
    return util::product<dimensions...>::value;
  }

  static constexpr size_t getSizeInBytes() {
    return getSize() * sizeof(ComponentType);
  }

  const ComponentType* getData() const {return data;}
  ComponentType* getData() {return data;}

  TensorType getType() const{return computeType<ComponentType,dimensions...>();}

private:
  ComponentType *data;
};

// TODO: Add dynamic tensor class
template <>
class Tensor<Dynamic_Tensor> {
public:
  Tensor(const TensorType &type) : type(type) {}

//  size_t getSize() {}

private:
  TensorType type;
};


/// Helper typedefs
typedef Tensor<float>          Float;
typedef Tensor<double>         Double;
typedef Tensor<int>            Int;
typedef Tensor<bool>           Bool;
typedef Tensor<float_complex>  FloatComplex;
typedef Tensor<double_complex> DoubleComplex;

typedef Tensor<float,2>          Vector2f;
typedef Tensor<float,3>          Vector3f;
typedef Tensor<float,4>          Vector4f;

typedef Tensor<double,2>         Vector2d;
typedef Tensor<double,3>         Vector3d;
typedef Tensor<double,4>         Vector4d;

typedef Tensor<int,2>            Vector2i;
typedef Tensor<int,3>            Vector3i;
typedef Tensor<int,4>            Vector4i;

typedef Tensor<bool,2>           Vector2b;
typedef Tensor<bool,3>           Vector3b;
typedef Tensor<bool,4>           Vector4b;

typedef Tensor<float_complex,2>  Vector2fc;
typedef Tensor<float_complex,3>  Vector3fc;
typedef Tensor<float_complex,4>  Vector4fc;

typedef Tensor<double_complex,2> Vector2dc;
typedef Tensor<double_complex,3> Vector3dc;
typedef Tensor<double_complex,4> Vector4dc;

typedef Tensor<float,2,2>          Matrix2f;
typedef Tensor<float,3,3>          Matrix3f;
typedef Tensor<float,4,4>          Matrix4f;

typedef Tensor<double,2,2>         Matrix2d;
typedef Tensor<double,3,3>         Matrix3d;
typedef Tensor<double,4,4>         Matrix4d;

typedef Tensor<int,2,2>            Matrix2i;
typedef Tensor<int,3,3>            Matrix3i;
typedef Tensor<int,4,4>            Matrix4i;

typedef Tensor<bool,2,2>           Matrix2b;
typedef Tensor<bool,3,3>           Matrix3b;
typedef Tensor<bool,4,4>           Matrix4b;

typedef Tensor<float_complex,2,2>  Matrix2fc;
typedef Tensor<float_complex,3,3>  Matrix3fc;
typedef Tensor<float_complex,4,4>  Matrix4fc;

typedef Tensor<double_complex,2,2> Matrix2dc;
typedef Tensor<double_complex,3,3> Matrix3dc;
typedef Tensor<double_complex,4,4> Matrix4dc;


/// Compare two tensor data pointers.
bool compareTensors(const TensorType& ltype, const void *ldata,
                    const TensorType& rtype, const void *rdata);


/// Two tensors are equal if their type and all of their elements are equal.
template <typename ComponentType1, int... dimensions1,
          typename ComponentType2, int... dimensions2>
bool operator==(const Tensor<ComponentType1,dimensions1...>& l,
                const Tensor<ComponentType2,dimensions2...>& r) {
  return false;
}
template <typename ComponentType1, int... dimensions>
bool operator==(const Tensor<ComponentType1,dimensions...>& l,
                const Tensor<ComponentType1,dimensions...>& r) {
  return compareTensors(l.getType(), l.getData(), r.getType(), r.getData());
}
template <typename ComponentType>
bool operator==(const Tensor<ComponentType>& l, const ComponentType& r) {
  return l == Tensor<ComponentType>(r);
}
template <typename ComponentType>
bool operator==(const ComponentType& l, const Tensor<ComponentType>& r) {
  return Tensor<ComponentType>(l) == r;
}

/// Two tensors are unequal if their types or any of their elements are unequal.
template <typename ComponentType1, int... dimensions1,
          typename ComponentType2, int... dimensions2>
bool operator!=(const Tensor<ComponentType1,dimensions1...>& l,
                const Tensor<ComponentType2,dimensions2...>& r) {
  return true;
}
template <typename ComponentType1, int... dimensions>
bool operator!=(const Tensor<ComponentType1,dimensions...>& l,
                const Tensor<ComponentType1,dimensions...>& r) {
  return !(l == r);
}
template <typename ComponentType>
bool operator!=(const Tensor<ComponentType>& l, const ComponentType& r) {
  return !(l == r);
}
template <typename ComponentType>
bool operator!=(const ComponentType& l, const Tensor<ComponentType>& r) {
  return !(l == r);
}

/// Print tensors to a stream.
template <typename ComponentType, int... dimensions> std::ostream&
operator<<(std::ostream& os, const Tensor<ComponentType,dimensions...>& t) {
  const std::string sep = " ";
  TensorType type = t.getType();

  std::ios::fmtflags osflags(os.flags());
  if (type.getOrder() == 0) {
    simit_iassert(t.getOrder() == 0);
    os << t({});
  }
  else if (type.getOrder() == 1) {
    os << std::setprecision(2) << std::fixed;
    size_t size = type.getDimension(0);
    os << "[";
    if (size > 0) {
      os << t({0});
    }
    for (size_t i=1; i<size; ++i) {
      os << sep << t({i});
    }
    os << "]";
  }
  else if (type.getOrder() == 2) {
    os << std::setprecision(2) << std::fixed;
    size_t rows = type.getDimension(0);
    size_t cols = type.getDimension(1);
    os << "[";
    if (0 < rows) {
      if (0 < cols) {
        os << t({0,0});
      }
      for (size_t j=1; j<cols; ++j) {
        os << sep << t({0,j});
      }
    }
    for (size_t i=1; i<rows; ++i) {
      os << "; ";
      if (0 < cols) {
        os << t({i,0});
      }
      for (size_t j=1; j<cols; ++j) {
        os << sep << t({i,j});
      }
    }
    os << "]";

  }
  else {
    os << "tensor";
  }
  os.flags(osflags);
  return os;
}

}
#endif
