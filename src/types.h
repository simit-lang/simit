#ifndef SIMIT_TYPES_H
#define SIMIT_TYPES_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <memory>

#include "graph.h"

namespace simit {
namespace internal {


/** An index set is a set of labels into a set.  There are three types of index
  * set distringuished by the type of set they index into: a range (RANGE), a 
  * simit::Set (SET) or the set of all integers (VARIABLE). */
class IndexSet {
 public:
  enum Type {RANGE, SET, VARIABLE};

  IndexSet(int rangeSize) : type(RANGE), rangeSize(rangeSize) {}
  IndexSet(const simit::Set *set) : type(SET), set(set) {}
  IndexSet() : type(VARIABLE) {}

  int getSize() const;
  std::ostream &print(std::ostream &os) const;

  friend bool operator==(const IndexSet &left, const IndexSet &right);

 private:
  Type type;
  union {
    int rangeSize;
    const simit::Set *set;
  };
};

bool operator==(const IndexSet &left, const IndexSet &right);

static inline
bool operator!=(const IndexSet &left, const IndexSet &right) {
  return !(left == right);
}

static inline
std::ostream &operator<<(std::ostream &os, const IndexSet &obj) {
  return obj.print(os);
}


/** The set product of zero or more sets. */
class IndexSetProduct {
 public:
  enum Type {RANGE, SET, VARIABLE};

  IndexSetProduct() {}
  IndexSetProduct(const IndexSet &is) { indexSets.push_back(is); }
  IndexSetProduct(const std::vector<IndexSet> &indexSets)
      : indexSets(indexSets) {};

  int getSize() const;
  const std::vector<IndexSet> &getIndexSets() const {return indexSets; }
  std::ostream &print(std::ostream &os) const;
  
 private:
  std::vector<IndexSet> indexSets;
};

bool operator==(const IndexSetProduct &left, const IndexSetProduct &right);

static inline
bool operator!=(const IndexSetProduct &left, const IndexSetProduct &right) {
  return !(left == right);
}

static inline
IndexSetProduct operator*(const IndexSetProduct &left,
                          const IndexSetProduct &right) {
  std::vector<IndexSet> is = left.getIndexSets();
  is.insert(is.end(), right.getIndexSets().begin(), right.getIndexSets().end());
  return IndexSetProduct(is);
}

static inline
std::ostream &operator<<(std::ostream &os, const IndexSetProduct &obj) {
  return obj.print(os);
}


/** The type of a tensor (the type of its components and its shape). */
class Type {
 public:
  enum ComponentType {INT, FLOAT, ELEMENT};
  static std::size_t componentSize(ComponentType ct);
  static std::string componentTypeString(ComponentType ct);

  Type(ComponentType componentType) : componentType(componentType) {}
  Type(ComponentType componentType,
       const std::vector<IndexSetProduct> &dimensions)
      : componentType(componentType), dimensions(dimensions) {}

  /** Get the order of the tensor (the number of dimensions). */
  int getOrder() const {
    return dimensions.size();
  }

  /** Get the number of components in the tensor. */
  int getSize() const;

  ComponentType getComponentType() const {
    return componentType;
  }

  const std::vector<IndexSetProduct> &getDimensions() const {
    return dimensions;
  }

  std::ostream &print(std::ostream &os) const;

 private:
  ComponentType componentType;
  std::vector<IndexSetProduct> dimensions;
};

bool operator==(const Type& left, const Type& right);

static inline
bool operator!=(const Type& left, const Type& right) {
  return !(left == right);
}

static inline
std::ostream &operator<<(std::ostream &os, const Type &obj) {
  return obj.print(os);
}

}} // namespace simit::internal

#endif
