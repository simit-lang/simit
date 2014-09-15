#ifndef SIMIT_TYPES_H
#define SIMIT_TYPES_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <memory>

#include "interfaces.h"
#include "tensor_components.h"

namespace simit {
class SetBase;

namespace ir {

/// A Simit type, which is either a Set or a Tensor.
class Type : public simit::interfaces::Printable, simit::interfaces::Uncopyable{
public:
  enum Kind { Set, Tensor };
  Type(Kind kind) : kind(kind) {}

  virtual ~Type() {}

  Kind getKind() const { return kind; }
  bool isTensor() const { return kind == Type::Tensor; }
  bool isSet() const { return kind == Type::Set; }

  virtual size_t getByteSize() const = 0;

private:
  Kind kind;
};

bool operator==(const Type& l, const Type& r);
bool operator!=(const Type& l, const Type& r);


// Tensor types

/// An index set is a set of labels into a set.  There are three types of index
/// set distringuished by the type of set they index into: a range (RANGE), a 
/// simit::Set (SET) or the set of all integers (VARIABLE).
class IndexSet {
public:
  /// The types of index sets that are supported.
  enum Type {RANGE, SET, VARIABLE};

  /// Create an index set consisting of the items in the given range.
  IndexSet(int rangeSize) : type(RANGE), rangeSize(rangeSize) {}

  /// Create an index set over the given set.
  IndexSet(const simit::SetBase *set) : type(SET), set(set) {}

  /// Create a variable-size index set.
  IndexSet() : type(VARIABLE) {}

  /// Get the number of elements in the index set.
  size_t getSize() const;

  friend bool operator==(const IndexSet &l, const IndexSet &r);
  friend std::ostream &operator<<(std::ostream &os, const IndexSet &is);

private:
  Type type;
  union {
    int rangeSize;
    const simit::SetBase *set;
  };
};

bool operator!=(const IndexSet &l, const IndexSet &r);


/// The set product of zero or more sets.
class IndexSetProduct {
public:
  IndexSetProduct() {}
  IndexSetProduct(const IndexSet &is) { indexSets.push_back(is); }
  IndexSetProduct(const std::vector<IndexSet> &iss) : indexSets(iss) {};

  /// Get the index sets that are multiplied to get the index set product.
  const std::vector<IndexSet> &getFactors() const {return indexSets; }

  /// Get the number of elements in the product of the index sets.
  size_t getSize() const;

private:
  std::vector<IndexSet> indexSets;
};

bool operator==(const IndexSetProduct &l, const IndexSetProduct &r);
bool operator!=(const IndexSetProduct &l, const IndexSetProduct &r);
IndexSetProduct operator*(const IndexSetProduct &l, const IndexSetProduct &r);
std::ostream &operator<<(std::ostream &os, const IndexSetProduct &isp);


/// The type of a tensor (the type of its components and its shape). Note that
/// a scalar in Simit is a o-order tensor.
class TensorType : public Type {
public:
  TensorType(ComponentType componentType)
      : Type(Type::Tensor), componentType(componentType) {
    assert(getKind() == 1);
  }

  TensorType(ComponentType componentType,
             const std::vector<IndexSetProduct> &dimensions)
      : Type(Type::Tensor), componentType(componentType),
        dimensions(dimensions) {}

  /// Get the order of the tensor (the number of dimensions).
  size_t getOrder() const { return dimensions.size(); }

  /// Get the type of the components in the vector.
  ComponentType getComponentType() const { return componentType; }

  /// Get the index sets that form the dimensions of the tensor.
  const std::vector<IndexSetProduct> &getDimensions() const {return dimensions;}

  /// Get the number of components in the tensor.
  size_t getSize() const;

  size_t getByteSize() const;

private:
  ComponentType componentType;
  std::vector<IndexSetProduct> dimensions;

  void print(std::ostream &os) const;
};

bool operator==(const TensorType& l, const TensorType& r);
bool operator!=(const TensorType& l, const TensorType& r);


// Set types
class ElementType {
public:
  typedef std::map<std::string,std::shared_ptr<TensorType>> FieldsMapType;

  ElementType(const std::string &name, const FieldsMapType &fields)
      : name(name), fields(fields) {}

  const std::string &getName() const { return name; }
  const FieldsMapType &getFields() const { return fields; }

private:
  std::string name;
  FieldsMapType fields;
};

std::ostream &operator<<(std::ostream &os, const ElementType &elementType);
bool operator==(const ElementType &l, const ElementType &r);
bool operator!=(const ElementType &l, const ElementType &r);

/// The type of a Simit set (defined by the type of its elements and it's
/// connectivity information).  Note that a single element in Simit is a set
/// of size 1 without any connectivity.
class SetType : public Type {
public:
  SetType(std::shared_ptr<ElementType> elementType)
      : Type(Type::Set), elementType(elementType) {}

  virtual ~SetType() {}

  const std::shared_ptr<ElementType> &getElementType() const {
    return elementType;
  }

  // TODO: Remove the getByteSize methods from the Type hierarchy since the
  //       types of sets and variable-sized tensors don't have well-defined
  //       sizes.
  size_t getByteSize() const;

private:
  std::shared_ptr<ElementType> elementType;

  void print(std::ostream &os) const;
};

bool operator==(const SetType& l, const SetType& r);
bool operator!=(const SetType& l, const SetType& r);


// Conversion functions
inline TensorType *tensorTypePtr(Type *type) {
  assert(type->isTensor());
  return static_cast<TensorType*>(type);
}

inline TensorType *tensorTypePtr(const std::shared_ptr<Type> &type) {
  return tensorTypePtr(type.get());
}

inline SetType *setTypePtr(Type *type) {
  assert(type->isSet());
  return static_cast<SetType*>(type);
}

inline SetType *setTypePtr(const std::shared_ptr<Type> &type) {
  return setTypePtr(type.get());
}

}} // namespace simit::internal

#endif
