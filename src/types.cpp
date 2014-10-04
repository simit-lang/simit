#include "types.h"

#include <assert.h>
#include <iostream>

#include "util.h"
#include "macros.h"

using namespace std;

namespace simit {
namespace ir {

// class IndexSet
bool operator==(const IndexSet &l, const IndexSet &r) {
   if (l.kind != r.kind) {
    return false;
  }
  switch (l.kind) {
    case IndexSet::Range:
      return l.rangeSize == r.rangeSize;
    case IndexSet::Set:
      return l.setName == r.setName;
    case IndexSet::Dynamic:
      NOT_SUPPORTED_YET;
      break;
  }
  return true;
}

bool operator!=(const IndexSet &l, const IndexSet &r) {
  return !(l == r);
}

std::ostream &operator<<(std::ostream &os, const IndexSet &is) {
  switch (is.kind) {
    case IndexSet::Range:
      os << to_string(is.rangeSize);
      break;
    case IndexSet::Set:
      os << is.setName;
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


// class Type
bool operator==(const Type& l, const Type& r) {
  if (l.getKind() != r.getKind()) {
    return false;
  }

  switch (l.getKind()) {
    case Type::Tensor:
      return static_cast<const TensorType&>(l) ==
      static_cast<const TensorType&>(r);
    case Type::Element:
      NOT_SUPPORTED_YET;
      break;
    case Type::Set:
      return static_cast<const SetType&>(l) == static_cast<const SetType&>(r);
    case ir::Type::Tuple:
      NOT_SUPPORTED_YET;
      break;
  }
}

bool operator!=(const Type& l, const Type& r) {
  return !(l == r);
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


// class TensorType
size_t TensorType::getSize() const {
  int size = 1;
  for (auto &dimension : getDimensions()) {
    size *= dimension.getSize();
  }
  return size;
}

void TensorType::print(std::ostream &os) const {
  if (getOrder() == 0) {
    os << componentTypeString(getComponentType());
  }
  else {
    os << "tensor";
    os << "[" << util::join(getDimensions(), ",") << "]";
    os << "(" << componentTypeString(getComponentType()) << ")";
  }
}

bool operator==(const TensorType& l, const TensorType& r) {
  if (l.getComponentType() != r.getComponentType() ) {
    return false;
  }
  if (l.getOrder() != r.getOrder()) {
    return false;
  }

  auto li = l.getDimensions().begin();
  auto ri = r.getDimensions().begin();
  for (; li != l.getDimensions().end(); ++li, ++ri) {
    if (*li != *ri) {
      return false;
    }
  }

  return true;
}

bool operator!=(const TensorType& l, const TensorType& r) {
  return !(l == r);
}


// class ElementType
void ElementType::print(std::ostream &os) const {
  os << getName();
}

bool operator==(const ElementType &l, const ElementType &r) {
  // Element type names are unique
  if (l.getName() != r.getName()) {
    return false;
  }

  return true;
}

bool operator!=(const ElementType &l, const ElementType &r) {
  return !(l == r);
}


// class TupleType
void TupleType::print(std::ostream &os) const {
  os << "(" << util::join(elementTypes, ",") << ")";
}


// class SetType
void SetType::print(std::ostream &os) const {
  os << elementType->getName() << "{}";
}

bool operator==(const SetType& l, const SetType& r) {
  if (*l.getElementType() != *r.getElementType()) {
    return false;
  }
  return true;
}

bool operator!=(const SetType& l, const SetType& r) {
  return !(l == r);
}

}} // namespace simit::internal
