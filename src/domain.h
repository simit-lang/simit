#ifndef SIMIT_DOMAIN_H
#define SIMIT_DOMAIN_H

#include <cstddef>
#include <cassert>
#include <string>
#include <vector>
#include <ostream>
#include <memory>

namespace simit {
namespace ir {

class Expr;

/// An index set is a set of labels into a set.  There are three types of index
/// set distringuished by the type of set they index into: a range (Range), a 
/// set name (Set) or the set of all integers (Dynamic).
class IndexSet {
public:
  /// The types of index sets that are supported.
  enum Kind {Range, Set, Dynamic};

  /// Create an index set consisting of the items in the given range.
  IndexSet(signed rangeSize) : kind(Range), rangeSize(rangeSize), set(nullptr){}

  /// Create an index set over the given set.
  IndexSet(const Expr &set);

  /// Create a variable-size index set.
  IndexSet() : kind(Dynamic) {}

  /// Get the Kind of the index set (Range, Set or Dynamic)
  Kind getKind() const { return kind; }

  /// Returns the size of the index set if kind is Range, otherwise undefined
  int getSize() const {
    assert(kind == Range && "Only Range index sets have a static size");
    return rangeSize;
  }

  /// Returns the a set if kind is Set, otherwise undefined
  const Expr &getSet() const;

private:
  Kind kind;

  int rangeSize;
  std::shared_ptr<Expr> set;
};

bool operator==(const IndexSet &l, const IndexSet &r);
bool operator!=(const IndexSet &l, const IndexSet &r);
std::ostream &operator<<(std::ostream &os, const IndexSet &is);


/// An index domain is a set product of zero or more index sets.
/// \todo Domain one of: a full set, edge endpoints, element edges
class IndexDomain {
public:
  IndexDomain() {}
  explicit IndexDomain(IndexSet is) { indexSets.push_back(is); }
  explicit IndexDomain(std::vector<IndexSet> iss) : indexSets(iss) {};

  /// Get the index sets whose set product forms the domain.
  const std::vector<IndexSet> &getIndexSets() const {return indexSets; }

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
