#include "types.h"

#include <assert.h>
#include <climits>
#include <iostream>

#include "util.h"
#include "macros.h"

using namespace simit::internal;
using namespace std;

/* class IndexSet */
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

bool simit::internal::operator==(const IndexSet &left, const IndexSet &right) {
   if (left.type != right.type) {
    return false;
  }

  switch (left.type) {
    case IndexSet::RANGE:
      if (left.rangeSize != right.rangeSize) {
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


/* class IndexSetProduct */
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

bool simit::internal::operator==(const IndexSetProduct &left,
                                 const IndexSetProduct &right) {
  if (left.getIndexSets().size() != right.getIndexSets().size()) {
    return false;
  }
  auto leftIter = left.getIndexSets().begin();
  auto rightIter = right.getIndexSets().begin();
  auto leftEnd = left.getIndexSets().end();
  for (; leftIter != leftEnd; ++leftIter, ++rightIter) {
    if (*leftIter != *rightIter) {
      return false;
    }
  }
  return true;
}


/* class Type */
std::size_t Type::componentSize(ComponentType ct) {
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

std::string Type::componentTypeString(ComponentType ct) {
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

int Type::getSize() const {
  int size = 1;
  for (auto &dimension : getDimensions()) {
    size *= dimension.getSize();
  }
  return size;
}

std::ostream &Type::print(std::ostream &os) const {
  os << "Tensor";
  os << "[" << util::join(getDimensions(), "][") << "]";
  os << "(" << componentTypeString(getComponentType()) << ")";
  return os;
}

bool simit::internal::operator==(const Type& left, const Type& right) {
  if (left.getComponentType() != right.getComponentType() ) {
    return false;
  }
  if (left.getOrder() != right.getOrder()) {
    return false;
  }

  auto leftIter = left.getDimensions().begin();
  auto rightIter = right.getDimensions().begin();
  auto leftEnd = left.getDimensions().end();
  for (; leftIter != leftEnd; ++leftIter, ++rightIter) {
    if (*leftIter != *rightIter) {
      return false;
    }
  }

  return true;
};
