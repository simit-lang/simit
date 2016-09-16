#include "types.h"

#include <ostream>

#include "ir.h"
#include "macros.h"
#include "util/util.h"
#include "util/collections.h"

using namespace std;

namespace simit {
namespace ir {

// class Type
bool Type::isUnstructuredSet() const {
  return _kind == Set &&
      dynamic_cast<UnstructuredSetType*>(set) != nullptr;
}

bool Type::isGridSet() const {
  return _kind == Set &&
      dynamic_cast<GridSetType*>(set) != nullptr;
}

const UnstructuredSetType* Type::toUnstructuredSet() const {
  iassert(isUnstructuredSet());
  return dynamic_cast<UnstructuredSetType*>(set);
}

const GridSetType* Type::toGridSet() const {
  iassert(isGridSet());
  return dynamic_cast<GridSetType*>(set);
}

// Default to double size
unsigned ScalarType::floatBytes = sizeof(double);

bool ScalarType::singleFloat() {
  iassert(floatBytes == sizeof(float) || floatBytes == sizeof(double))
      << "Invalid float size: " << floatBytes;
  return floatBytes == sizeof(float);
}

// struct TensorType
// TODO: Define below functions in terms of block types instead of in terms of
//       the dimensions
size_t TensorType::order() const {
  return getDimensions().size();
}

std::vector<IndexDomain> TensorType::getDimensions() const {
  return dims;
}

std::vector<IndexSet> TensorType::getOuterDimensions() const {
  vector<IndexDomain> dimensions = getDimensions();
  unsigned maxNest = 0;
  for (auto& dim : dimensions) {
    if (dim.getIndexSets().size() > maxNest) {
      maxNest = dim.getIndexSets().size();
    }
  }

  std::vector<IndexSet> outerDimensions;
  for (auto& dim : dimensions) {
    if (dim.getIndexSets().size() == maxNest) {
      outerDimensions.push_back(dim.getIndexSets()[0]);
    }
  }

  return outerDimensions;
}

Type TensorType::getBlockType() const {
  vector<IndexDomain> dimensions = getDimensions();
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
    for (auto& dim : dimensions) {
      if (dim.getIndexSets().size() > maxNesting) {
        maxNesting = dim.getIndexSets().size();
      }
    }

    for (auto& dim : dimensions) {
      if (dim.getIndexSets().size() < maxNesting) {
        const std::vector<IndexSet>& nests = dim.getIndexSets();
        std::vector<IndexSet> blockNests(nests.begin(), nests.end());
        blockDimensions.push_back(IndexDomain(blockNests));
      }
      else {
        const std::vector<IndexSet>& nests = dim.getIndexSets();
        std::vector<IndexSet> blockNests(nests.begin()+1, nests.end());
        blockDimensions.push_back(IndexDomain(blockNests));
      }
    }
    blockType = TensorType::make(componentType, blockDimensions, 
                                 isColumnVector);
  }
  iassert(blockType.defined());

