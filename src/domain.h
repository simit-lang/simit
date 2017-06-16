#ifndef SIMIT_DOMAIN_H
#define SIMIT_DOMAIN_H

#include <string>
#include <vector>
#include <ostream>
#include <memory>

#include "error.h"

namespace simit {
namespace ir {

class Expr;

/// An index set is a set of labels into a set.  There are three types of index
/// set distringuished by the type of set they index into: a range (Range), a 
/// set name (Set) or the set of all integers (Dynamic).
class IndexSet {
public:
  /// The types of index sets that are supported.
  enum Kind {Range, Set, Dynamic, Single};

  /// Create an index set consisting of the items in the given range.
  IndexSet(unsigned rangeSize) : kind(Range),
      rangeSize(rangeSize), set(nullptr){}

  /// Create an index set over the given set.
  IndexSet(const Expr &set);

  /// Create a variable-size index set.
  IndexSet() : kind(Dynamic), rangeSize(0) {}
  
  /// Create an index set of a single expression
  IndexSet(const Expr &set, Kind kind);

  /// Get the Kind of the index set (Range, Set or Dynamic)
  Kind getKind() const { return kind; }

  /// Returns the size of the index set if kind is Range, otherwise undefined
  unsigned getSize() const {
    simit_iassert(kind == Range) << "Only Range index sets have a static size";
    return rangeSize;
  }

  /// Returns the a set if kind is Set, otherwise undefined
  const Expr &getSet() const;

private:
  Kind kind;

  unsigned rangeSize;
  std::shared_ptr<Expr> set;
};

bool operator==(const IndexSet &l, const IndexSet &r);
bool operator!=(const IndexSet &l, const IndexSet &r);
std::ostream &operator<<(std::ostream &os, const IndexSet &is);


/// An index domain is the set product of zero or more index sets.
class IndexDomain {
public:
  IndexDomain() {}
  IndexDomain(const IndexSet &is) { indexSets.push_back(is); }
  IndexDomain(const std::vector<IndexSet> &iss) : indexSets(iss) {};

  /// Get the number of index sets in the index domain.
  size_t getNumIndexSets() const {return indexSets.size();}

  /// Get the ith index set of the index domain.
  const std::vector<IndexSet> &getIndexSets() const {return indexSets; }

  /// Get the number of elements in the product of the index sets if all the
  /// index sets are Range sets, otherwise undefined.
  int getSize() const;

private:
  std::vector<IndexSet> indexSets;
};

bool operator==(const IndexDomain &l, const IndexDomain &r);
bool operator!=(const IndexDomain &l, const IndexDomain &r);
IndexDomain operator*(const IndexDomain &l, const IndexDomain &r);
std::ostream &operator<<(std::ostream &os, const IndexDomain &isp);

}} // namespace simit::ir

#endif
