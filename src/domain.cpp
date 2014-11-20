#include "domain.h"

#include "macros.h"
#include "util.h"
#include "ir.h"
#include "ir_printer.h"

namespace simit {
namespace ir {

// class IndexSet
IndexSet::IndexSet(const Expr &set) : kind(Set), rangeSize(-1),
                                      set(new Expr(set)) {}

const Expr &IndexSet::getSet() const {
  iassert(kind==Set);
  return *set;
}

bool operator==(const IndexSet &l, const IndexSet &r) {
   if (l.getKind() != r.getKind()) {
    return false;
  }
  switch (l.getKind()) {
    case IndexSet::Range:
      return l.getSize() == r.getSize();
    case IndexSet::Set:
      return l.getSet() == r.getSet();
    case IndexSet::Dynamic:
      return true;
      break;
  }
}

bool operator!=(const IndexSet &l, const IndexSet &r) {
  return !(l == r);
}

std::ostream &operator<<(std::ostream &os, const IndexSet &is) {
  switch (is.getKind()) {
    case IndexSet::Range:
      os << "0:" << std::to_string(is.getSize());
      break;
    case IndexSet::Set:
      os << is.getSet();
      break;
    case IndexSet::Dynamic:
      os << "*";
      break;
    default:
      ierror;
      break;
  }
  return os;
}


// class IndexDomain
size_t IndexDomain::getSize() const {
  int size = 1;
  for (auto &indexSet : getIndexSets()) {
    size *= indexSet.getSize();
  }
  return size;
}

bool operator==(const IndexDomain &l, const IndexDomain &r) {
  if (l.getIndexSets().size() != r.getIndexSets().size()) {
    return false;
  }
  for (size_t i=0; i < l.getIndexSets().size(); ++i) {
    if (l.getIndexSets()[i] != r.getIndexSets()[i]) {
      return false;
    }
  }
  return true;
}

bool operator!=(const IndexDomain &l, const IndexDomain &r) {
  return !(l == r);
}

IndexDomain operator*(const IndexDomain &l, const IndexDomain &r) {
  std::vector<IndexSet> is = l.getIndexSets();
  is.insert(is.end(), r.getIndexSets().begin(), r.getIndexSets().end());
  return IndexDomain(is);
}

std::ostream &operator<<(std::ostream &os, const IndexDomain &isp) {
  return os << util::join(isp.getIndexSets(), "*");
}

}} //namespace simit::ir
