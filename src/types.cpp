#include "types.h"

#include <ostream>

#include "ir.h"
#include "macros.h"
#include "util.h"

namespace simit {
namespace ir {

// Default to double size
unsigned ScalarType::floatBytes = sizeof(double);

bool ScalarType::singleFloat() {
  iassert(floatBytes == sizeof(float) || floatBytes == sizeof(double))
      << "Invalid float size: " << floatBytes;
  return floatBytes == sizeof(float);
}

// struct TensorType
std::vector<IndexSet> TensorType::outerDimensions() const {
  unsigned maxNest = 0;
  for (auto &dim : dimensions) {
    if (dim.getIndexSets().size() > maxNest) {
      maxNest = dim.getIndexSets().size();
    }
  }

  std::vector<IndexSet> outerDimensions;
  for (auto &dim : dimensions) {
    if (dim.getIndexSets().size() == maxNest) {
      outerDimensions.push_back(dim.getIndexSets()[0]);
    }
  }

  return outerDimensions;
}


Type TensorType::blockType() const {
  // TODO (grab blocktype computation in ir.h/ir.cpp)
  if (dimensions.size() == 0) {
    return TensorType::make(componentType);
  }

  std::vector<IndexDomain> blockDimensions;

  size_t numNests = dimensions[0].getIndexSets().size();
  iassert(numNests > 0);

  Type blockType;
  if (numNests == 1) {
    blockType = TensorType::make(componentType);
  }
  else {
    unsigned maxNesting = 0;
    for (auto &dim : dimensions) {
      if (dim.getIndexSets().size() > maxNesting) {
        maxNesting = dim.getIndexSets().size();
      }
    }

    for (auto &dim : dimensions) {
      if (dim.getIndexSets().size() < maxNesting) {
        const std::vector<IndexSet> &nests = dim.getIndexSets();
        std::vector<IndexSet> blockNests(nests.begin(), nests.end());
        blockDimensions.push_back(IndexDomain(blockNests));
      }
      else {
        const std::vector<IndexSet> &nests = dim.getIndexSets();
        std::vector<IndexSet> blockNests(nests.begin()+1, nests.end());
        blockDimensions.push_back(IndexDomain(blockNests));
      }
    }
    blockType = TensorType::make(componentType, blockDimensions);
  }
  iassert(blockType.defined());

  return blockType;
}

size_t TensorType::size() const {
  size_t size = 1;
  for (auto &dimension : dimensions) {
    size *= dimension.getSize();
  }
  return size;
}

bool TensorType::isSparse() const {
  for (auto &indexDom : dimensions) {
    for (auto &indexSet : indexDom.getIndexSets()) {
      if (indexSet.getKind() != IndexSet::Range) {
        return true;
      }
    }
  }
  return false;
}


// struct SetType
Type SetType::make(Type elementType, const std::vector<Expr> &endpointSets) {
  iassert(elementType.isElement());
  SetType *type = new SetType;
  type->elementType = elementType;
  for (auto &eps : endpointSets) {
    type->endpointSets.push_back(new Expr(eps));
  }
  return type;
}

SetType::~SetType() {
  for (auto &eps : endpointSets) {
    delete eps;
  }
}


// Free operator functions
bool operator==(const Type& l, const Type& r) {
  if (l.kind() != r.kind()) {
    return false;
  }

  switch (l.kind()) {
    case Type::Tensor:
      return *l.toTensor() == *r.toTensor();
    case Type::Element:
      return *l.toElement() == *r.toElement();
    case Type::Set:
      return *l.toSet() == *r.toSet();
    case Type::Tuple:
      return *l.toTuple() == *r.toTuple();
  }
  unreachable;
  return false;
}

bool operator!=(const Type& l, const Type& r) {
  return !(l == r);
}

bool operator==(const ScalarType &l, const ScalarType &r) {
  return l.kind == r.kind;
}

bool operator==(const TensorType &l, const TensorType &r) {
  if (l.componentType != r.componentType) {
    return false;
  }
  if (l.order() != r.order()) {
    return false;
  }

  auto li = l.dimensions.begin();
  auto ri = r.dimensions.begin();
  for (; li != l.dimensions.end(); ++li, ++ri) {
    if (*li != *ri) {
      return false;
    }
  }

  return true;
}

bool operator==(const ElementType &l, const ElementType &r) {
  // Element type names are unique
  return (l.name == r.name);
}


bool operator==(const SetType &l, const SetType &r) {
  return l.elementType == r.elementType;
}

bool operator==(const TupleType &l, const TupleType &r) {
  return l.elementType == r.elementType && l.size == r.size;
}

bool operator!=(const ScalarType &l, const ScalarType &r) {
  return !(l == r);
}

bool operator!=(const TensorType &l, const TensorType &r) {
  return !(l == r);
}

bool operator!=(const ElementType &l, const ElementType &r) {
  return !(l == r);
}

bool operator!=(const SetType &l, const SetType &r) {
  return !(l == r);
}

bool operator!=(const TupleType &l, const TupleType &r) {
  return !(l == r);
}

std::ostream &operator<<(std::ostream &os, const Type &type) {
  switch (type.kind()) {
    case Type::Tensor:
      return os << *type.toTensor();
    case Type::Element:
      return os << *type.toElement();
    case Type::Set:
      return os << *type.toSet();
    case Type::Tuple:
      return os << *type.toTuple();
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const ScalarType &type) {
  switch (type.kind) {
    case ScalarType::Int:
      os << "int";
      break;
    case ScalarType::Float:
      os << "float";
      break;
    case ScalarType::Boolean:
      os << "boolean";
      break;
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const TensorType &type) {
  if (type.order() == 0) {
    os << type.componentType;
  }
  else {
    os << "tensor";
    os << "[" << util::join(type.dimensions, "][") << "]";
    os << "(" << type.componentType << ")";
  }
  return os;
}

std::ostream &operator<<(std::ostream &os, const ElementType &type) {
  os << type.name;
  return os;
}

std::ostream &operator<<(std::ostream &os, const SetType &type) {
  os << "set{" << type.elementType.toElement()->name << "}";

  if (type.endpointSets.size() > 0) {
    os << "(" << util::join(type.endpointSets) << ")";
  }

  return os;
}

std::ostream &operator<<(std::ostream &os, const TupleType &type) {
  return os << "(" << type.elementType.toElement()->name << "*" << type.size
            << ")";
}


}} // namespace simit::ir
