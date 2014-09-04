#ifndef SIMIT_TYPES_H
#define SIMIT_TYPES_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <memory>

#include "tensor_components.h"

namespace simit {
class Set;

namespace internal {

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
  IndexSet(const simit::Set *set) : type(SET), set(set) {}

  /// Create a variable-size index set.
  IndexSet() : type(VARIABLE) {}

  /// Get the number of elements in the index set.
  int getSize() const;

  std::ostream &print(std::ostream &os) const;
  friend bool operator==(const IndexSet &l, const IndexSet &r);

 private:
  Type type;
  union {
    int rangeSize;
    const simit::Set *set;
  };
};

bool operator==(const IndexSet &l, const IndexSet &r);
bool operator!=(const IndexSet &l, const IndexSet &r);
std::ostream &operator<<(std::ostream &os, const IndexSet &o);


/// The set product of zero or more sets.
class IndexSetProduct {
 public:
  IndexSetProduct() {}
  IndexSetProduct(const IndexSet &is) { indexSets.push_back(is); }
  IndexSetProduct(const std::vector<IndexSet> &iss) : indexSets(iss) {};

  /// Get the index sets that are multiplied to get the index set product.
  const std::vector<IndexSet> &getFactors() const {return indexSets; }

  /// Get the number of elements in the product of the index sets.
  int getSize() const;

  std::ostream &print(std::ostream &os) const;
  
 private:
  std::vector<IndexSet> indexSets;
};

bool operator==(const IndexSetProduct &l, const IndexSetProduct &r);
bool operator!=(const IndexSetProduct &l, const IndexSetProduct &r);
IndexSetProduct operator*(const IndexSetProduct &l, const IndexSetProduct &r);
std::ostream &operator<<(std::ostream &os, const IndexSetProduct &o);


/// The type of a tensor (the type of its components and its shape).
class TensorType {
 public:
  TensorType(Type componentType) : componentType(componentType) {}
  TensorType(Type componentType,
       const std::vector<IndexSetProduct> &dimensions)
      : componentType(componentType), dimensions(dimensions) {}

  /// Get the order of the tensor (the number of dimensions).
  int getOrder() const { return dimensions.size(); }

  /// Get the number of components in the tensor.
  int getSize() const;

  /// Get the type of the components in the vector.
  Type getComponentType() const { return componentType; }

  /// Get the index sets that form the dimensions of the tensor.
  const std::vector<IndexSetProduct> &getDimensions() const {return dimensions;}

  std::ostream &print(std::ostream &os) const;

  static std::size_t componentSize(Type ct);
  static std::string componentTypeString(Type ct);

 private:
  Type componentType;
  std::vector<IndexSetProduct> dimensions;
};

bool operator==(const TensorType& l, const TensorType& r);
bool operator!=(const TensorType& l, const TensorType& r);
std::ostream &operator<<(std::ostream &os, const TensorType &o);

}} // namespace simit::internal

#endif
