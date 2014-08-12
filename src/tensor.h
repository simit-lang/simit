#ifndef SIMIT_TENSOR_H
#define SIMIT_TENSOR_H

namespace simit {
/** The types of supported tensor components. */
enum Type {INT, FLOAT, ELEMENT};

/** Helper to convert from C++ type to Simit Type. */
template <typename T>
  Type type_of() { return Type::ELEMENT; }


/// A tensor is a generalization of scalars, vectors and matrices. Tensors can
/// be fields of \ref Set instances and can be bound to \ref Program instances.
class Tensor {

};

}

#endif
