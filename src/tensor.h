#ifndef SIMIT_TENSOR_H
#define SIMIT_TENSOR_H

#include <iostream>
#include <ostream>
#include <iomanip>

#include "error.h"
#include "util/compare.h"
#include "variadic.h"
#include "tensor_type.h"

namespace simit {

namespace ir {
class Type;
}

class Tensor {
public:
  Tensor(int val);
  Tensor(double val);

  Tensor(const ir::Type &type);
  Tensor(const ir::Type &type, void *data);

  Tensor(const Tensor &other);
  Tensor(Tensor &&other) noexcept;
  Tensor &operator=(const Tensor &other);
  Tensor &operator=(Tensor &&other) noexcept;
  ~Tensor();

  const ir::Type &getType() const;

  unsigned getSize() const;
  unsigned getComponentSize() const;

  void *getData();
  const void *getData() const;

  friend bool operator==(const Tensor&, const Tensor&);
  friend bool operator!=(const Tensor&, const Tensor&);

  friend std::ostream &operator<<(std::ostream &os, const Tensor &tensor);

private:
  struct Content;
  Content *content;
};

class Dynamic_Tensor {};

/// Non-templated Tensor base class that is stored in the IR
template <typename ComponentType=Dynamic_Tensor, int... Dimensions>
class DenseTensor {
public:
  DenseTensor() : data(new ComponentType[getSize()]) {
    std::fill(&data[0], &data[getSize()], ComponentType());
  }

  DenseTensor(const ComponentType &val) : data(new ComponentType(val)) {
    static_assert(getSize() == 1, "Using scalar constructor with non-scalar");
  }

  DenseTensor(const std::initializer_list<ComponentType>& vals) : DenseTensor(){
    std::copy(vals.begin(), vals.end(), data);
  }

  DenseTensor(const DenseTensor& other) : DenseTensor() {
    std::copy(&other.data[0], &other.data[getSize()], data);
  }

  DenseTensor(DenseTensor&& other) noexcept : data(other.data) {
    other.data = nullptr;
  }

  DenseTensor& operator=(const DenseTensor& other) {
    if (&other == this) {
      return *this;
    }
    DenseTensor tmp(other);
    *this = std::move(tmp);
    return *this;
  }

  DenseTensor& operator=(DenseTensor&& other) noexcept {
    std::swap(data, other.data);
    return *this;
  }
  
  ~DenseTensor() {
    delete[] data;
  }

  static constexpr size_t getOrder() {return sizeof...(Dimensions);}

  template <typename... Indices> inline
  ComponentType& operator()(Indices... indices) {
    static_assert(getOrder() > 0, "Indexing is not defined for scalars.");
    static_assert(sizeof...(indices) == sizeof...(Dimensions),
                  "Incorrect number of indices used to index tensor.");
    return data[util::computeOffset(util::seq<Dimensions...>(), indices...)];
  }

  template <typename... Indices> inline
  const ComponentType& operator()(Indices... indices) const {
    static_assert(getOrder() > 0, "Indexing is not defined for scalars.");
    static_assert(sizeof...(indices) == getOrder(),
                  "Incorrect number of indices used to index tensor.");
    return data[util::computeOffset(util::seq<Dimensions...>(), indices...)];
  }

  ComponentType& operator()(const std::vector<size_t>& indices) {
    iassert(indices.size() == getOrder())
        << "Incorrect number of indices used to index tensor.";
    return data[util::computeOffset(util::seq<Dimensions...>(), indices)];
  }

  const ComponentType& operator()(const std::vector<size_t>& indices) const {
    iassert(indices.size() == getOrder())
        << "Incorrect number of indices used to index tensor.";
    return data[util::computeOffset(util::seq<Dimensions...>(), indices)];
  }

  static constexpr size_t getSize() {
    return util::product<Dimensions...>::value;
  }

  static constexpr size_t getSizeInBytes() {
    return getSize() * sizeof(ComponentType);
  }

  const ComponentType* getData() const {return data;}
  ComponentType* getData() {return data;}

