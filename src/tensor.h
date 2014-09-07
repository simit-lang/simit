#ifndef SIMIT_TENSOR_H
#define SIMIT_TENSOR_H

#include <cassert>
#include <initializer_list>

#include "tensor_components.h"

// TODO: Remove
#include <iostream>
using namespace std;

const int Dynamic = -1;

namespace simit {

// Tensors

/// A tensor is a generalization of scalars, vectors and matrices. Tensors can
/// be fields of \ref Set instances and can be bound to \ref Program instances.
template <typename T, size_t order>
class Tensor {
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
class Tensor<T,0> {
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
/// Curiously Recurring Template Pattern to reuse code.
template <typename T,class Vector>
class VectorBase {
 public:
  Type getType() const { return typeOf<T>(); }

  inline size_t getOrder() const { return 1; }
  inline size_t getDimension(size_t i) const {
    assert(i == 0);
    return getSize();
  }

  inline size_t getSize() const {
    return static_cast<const Vector*>(this)->getSize();
  }

  inline T &operator()(size_t i) {
    assert(i < getSize());
    return static_cast<Vector*>(this)->data[i];
  }

  inline const T &operator()(size_t i) const {
    return static_cast<const Vector*>(this)->data[i];
  }

 protected:
  inline void init(std::initializer_list<T> vals) {
    assert(vals.size() == getSize());
    size_t i = 0;
    for (T val : vals) {
      static_cast<Vector*>(this)->data[i++] = val;
    }
  }
};

template <typename T, int size>
class Vector : public VectorBase<T,Vector<T,size>> {
  friend class VectorBase<T,Vector<T,size>>;
 public:
  inline explicit Vector() : VectorBase<T,Vector<T,size>>() {
    assert(size > 0);
  }

  inline explicit Vector(std::initializer_list<T> vals) {
    assert(size > 0);
    this->init(vals);
  }

  inline size_t getSize() const { return size; };

 private:
  T data[size];
};

template <typename T>
class Vector<T,Dynamic> : public VectorBase<T,Vector<T,Dynamic>> {
  friend class VectorBase<T,Vector<T,Dynamic>>;
 public:
  inline explicit Vector(size_t size)
      : size{size}, data{(T*)malloc(size * sizeof(T))} {
    assert(size > 0);
  }

  inline explicit Vector(size_t size, std::initializer_list<T> vals)
      : Vector(size) {
    assert(size > 0);
    this->init(vals);
  }

  inline ~Vector() { free(data);}

  inline size_t getSize() const { return size; };

 private:
  size_t size;
  T *data;
};

template <typename T>
class Tensor<T,1> : public Vector<T,Dynamic> {
 public:
  inline explicit Tensor(size_t size) : Vector<T,Dynamic>(size) {}
  inline explicit Tensor(size_t size, std::initializer_list<T> vals)
      : Vector<T,Dynamic>(size, vals) {}
};

template <class T1, int size1, class T2, int size2>
bool operator==(const Vector<T1,size1> &t1, const Vector<T2,size2> &t2) {
  if (t1.getSize() != t2.getSize()) {
    return false;
  }
  for (size_t i=0; i<t1.getSize(); ++i) {
    if (t1(i) != t2(i)) {
      return false;
    }
  }
  return true;
}

template <class T1, int size1, class T2, int size2>
bool operator!=(const Vector<T1,size1> &t1, const Vector<T2,size2> &t2) {
  return !(t1 == t2);
}

template <class T, int size>
std::ostream &operator<<(std::ostream &os, const Vector<T,size> &t) {
  os << "[";
  assert(t.getSize() > 0);
  os << t(0);
  for (size_t i=1; i<t.getSize(); i++) {
    os << ", " << t(i);
  }
  os << "]";
  return os;
}


// Matrices
template <typename T>
class Tensor<T, 2> {
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
class Tensor<T, 3> {
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
