#include "types.h"

#include <assert.h>
#include <climits>
#include <iostream>

#include "util.h"
#include "macros.h"

#include "graph.h"

using namespace std;

namespace simit {
namespace ir {

// class Type
bool operator==(const Type& l, const Type& r) {
  if (l.getKind() != r.getKind()) {
    return false;
  }

  switch (l.getKind()) {
    case Type::Kind::Set:
      return static_cast<const SetType&>(l) == static_cast<const SetType&>(r);
    case Type::Kind::Tensor:
      return static_cast<const TensorType&>(l) ==
             static_cast<const TensorType&>(r);
  }
}

bool operator!=(const Type& l, const Type& r) {
  return !(l == r);
}


// class IndexSet
size_t IndexSet::getSize() const {
  int size = 0;
  switch (kind) {
    case Range:
      size = rangeSize;
      break;
    case Set:
      NOT_SUPPORTED_YET;
      break;
    case Dynamic:
      NOT_SUPPORTED_YET;
      break;
    default:
      assert(false);
  }
  return size;
}

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


// class IndexSetProduct
size_t IndexSetProduct::getSize() const {
  int size = 1;
  for (auto &indexSet : getFactors()) {
    size *= indexSet.getSize();
  }
  return size;
}

bool operator==(const IndexSetProduct &l, const IndexSetProduct &r) {
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

bool operator!=(const IndexSetProduct &l, const IndexSetProduct &r) {
  return !(l == r);
}

IndexSetProduct operator*(const IndexSetProduct &l, const IndexSetProduct &r) {
  std::vector<IndexSet> is = l.getFactors();
  is.insert(is.end(), r.getFactors().begin(), r.getFactors().end());
  return IndexSetProduct(is);
}

std::ostream &operator<<(std::ostream &os, const IndexSetProduct &isp) {
  return os << util::join(isp.getFactors(), " x ");
}


// class TensorType
size_t TensorType::getSize() const {
  int size = 1;
  for (auto &dimension : getDimensions()) {
    size *= dimension.getSize();
  }
  return size;
}

size_t TensorType::getByteSize() const {
  return getSize() * simit::componentSize(getComponentType());
}

void TensorType::print(std::ostream &os) const {
  if (getOrder() == 0) {
    os << componentTypeString(getComponentType());
  }
  else {
    os << "Tensor";
    os << "[" << util::join(getDimensions(), "][") << "]";
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
std::ostream &operator<<(std::ostream &os, const ElementType &elementType) {
  os << "struct " << elementType.getName();
  if (elementType.getFields().size() > 0) {
    os << endl << "  ";
  }
  for (auto &field : elementType.getFields()) {
    os << field.first << " : " << *field.second << ";" << endl;
  }
  os << "end";
  return os;
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


// class SetType
size_t SetType::getByteSize() const {
  return 0;
}

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