  TensorType getType() const{return computeType<ComponentType,Dimensions...>();}

private:
  ComponentType *data;
};

template <>
class DenseTensor<Dynamic_Tensor> {
public:
  DenseTensor(const TensorType &type) : type(type) {}

//  size_t getSize() {}

private:
  TensorType type;
};


/// Helper typedefs
typedef DenseTensor<float>  Float;
typedef DenseTensor<double> Double;
typedef DenseTensor<int>    Int;
typedef DenseTensor<bool>   Bool;

typedef DenseTensor<float,2>  Vector2f;
typedef DenseTensor<float,3>  Vector3f;
typedef DenseTensor<float,4>  Vector4f;

typedef DenseTensor<double,2> Vector2d;
typedef DenseTensor<double,3> Vector3d;
typedef DenseTensor<double,4> Vector4d;

typedef DenseTensor<int,2>    Vector2i;
typedef DenseTensor<int,3>    Vector3i;
typedef DenseTensor<int,4>    Vector4i;

typedef DenseTensor<bool,2>   Vector2b;
typedef DenseTensor<bool,3>   Vector3b;
typedef DenseTensor<bool,4>   Vector4b;

typedef DenseTensor<float,2,2>  Matrix2f;
typedef DenseTensor<float,3,3>  Matrix3f;
typedef DenseTensor<float,4,4>  Matrix4f;

typedef DenseTensor<double,2,2> Matrix2d;
typedef DenseTensor<double,3,3> Matrix3d;
typedef DenseTensor<double,4,4> Matrix4d;

typedef DenseTensor<int,2,2>    Matrix2i;
typedef DenseTensor<int,3,3>    Matrix3i;
typedef DenseTensor<int,4,4>    Matrix4i;

typedef DenseTensor<bool,2,2>   Matrix2b;
typedef DenseTensor<bool,3,3>   Matrix3b;
typedef DenseTensor<bool,4,4>   Matrix4b;


/// Compare two tensor data pointers.
bool compareTensors(const TensorType& ltype, const void *ldata,
                    const TensorType& rtype, const void *rdata);

/// Two tensors are equal if their type and all of their elements are equal.
template <typename ComponentType1, int... Dimensions1,
          typename ComponentType2, int... Dimensions2>
bool operator==(const DenseTensor<ComponentType1,Dimensions1...>& l,
                const DenseTensor<ComponentType2,Dimensions2...>& r) {
  return false;
}
template <typename ComponentType1, int... Dimensions>
bool operator==(const DenseTensor<ComponentType1,Dimensions...>& l,
                const DenseTensor<ComponentType1,Dimensions...>& r) {
  return compareTensors(l.getType(), l.getData(), r.getType(), r.getData());
}

/// Two tensors are unequal if their types or any of their elements are unequal.
template <typename ComponentType1, int... Dimensions1,
          typename ComponentType2, int... Dimensions2>
bool operator!=(const DenseTensor<ComponentType1,Dimensions1...>& l,
                const DenseTensor<ComponentType2,Dimensions2...>& r) {
  return true;
}
template <typename ComponentType1, int... Dimensions>
bool operator!=(const DenseTensor<ComponentType1,Dimensions...>& l,
                const DenseTensor<ComponentType1,Dimensions...>& r) {
  return !(l == r);
}

/// Print tensors to a stream.
template <typename ComponentType, int... Dimensions>
std::ostream&
operator<<(std::ostream& os, const DenseTensor<ComponentType,Dimensions...>& t){
  os << "tensor";
  return os;
}
template <typename ComponentType, int rows, int cols>
std::ostream&
operator<<(std::ostream& os, const DenseTensor<ComponentType,rows,cols>& t) {
  os << "matrix:\n";
  std::ios::fmtflags osflags(os.flags());
  os << std::setprecision(2) << std::fixed;
  if (0 < rows) {
    if (0 < cols) {
      os << t(0,0);
    }
    for (int j=1; j<cols; ++j) {
      os << " " << t(0,j);
    }
  }

  for (int i=1; i<rows; ++i) {
    os << "\n";
    if (0 < cols) {
      os << t(i,0);
    }
    for (int j=1; j<cols; ++j) {
      os << " " << t(i,j);
    }
  }
  os.flags(osflags);
  return os;
}
template <typename ComponentType, int size>
std::ostream& operator<<(std::ostream& os, const DenseTensor<ComponentType,size>& t){
  os << "[";
  std::ios::fmtflags osflags(os.flags());
  os << std::setprecision(2) << std::fixed;
  if (0 < size) {
    os << t(0);
  }
  for (int i=1; i<size; ++i) {
    os << " " << t(i);
  }
  os.flags(osflags);
  os << "]";
  return os;
}

}
#endif
