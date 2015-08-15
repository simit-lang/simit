#ifndef SIMIT_TENSOR_H
#define SIMIT_TENSOR_H

#include <iostream>
#include <ostream>
#include <iomanip>

#include "util/compare.h"
#include "variadic.h"

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

template <typename ComponentType, int... Dimensions>
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

  static size_t getSize() {return util::product<Dimensions...>::value;}

  template <typename... Indices> inline
  ComponentType& operator()(Indices... index) {
    static_assert(sizeof...(index) == sizeof...(Dimensions),
                  "Incorrect number of indices used to index tensor");
    return data[util::computeOffset(util::seq<Dimensions...>(), index...)];
  }

  template <typename... Indices> inline
  const ComponentType& operator()(Indices... index) const {
    static_assert(sizeof...(index) == sizeof...(Dimensions),
                  "Incorrect number of indices used to index tensor");
    return data[util::computeOffset(util::seq<Dimensions...>(), index...)];
  }

  const ComponentType* getData() const {return data;}
  ComponentType* getData() {return data;}

private:
  ComponentType *data;
};

template <int... Dimensions>
bool operator==(const DenseTensor<float,Dimensions...>& l,
                const DenseTensor<float,Dimensions...>& r) {
  const float* ldata = l.getData();
  const float* rdata = r.getData();
  for (size_t i=0; i < l.getSize(); ++i) {
    if (!util::almostEqual(ldata[i], rdata[i], 0.0001f)) {
      return false;
    }
  }
  return true;
}

template <int... Dimensions>
bool operator==(const DenseTensor<double,Dimensions...>& l,
                const DenseTensor<double,Dimensions...>& r) {
  const double* ldata = l.getData();
  const double* rdata = r.getData();
  for (size_t i=0; i < l.getSize(); ++i) {
    if (!util::almostEqual(ldata[i], rdata[i], 0.0001)) {
      return false;
    }
  }
  return true;
}

template <typename ComponentType, int... Dimensions>
bool operator==(const DenseTensor<ComponentType,Dimensions...>& l,
                const DenseTensor<ComponentType,Dimensions...>& r) {
  const ComponentType* ldata = l.getData();
  const ComponentType* rdata = r.getData();
  for (size_t i=0; i < l.getSize(); ++i) {
    if (ldata[i] != rdata[i]) {
      return false;
    }
  }
  return true;
}

/// Tensors with different types are different
template <typename ComponentType1, int... Dimensions1,
          typename ComponentType2, int... Dimensions2>
bool operator==(const DenseTensor<ComponentType1,Dimensions1...>& l,
                const DenseTensor<ComponentType2,Dimensions2...>& r) {
  return false;
}

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
std::ostream&
operator<<(std::ostream& os, const DenseTensor<ComponentType,size>& t) {
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
