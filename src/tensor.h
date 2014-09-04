#ifndef SIMIT_TENSOR_H
#define SIMIT_TENSOR_H

#include "tensor_components.h"

// TODO: Remove
#include <iostream>
using namespace std;

namespace simit {

/** Helper to convert from C++ type to Simit Type. */
template <typename T>
inline Type typeOf() { return Type::ELEMENT; }

template<>
inline Type typeOf<double>() { return Type::FLOAT; }
  
template<>
inline Type typeOf<int>() { return Type::INT; }

/// A tensor is a generalization of scalars, vectors and matrices. Tensors can
/// be fields of \ref Set instances and can be bound to \ref Program instances.
template <Type type, size_t order>
class TensorBase {
 public:
  inline size_t getOrder() const { return order; }

  size_t getDimension(size_t i) { assert(i < order); return dims[i]; }

 protected:
  TensorBase() {}
  ~TensorBase() {}

  size_t dims[order];

  void *data;
  int dataSize;

  void initData() {}
};

template <>
class TensorBase<INT, 0> {
 public:
  inline size_t getOrder() const { return 0; }

 protected:
  TensorBase() {}
  ~TensorBase() {}

  int data;
};

template <>
class TensorBase<FLOAT, 0> {
 public:
  inline size_t getOrder() const { return 0; }

 protected:
  TensorBase() {}
  ~TensorBase() {}

  double data;
};

template <>
class TensorBase<ELEMENT, 0> {
 public:
  inline size_t getOrder() const { return 0; }

 protected:
  TensorBase() {
    // TODO: Init element data
  }
  ~TensorBase() {}

  void *data;
  int dataSize;
};

template <Type type, size_t order>
class Tensor : public TensorBase<type, order> {
 public:
  Tensor() : TensorBase<type, order>() {}

  template<typename... Dims>
  Tensor(Dims... dims) : TensorBase<type, order>() {
    initDims(0, dims...);
    this->initData();
  }

 private:
  template<typename... Dims>
  void initDims(size_t i, size_t d, Dims... dims) {
    this->dims[i] = d;
    initDims(i+1, dims...);
  }

  template<typename... Dims>
  void initDims(size_t i, size_t d) {
    this->dims[i] = d;
  }
};

template <Type type>
class Tensor<type, 0> : public TensorBase<type, 0> {
 public:
  explicit Tensor() {}
};

template <Type type>
class Tensor<type, 1> : public TensorBase<type, 1> {
 public:
  explicit Tensor(size_t d0) {
    this->dims[0] = d0;
    this->initData();
  }
};

template <Type type>
class Tensor<type, 2> : public TensorBase<type, 2> {
 public:
  explicit Tensor(size_t d0, size_t d1) {
    this->dims[0] = d0; this->dims[1] = d1;
    this->initData();
  }
};

template <Type type>
class Tensor<type, 3> : public TensorBase<type, 3> {
 public:
  explicit Tensor(size_t d0, size_t d1, size_t d2) {
    this->dims[0] = d0; this->dims[1] = d1; this->dims[2] = d2;
    this->initData();
  }
};

template <Type type> class Scalar : public Tensor<type, 0> {
 public:
  explicit Scalar() {}
};

template <Type type> class Vector : public Tensor<type, 1> {
 public:
  explicit Vector(size_t d0) : Tensor<type, 1>(d0) {}
};

template <Type type> class Matrix : public Tensor<type, 2> {
 public:
  explicit Matrix(size_t d0, size_t d1) : Tensor<type, 2>(d0, d1) {}
};


}

#endif
