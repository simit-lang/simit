#ifndef SIMIT_TENSOR_H
#define SIMIT_TENSOR_H

#include <cassert>

#include "tensor_components.h"

// TODO: Remove
#include <iostream>
using namespace std;

namespace simit {

/// A tensor is a generalization of scalars, vectors and matrices. Tensors can
/// be fields of \ref Set instances and can be bound to \ref Program instances.
class TensorBase {
 public:
  virtual Type getType() const = 0;
  virtual size_t getOrder() const = 0;
  virtual size_t getDimension(size_t i) const = 0;
};

template <typename T, size_t order>
class Tensor : public TensorBase {
 public:
//  template<typename... Dims>
//  Tensor(Dims... dims) : TensorBase<T>(dims...) {}

//  template<typename... Indices>
//  inline T &operator()(Indices... indices) {
//    return this->get(indices...);
//  }
//
//  template<typename... Indices>
//  inline const T &operator()(Indices... indices) const {
//    return this->get(indices...);
//  }

 private:

};

// Scalars
template <typename T>
class Tensor<T,0> : public TensorBase {
 public:
  inline explicit Tensor() {}
  inline explicit Tensor(T val) : val(val) {}

  Type getType() const { return typeOf<T>(); }
  size_t getOrder() const { return 0; }
  size_t getDimension(size_t i) const {
    assert(false && "scalars don't have dimensions");
  }

  inline Tensor<T,0> &operator= (T val) {
    this->val = val;
    return *this;
  }

  inline operator T() const {
    return this->val;
  }

 protected:
  T val;
};

template <typename T>
class Scalar : public Tensor<T, 0> {
 public:
  inline explicit Scalar() {}
  inline explicit Scalar(T val) : Tensor<T, 0>(val) {}

  inline Scalar<T> &operator= (T val) {
    Tensor<T,0>::operator=(val);
    return *this;
  }
};

class Int : public Scalar<int> {
 public:
  inline explicit Int() {}
  inline explicit Int(int val) : Scalar<int>(val) {}

  inline Int &operator= (int val) {
    Scalar<int>::operator=(val);
    return *this;
  }
};

class Double : public Scalar<double> {
 public:
  inline explicit Double() {}
  inline explicit Double(double val) : Scalar<double>(val) {}

  inline Double &operator= (double val) {
    Scalar<double>::operator=(val);
    return *this;
  }
};


// Vectors
template <typename T>
class Tensor<T,1> : public TensorBase {
 public:
  inline explicit Tensor(size_t d0) : dim{d0}{
    data = (double*)malloc(d0 * sizeof(T));
  }

  inline ~Tensor() {
    free(data);
  }

  Type getType() const { return typeOf<T>(); }
  size_t getOrder() const { return 1; }
  size_t getDimension(size_t i) const {
    assert(i == 0);
    return dim;
  }

  inline T &operator()(size_t i) {
    assert(i < dim);
    return data[i];
  }

  inline const T &operator()(size_t i) const {
    return operator()(i);
  }

 private:
  size_t dim;
  T *data;
};

template <typename T>
class Vector : public Tensor<T, 1> {
 public:
  explicit Vector(size_t d0) : Tensor<T, 1>(d0) {}
};


// Matrices
template <typename T>
class Tensor<T, 2> : public TensorBase {
 public:
  inline explicit Tensor(size_t d0, size_t d1) : dims{d0, d1} {
    data = (double*)malloc(d0 * d1 * sizeof(T));
  }

  inline ~Tensor() {
    free(data);
  }

  Type getType() const { return typeOf<T>(); }
  size_t getOrder() const { return 2; }
  size_t getDimension(size_t i) const {
    assert(i < 2);
    return dims[i];
  }

  inline T &operator()(size_t i, size_t j) {
    assert(i < dims[0] && j < dims[1]);
    return data[i*dims[1] + j];
  }

  inline const T &operator()(size_t i, size_t j) const {
    return operator()(i,j);
  }

 private:
  size_t dims[2];
  T *data;
};

template <typename T>
class Matrix : public Tensor<T, 2> {
 public:
  explicit Matrix(size_t d0, size_t d1) : Tensor<T, 2>(d0, d1) {}
};


template <typename T>
class Tensor<T, 3> : public TensorBase {
 public:
  inline explicit Tensor(size_t d0, size_t d1, size_t d2) : dims{d0, d1, d2} {
    data = (double*)malloc(d0 * d1 * d2 *sizeof(T));
  }

  inline ~Tensor() {
    free(data);
  }

  Type getType() const { return typeOf<T>(); }
  size_t getOrder() const { return 3; }
  size_t getDimension(size_t i) const {
    assert(i < 3);
    return dims[i];
  }

  inline T &operator()(size_t i, size_t j, size_t k) {
    assert(i < dims[0] && j < dims[1] && k < dims[2]);
    return data[i*dims[1]*dims[2] + j*dims[2] + k];
  }

  inline const T &operator()(size_t i, size_t j, size_t k) const {
    return operator()(i,j,k);
  }

 private:
  size_t dims[3];
  T *data;
};


}

#endif
