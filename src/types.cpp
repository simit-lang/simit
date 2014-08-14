#include "types.h"

#include <assert.h>
#include <climits>
#include <iostream>

#include "util.h"
#include "macros.h"

using namespace std;

namespace simit {
namespace internal {

// class IndexSet
int IndexSet::getSize() const {
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

std::ostream &IndexSet::print(std::ostream &os) const {
  switch (type) {
    case RANGE:
      os << to_string(rangeSize);
      break;
    case SET:
      NOT_SUPPORTED_YET;
      break;
    case VARIABLE:
      os << "*";
      break;
    default:
      assert(false);
      break;
  }
  return os;
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

std::ostream &operator<<(std::ostream &os, const IndexSet &o) {
  return o.print(os);
}


// class IndexSetProduct
int IndexSetProduct::getSize() const {
  int size = 1;
  for (auto &indexSet : getIndexSets()) {
    size *= indexSet.getSize();
  }
  return size;
}

std::ostream &IndexSetProduct::print(std::ostream &os) const {
  return os << util::join(getIndexSets(), " x ");
}

bool operator==(const IndexSetProduct &l,
                                 const IndexSetProduct &r) {
  if (l.getIndexSets().size() != r.getIndexSets().size()) {
    return false;
  }
  auto li = l.getIndexSets().begin();
  auto ri = r.getIndexSets().begin();
  for (; li != l.getIndexSets().end(); ++li, ++ri) {
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
  std::vector<IndexSet> is = l.getIndexSets();
  is.insert(is.end(), r.getIndexSets().begin(), r.getIndexSets().end());
  return IndexSetProduct(is);
}

std::ostream &operator<<(std::ostream &os, const IndexSetProduct &o) {
  return o.print(os);
}


// class TensorType
int TensorType::getSize() const {
  int size = 1;
  for (auto &dimension : getDimensions()) {
    size *= dimension.getSize();
  }
  return size;
}

std::ostream &TensorType::print(std::ostream &os) const {
  if (getOrder() == 0) {
    os << componentTypeString(getComponentType());
  }
  else {
    os << "Tensor";
    os << "[" << util::join(getDimensions(), "][") << "]";
    os << "(" << componentTypeString(getComponentType()) << ")";
  }
  return os;
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
};

bool operator!=(const TensorType& l, const TensorType& r) {
  return !(l == r);
}

std::ostream &operator<<(std::ostream &os, const TensorType &o) {
  return o.print(os);
}

std::size_t TensorType::componentSize(Type ct) {
  switch (ct) {
    case Type::INT:
      return sizeof(int);
    case Type::FLOAT:
      return sizeof(double);
    case Type::ELEMENT:
      assert(false && "currently unsupported");  // TODO
      return INT_MAX;
  }
  assert(false);
  return 0;
}

std::string TensorType::componentTypeString(Type ct) {
  switch (ct) {
    case Type::INT:
      return "int";
    case Type::FLOAT:
      return "float";
    case Type::ELEMENT:
      return "element";
  }
  assert(false);
  return "";
}

}} // namespace simit::internal
