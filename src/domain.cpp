#include "domain.h"

#include "macros.h"
#include "util.h"

namespace simit {
namespace ir {

// class IndexSet
bool operator==(const IndexSet &l, const IndexSet &r) {
   if (l.getKind() != r.getKind()) {
    return false;
  }
  switch (l.getKind()) {
    case IndexSet::Range:
      return l.getSize() == r.getSize();
    case IndexSet::Set:
      return l.getSetName() == r.getSetName();
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
      os << std::to_string(is.getSize());
      break;
    case IndexSet::Set:
      os << is.getSetName();
      break;
    case IndexSet::Dynamic:
      os << "*";
      break;
    default:
      assert(false);
      break;
  }
  return os;
}


// class IndexDomain
size_t IndexDomain::getSize() const {
  int size = 1;
  for (auto &indexSet : getFactors()) {
    size *= indexSet.getSize();
  }
  return size;
}

bool operator==(const IndexDomain &l, const IndexDomain &r) {
  if (l.getFactors().size() != r.getFactors().size()) {
    return false;
  }
  auto li = l.getFactors().begin();
  auto ri = r.getFactors().begin();
  for (; li != l.getFactors().end(); ++li, ++ri) {
    if (*li != *ri) {
      return false;
    }
  }
  return true;
}

bool operator!=(const IndexDomain &l, const IndexDomain &r) {
  return !(l == r);
}

IndexDomain operator*(const IndexDomain &l, const IndexDomain &r) {
  std::vector<IndexSet> is = l.getFactors();
  is.insert(is.end(), r.getFactors().begin(), r.getFactors().end());
  return IndexDomain(is);
}

std::ostream &operator<<(std::ostream &os, const IndexDomain &isp) {
  return os << util::join(isp.getFactors(), "*");
}

}} //namespace simit::ir
