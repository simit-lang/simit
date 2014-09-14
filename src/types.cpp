#include "types.h"

#include <assert.h>
#include <climits>
#include <iostream>

#include "util.h"
#include "macros.h"

#include "graph.h"

using namespace std;

namespace simit {
namespace internal {

// class Type
bool operator==(const Type& l, const Type& r) {
  if (l.getKind() != r.getKind()) {
    return false;
  }

  switch (l.getKind()) {
    case Type::Kind::Set:
      NOT_SUPPORTED_YET;
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
  switch (type) {
    case RANGE:
      size = rangeSize;
      break;
    case SET:
      NOT_SUPPORTED_YET;
      break;
    case VARIABLE:
      NOT_SUPPORTED_YET;
      break;
    default:
      assert(false);
  }
  return size;
}

bool operator==(const IndexSet &l, const IndexSet &r) {
   if (l.type != r.type) {
    return false;
  }
  switch (l.type) {
    case IndexSet::RANGE:
      if (l.rangeSize != r.rangeSize) {
        return false;
      }
      break;
    case IndexSet::SET:
      NOT_SUPPORTED_YET;
      break;
    case IndexSet::VARIABLE:
      NOT_SUPPORTED_YET;
      break;
    default:
      assert(false);
      break;
  }
  return true;
}

bool operator!=(const IndexSet &l, const IndexSet &r) {
  return !(l == r);
}

std::ostream &operator<<(std::ostream &os, const IndexSet &is) {
  switch (is.type) {
    case IndexSet::RANGE:
      os << to_string(is.rangeSize);
      break;
    case IndexSet::SET:
      NOT_SUPPORTED_YET;
      break;
    case IndexSet::VARIABLE:
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

bool operator==(const IndexSetProduct &l,
                                 const IndexSetProduct &r) {
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
  for (ElementField *field : elementType.getFields()) {
    os << *field << ";" << endl;
  }
  os << "end";
  return os;
}


// class ElementField
std::ostream &operator<<(std::ostream &os, const ElementField &field) {
  os << field.getName() << " : " << *field.getType();
  return os;
}


}} // namespace simit::internal
