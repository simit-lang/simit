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
template <typename T, size_t order>
class TensorBase {
 public:
  inline Type getType() { typeOf<T>(); }

  inline size_t getOrder() const { return order; }
  inline size_t getDimension(size_t i) { assert(i < order); return dims[i]; }


 protected:
  inline TensorBase() {}
  inline TensorBase(size_t d0) : dims{d0} { init(); }
  inline TensorBase(size_t d0, size_t d1) : dims{d0,d1} { init(); }
  inline TensorBase(size_t d0, size_t d1, size_t d2) : dims{d0,d1,d2} { init();}

  template<typename... Dims>
  inline TensorBase(Dims... dims) { initDims(0u, dims...); init(); };

  ~TensorBase() { free(data); }

  inline T &get(size_t i) {
    return static_cast<T*>(this->data)[i];
  }

  inline T &get(size_t i, size_t j) {
    assert(i < this->dims[0] && j < this->dims[1]);
    return static_cast<T*>(this->data)[i*this->dims[1] + j];
  }

  inline T &get(size_t i, size_t j, size_t k) {
    assert(i < this->dims[0] && j < this->dims[1] && k < this->dims[2]);
    return static_cast<T*>(this->data)[i*this->dims[1]*this->dims[2] +
                                       j*this->dims[2] + k];
  }

  template<typename... Indices>
  inline T &get(Indices... indices) {
    return static_cast<T*>(this->data)[computeOffset(0, indices...)];
  }

 private:
  void *data;
  Type type;
  size_t dims[order];

  void init() {
    size_t numElements = 1;
    for (size_t i=0; i<order; ++i) {
      numElements *= dims[i];
    }
    data = malloc(numElements * sizeof(T));
  }

  template<typename... Dims>
  void initDims(size_t i, size_t d, Dims... dims) {
    this->dims[i] = d;
    initDims(i+1, dims...);
  }

  template<typename... Dims>
  void initDims(size_t i, size_t d) {
    this->dims[i] = d;
  }

  template<typename... Indices>
  inline size_t computeOffset(size_t i, size_t idx) {
    assert(idx < dims[i]);
    return i;
  }

  template<typename... Indices>
  inline size_t computeOffset(size_t i, size_t idx, Indices... indices) {
    assert(idx < dims[i]);
    size_t stride = 1;
    for (size_t j=i+1; j<order; ++j) {
      stride *= dims[j];
    }
    return stride*idx + computeOffset(i+1, indices...);
  }
};


template <typename T>
class TensorBase<T, 0> {
 public:
  inline size_t getOrder() const { return 0; }

 protected:
  TensorBase() {}
  ~TensorBase() {}

  T val;
};

template <typename T, size_t order>
class Tensor : public TensorBase<T, order> {
 public:
  template<typename... Dims>
  Tensor(Dims... dims) : TensorBase<T, order>(dims...) {}

  template<typename... Indices>
  inline T &operator()(Indices... indices) {
    return this->get(indices...);
  }

  template<typename... Indices>
  inline const T &operator()(Indices... indices) const {
    return this->get(indices...);
  }
  
 private:

};

template <typename T>
class Tensor<T,0> : public TensorBase<T,0> {
 public:
  explicit Tensor() {}
  Tensor(T val) { this->val = val; }

  inline void operator= (const T &val) { this->val = val; }
  inline operator T() const { return this->val; }
};

template <typename T>
class Tensor<T,1> : public TensorBase<T,1> {
 public:
  explicit Tensor(size_t d0) : TensorBase<T,1>(d0) {}

  inline T &operator()(size_t i) {
    return this->get(i);
  }

  inline const T &operator()(size_t i) const {
    return operator()(i);
  }
};

template <typename T>
class Tensor<T, 2> : public TensorBase<T,2> {
 public:
  explicit Tensor(size_t d0, size_t d1) : TensorBase<T,2>(d0,d1) {}

  inline T &operator()(size_t i, size_t j) {
    return this->get(i,j);
  }

  inline const T &operator()(size_t i, size_t j) const {
    return this->get(i,j);
  }
};

template <typename T>
class Tensor<T, 3> : public TensorBase<T, 3> {
 public:
  explicit Tensor(size_t d0, size_t d1, size_t d2) : TensorBase<T,3>(d0,d1,d2){}

  inline T &operator()(size_t i, size_t j, size_t k) {
    return this->get(i,j,k);
  }

  inline const T &operator()(size_t i, size_t j, size_t k) const {
    return this->get(i,j,k);
  }
};

template <typename T>
class Scalar : public Tensor<T, 0> {
 public:
  Scalar() {}
  Scalar(T val) : Tensor<T, 0>(val) {}

  inline void operator= (const T &val) { Tensor<T, 0>::operator=(val); }
};

template <typename T>
class Vector : public Tensor<T, 1> {
 public:
  explicit Vector(size_t d0) : Tensor<T, 1>(d0) {}
};

template <typename T>
class Matrix : public Tensor<T, 2> {
 public:
  explicit Matrix(size_t d0, size_t d1) : Tensor<T, 2>(d0, d1) {}
};


}

#endif