  return blockType;
}

size_t TensorType::size() const {
  vector<IndexDomain> dimensions = getDimensions();
  size_t size = 1;
  for (auto& dimension : dimensions) {
    size *= dimension.getSize();
  }
  return size;
}

// TODO: Get rid of this function. Sparsity is decided elsewhere.
bool TensorType::isSparse() const {
  if (order() < 2) {
    return false;
  }

  vector<IndexDomain> dimensions = getDimensions();
  for (auto& indexDom : dimensions) {
    for (auto& indexSet : indexDom.getIndexSets()) {
      if (indexSet.getKind() != IndexSet::Range) {
        return true;
      }
    }
  }
  return false;
}

bool TensorType::hasSystemDimensions() const {
  vector<IndexDomain> dimensions = getDimensions();
  for (auto& indexDom : dimensions) {
    for (auto& indexSet : indexDom.getIndexSets()) {
      switch (indexSet.getKind()) {
        case IndexSet::Set:
          return true;
        case IndexSet::Dynamic:
        case IndexSet::Range:
        case IndexSet::Single:
          break;
      }
    }
  }
  return false;
}

// struct SetType
SetType::~SetType() {}

// struct UnstructuredSetType
Type UnstructuredSetType::make(Type elementType,
                               const std::vector<Expr>& endpointSets) {
  iassert(elementType.isElement());
  UnstructuredSetType *type = new UnstructuredSetType;
  type->elementType = elementType;
  for (auto& eps : endpointSets) {
    type->endpointSets.push_back(new Expr(eps));
  }
  return type;
}

UnstructuredSetType::~UnstructuredSetType() {
  for (auto& eps : endpointSets) {
    delete eps;
  }
}

// struct GridSetType
Type GridSetType::make(Type elementType, IndexSet underlyingPointSet,
                       size_t dimensions) {
  iassert(elementType.isElement());
  iassert(underlyingPointSet.getKind() == IndexSet::Kind::Set);
  iassert(underlyingPointSet.getSet().type().isUnstructuredSet());
  iassert(underlyingPointSet.getSet().type()
          .toUnstructuredSet()->getCardinality() == 0);
  GridSetType *type = new GridSetType;
  type->elementType = elementType;
  type->underlyingPointSet = underlyingPointSet;
  type->dimensions = dimensions;
  return type;
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
      if (l.isUnstructuredSet()) {
        if (r.isUnstructuredSet()) {
          return *l.toUnstructuredSet() == *r.toUnstructuredSet();
        }
        return false;
      }
      else if (l.isGridSet()) {
        if (r.isGridSet()) {
          return *l.toGridSet() == *r.toGridSet();
        }
        return false;
      }
      unreachable;
    case Type::UnnamedTuple:
      return *l.toUnnamedTuple() == *r.toUnnamedTuple();
    case Type::NamedTuple:
      return *l.toNamedTuple() == *r.toNamedTuple();
    case Type::Array:
      return *l.toArray() == *r.toArray();
    case Type::Opaque:
      return r.kind() == Type::Opaque;
    case Type::Undefined:
      return r.kind() == Type::Undefined;
  }
  unreachable;
  return false;
}

bool operator!=(const Type& l, const Type& r) {
  return !(l == r);
}

bool operator==(const ScalarType& l, const ScalarType& r) {
  return l.kind == r.kind;
}

bool operator==(const TensorType& l, const TensorType& r) {
  if (l.getComponentType() != r.getComponentType()) {
    return false;
  }
  if (l.order() != r.order()) {
    return false;
  }

  vector<IndexDomain> ldimensions = l.getDimensions();
  vector<IndexDomain> rdimensions = r.getDimensions();

  auto li = ldimensions.begin();
  auto ri = rdimensions.begin();
  for (; li != ldimensions.end(); ++li, ++ri) {
    if (*li != *ri) {
      return false;
    }
  }

  return true;
}

bool operator==(const ArrayType& l, const ArrayType& r) {
  return l.elementType == r.elementType && l.size == r.size;
}

bool operator==(const ElementType& l, const ElementType& r) {
  // Element type names are unique
  return (l.name == r.name);
}


bool operator==(const UnstructuredSetType& l, const UnstructuredSetType& r) {
  if (l.getCardinality() != r.getCardinality()) {
    return false;
  }

  for (size_t i = 0; i < l.getCardinality(); ++i) {
    Expr *lexpr = l.endpointSets[i];
    Expr *rexpr = r.endpointSets[i];
    if (*lexpr != *rexpr) return false;
  }
  
  return l.elementType == r.elementType;
}

bool operator==(const GridSetType& l, const GridSetType& r) {
  return l.elementType == r.elementType &&
      l.underlyingPointSet == r.underlyingPointSet &&
      l.dimensions == r.dimensions;
}

bool operator==(const UnnamedTupleType& l, const UnnamedTupleType& r) {
  return l.elementType == r.elementType && l.size == r.size;
}

