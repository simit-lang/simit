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
  for (auto &indexSet : getFactors()) {
    size *= indexSet.getSize();
  }
  return size;
}

std::ostream &IndexSetProduct::print(std::ostream &os) const {
  return os << util::join(getFactors(), " x ");
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
void ElementType::print(std::ostream &os) const {
  os << "struct " << getName();
  if (getFields().size() > 0) {
    os << endl << "  ";
  }
  for (ElementField *field : getFields()) {
    os << *field << ";" << endl;
  }
  os << "end";
}


// class ElementField
std::ostream &operator<<(std::ostream &os, const ElementField &field) {
  os << field.getName() << " : " << *field.getType();
  return os;
}


}} // namespace simit::internal
