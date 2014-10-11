#ifndef SIMIT_DOMAIN_H
#define SIMIT_DOMAIN_H

#include <cstddef>
#include <cassert>
#include <string>
#include <vector>
#include <ostream>

namespace simit {
namespace ir {

/// An index set is a set of labels into a set.  There are three types of index
/// set distringuished by the type of set they index into: a range (Range), a 
/// set name (Set) or the set of all integers (Dynamic).
class IndexSet {
public:
  /// The types of index sets that are supported.
  enum Kind {Range, Set, Dynamic};

  /// Create an index set consisting of the items in the given range.
  IndexSet(signed rangeSize) : kind(Range), rangeSize(rangeSize), setName("") {}

  /// Create an index set over the given set.
  IndexSet(std::string setName) : kind(Set), rangeSize(-1), setName(setName) {}

  /// Create a variable-size index set.
  IndexSet() : kind(Dynamic) {}

  /// Get the Kind of the index set (Range, Set or Dynamic)
  Kind getKind() const { return kind; }

  /// Returns the size of the index set if kind is Range, otherwise undefined
  int getSize() const {
    assert(kind == Range && "Only Range index sets have a statically known size");
    return rangeSize;
  }

  /// Returns the name of the indexset set if kind is Set, otherwise undefined
  std::string getSetName() const {
    assert(kind==Set);
    return setName;
  }

private:
  Kind kind;

  int rangeSize;
  std::string setName;
};

bool operator==(const IndexSet &l, const IndexSet &r);
bool operator!=(const IndexSet &l, const IndexSet &r);
std::ostream &operator<<(std::ostream &os, const IndexSet &is);


/// An index domain is a set product of zero or more index sets.
class IndexDomain {
public:
  explicit IndexDomain() {}
  explicit IndexDomain(IndexSet is) { indexSets.push_back(is); }
  explicit IndexDomain(std::vector<IndexSet> iss) : indexSets(iss) {};

  /// Get the index sets that are multiplied to get the index set product.
  const std::vector<IndexSet> getFactors() const {return indexSets; }

  /// Get the number of elements in the product of the index sets if all the
  /// index sets are Range sets, otherwise undefined.
  size_t getSize() const;

private:
  std::vector<IndexSet> indexSets;
};

bool operator==(const IndexDomain &l, const IndexDomain &r);
bool operator!=(const IndexDomain &l, const IndexDomain &r);
IndexDomain operator*(const IndexDomain &l, const IndexDomain &r);
std::ostream &operator<<(std::ostream &os, const IndexDomain &isp);

}} // namespace simit::ir

#endif