bool operator==(const NamedTupleType& l, const NamedTupleType& r) {
  if (l.elements.size() != r.elements.size()) {
    return false;
  }

  for (size_t i = 0; i < l.elements.size(); ++i) {
    const Field &lElement = l.elements[i];
    const Field &rElement = r.elements[i];
    if (lElement.name != rElement.name || lElement.type != rElement.type) {
      return false;
    }
  }

  return true;
}

bool operator!=(const ScalarType& l, const ScalarType& r) {
  return !(l == r);
}

bool operator!=(const TensorType& l, const TensorType& r) {
  return !(l == r);
}

bool operator!=(const ElementType& l, const ElementType& r) {
  return !(l == r);
}

bool operator!=(const UnstructuredSetType& l, const UnstructuredSetType& r) {
  return !(l == r);
}

bool operator!=(const GridSetType& l, const GridSetType& r) {
  return !(l == r);
}

bool operator!=(const UnnamedTupleType& l, const UnnamedTupleType& r) {
  return !(l == r);
}

bool operator!=(const NamedTupleType& l, const NamedTupleType& r) {
  return !(l == r);
}

bool operator!=(const ArrayType& l, const ArrayType& r) {
  return !(l == r);
}

std::ostream& operator<<(std::ostream& os, const Type& type) {
  switch (type.kind()) {
    case Type::Tensor:
      return os << *type.toTensor();
    case Type::Element:
      return os << *type.toElement();
    case Type::Set:
      if (type.isUnstructuredSet()) {
        return os << *type.toUnstructuredSet();
      }
      else if (type.isGridSet()) {
        return os << *type.toGridSet();
      }
      else {
        unreachable;
        return os;
      }
    case Type::UnnamedTuple:
      return os << *type.toUnnamedTuple();
    case Type::NamedTuple:
      return os << *type.toNamedTuple();
    case Type::Array:
      return os << *type.toArray();
    case Type::Opaque:
      return os << "opaque";
    case Type::Undefined:
      return os << "undefined type";
  }
  unreachable;
  return os;
}

std::ostream& operator<<(std::ostream& os, const ScalarType& type) {
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
    case ScalarType::String:
      os << "string";
      break;
    case ScalarType::Complex:
      os << "complex";
      break;
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const TensorType& type) {
  if (type.order() == 0) {
    os << type.getComponentType();
  }
  else {
    os << "tensor";
    os << "[" << util::join(type.getOuterDimensions(), ",") << "]";
    os << "(" << type.getBlockType() << ")";
    if (type.getDimensions().size() == 1 && !type.isColumnVector) {
      os << "'";
    }
  }
  return os;
}

std::ostream& operator<<(std::ostream& os, const ElementType& type) {
  os << type.name;
  return os;
}

std::ostream& operator<<(std::ostream& os, const UnstructuredSetType& type) {
  os << "set{" << type.elementType.toElement()->name << "}";

  if (type.endpointSets.size() > 0) {
    os << "(" << *type.endpointSets[0];
    for (auto& epSet : util::excludeFirst(type.endpointSets)) {
      os << ", " << *epSet;
    }
    os << ")";
  }
  
  return os;
}

std::ostream& operator<<(std::ostream& os, const GridSetType& type) {
  os << "grid[" << type.dimensions << "]{"
     << type.elementType.toElement()->name << "}("
     << type.underlyingPointSet << ")";

  return os;
}

std::ostream& operator<<(std::ostream& os, const UnnamedTupleType& type) {
  return os << "(" << type.elementType.toElement()->name << "*" << type.size
            << ")";
}

std::ostream& operator<<(std::ostream& os, const NamedTupleType& type) {
  os << "(";

  bool printDelimiter = false;
  for (auto &element : type.elements) {
    if (printDelimiter) {
      os << ", ";
    }

    os << element.name << " : " << element.type;
    printDelimiter = true;
  }

  return os << ")";
}

std::ostream& operator<<(std::ostream& os, const ArrayType& type) {
  os << type.elementType;
  if (type.size > 0) {
    os << "[" << type.size << "]";
  }
  else {
    os << "*";
  }
  return os;
}


}} // namespace simit::ir
