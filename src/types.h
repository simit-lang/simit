#ifndef SIMIT_TYPES_H
#define SIMIT_TYPES_H

#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#include <map>
#include <iostream>

#include "domain.h"

// TODO: Refactor the type system:
//       - Make the Type class work similar to Expr
//       - Make ScalarType a Type TypeNode instance

namespace simit {
namespace ir {

class Expr;

struct TypeNode {};
struct ScalarType;
struct TensorType;
struct ElementType;
struct SetType;
struct TupleType;
struct ArrayType;

class Type {
public:
  enum Kind {Tensor, Element, Set, Tuple, Array};
  Type() : ptr(nullptr) {}
  Type(TensorType* tensor);
  Type(ElementType* element);
  Type(SetType* set);
  Type(TupleType* tuple);
  Type(ArrayType* array);

  bool defined() const { return ptr != nullptr; }

  Kind kind() const { return _kind; }

  bool isTensor()  const { return _kind==Tensor; }
  bool isElement() const { return _kind==Element; }
  bool isSet()     const { return _kind==Set; }
  bool isTuple()   const { return _kind==Tuple; }
  bool isArray()   const { return _kind==Array; }

  const TensorType*  toTensor()  const {iassert(isTensor());  return tensor;}
  const ElementType* toElement() const {iassert(isElement()); return element;}
  const SetType*     toSet()     const {iassert(isSet());     return set;}
  const TupleType*   toTuple()   const {iassert(isTuple());   return tuple;}
  const ArrayType*   toArray()   const {iassert(isArray());   return array;}

private:
  Kind _kind;
  union {
    TensorType  *tensor;
    ElementType *element;
    SetType     *set;
    TupleType   *tuple;
    ArrayType   *array;
  };
  std::shared_ptr<TypeNode> ptr;
};

struct ScalarType {
  enum Kind {Float, Int, Boolean};

  ScalarType() : kind(Int) {}
  ScalarType(Kind kind) : kind(kind) {}

  static unsigned floatBytes;

  Kind kind;

  static bool singleFloat();

  // TODO: Add variable bit sizes later
//  unsigned bits;
//  unsigned bytes() const { return (bits + 7) / 8; }

  unsigned bytes() const {
    if (isInt()) {
      return 4;
    }
    else if (isBoolean()) {
      return (unsigned int)sizeof(bool);
    }
    else {
      iassert(isFloat());
      return floatBytes;
    }
  }

  bool isInt () const { return kind == Int; }
  bool isFloat() const { return kind == Float; }
  bool isBoolean() const { return kind == Boolean; }
};

// TODO: Change the implementation of TensorType store a blockType plus outer
//       dimensions.  With the current scheme we cannot distinguish between a
//       vector of matrices and a matrix of vectors, both of which must be
//       permitted.
struct TensorType : TypeNode {
  ScalarType componentType;
  std::vector<IndexDomain> dims;

  /// Marks whether the tensor type is a column vector.  This information is
  /// not used by the Simit compiler, but is here to ease frontend development.
  bool isColumnVector;

  size_t order() const;

  std::vector<IndexDomain> getDimensions() const;

  /// Returns the dimensions of a tensor where each block is a component.  These
  /// dimensions are not nested.  Note also that it is allowed for a tensor to
  /// have fewer outer than inner dimensions (e.g. a vector of matrices).
  std::vector<IndexSet> getOuterDimensions() const;

  /// Returns the type of the blocks in this tensor.
  Type getBlockType() const;

  size_t size() const;

  bool isSparse() const;
  bool hasSystemDimensions() const;

  static Type make(ScalarType componentType) {
    std::vector<IndexDomain> dimensions;
    return TensorType::make(componentType, dimensions);
  }

  static Type make(ScalarType componentType,
                   std::vector<IndexDomain> dimensions,
                   bool isColumnVector = false) {
    TensorType *type = new TensorType;
    type->componentType = componentType;
    type->dims = dimensions;
    type->isColumnVector = isColumnVector;
    return type;
  }
};

struct Field {
  Field(std::string name, Type type) : name(name), type(type) {}

  std::string name;
  Type type;
};

struct ElementType : TypeNode {
  std::string name;

  /// Maps field names to their types and locations in the element
  std::vector<Field> fields;

  /// Lookup data structure, use the field method to access fields by names.
  std::map<std::string,unsigned> fieldNames;

  bool hasField(std::string fieldName) const {
    return fieldNames.find(fieldName) != fieldNames.end();
  }

  const Field& field(const std::string& fieldName) const {
    iassert(hasField(fieldName)) << "Undefined field '"
                                 << fieldName << "' in '" << name << "'";
    return fields[fieldNames.at(fieldName)];
  }

  static Type make(std::string name, std::vector<Field> fields) {
    ElementType *type = new ElementType;
    type->name = name;
    type->fields = fields;
    for (size_t i=0; i < fields.size(); ++i) {
      type->fieldNames[fields[i].name] = i;
    }
    return type;
  }
};


/// The type of a Simit set.  If the set is an edge set then the vector
/// endpointSets contains the sets each endpoint of an edge comes from.
struct SetType : TypeNode {
  Type elementType;

  /// Endpoint sets.  These are stored as pointers to break an include cycle
  /// between this file and ir.h.
  std::vector<Expr*> endpointSets;

  // TODO: Add method to retrieve a set field (compute from elementType fields)

  static Type make(Type elementType, const std::vector<Expr>& endpointSets);
  ~SetType();
};

struct TupleType : TypeNode {
  Type elementType;
  int size;

  static Type make(Type elementType, int size) {
    iassert(elementType.isElement());
    TupleType *type = new TupleType;
    type->elementType = elementType;
    type->size = size;
    return type;
  }
};

struct ArrayType : TypeNode {
  ScalarType elementType;

  unsigned size;  // optional size: 0 means dynamic size

  static Type make(ScalarType elementType, unsigned size=0) {
    ArrayType* type = new ArrayType;
    type->elementType = elementType;
    type->size = size;
    return type;
  }
};


// Type functions
inline Type::Type(TensorType* tensor)
    : _kind(Tensor), tensor(tensor), ptr(tensor) {}
inline Type::Type(ElementType* element)
    : _kind(Element), element(element), ptr(element) {}
inline Type::Type(SetType* set)
    : _kind(Set), set(set), ptr(set) {}
inline Type::Type(TupleType* tuple)
    : _kind(Tuple), tuple(tuple), ptr(tuple) {}
inline Type::Type(ArrayType* array)
    : _kind(Array), array(array), ptr(array) {}


inline bool isScalar(Type type) {
  return type.kind()==Type::Tensor && type.toTensor()->order() == 0;
}

inline bool isBoolean(Type type) {
  return type.kind()==Type::Tensor && type.toTensor()->order() == 0 &&
      type.toTensor()->componentType.isBoolean();
}

/// An element tensor type is one whose dimensions are not sets
inline bool isElementTensorType(const TensorType *type) {
  bool isElementType = true;
  for (auto& dim : type->getDimensions()) {
    for (auto& is : dim.getIndexSets()) {
      if (is.getKind() == IndexSet::Set) {
        isElementType = false;
      }
    }
  }
  return isElementType;
}

bool operator==(const Type&, const Type&);
bool operator!=(const Type&, const Type&);

bool operator==(const ScalarType&, const ScalarType&);
bool operator==(const TensorType&, const TensorType&);
bool operator==(const ElementType&, const ElementType&);
bool operator==(const SetType&, const SetType&);
bool operator==(const TupleType&, const TupleType&);
bool operator==(const ArrayType&, const ArrayType&);

bool operator!=(const ScalarType&, const ScalarType&);
bool operator!=(const TensorType&, const TensorType&);
bool operator!=(const ElementType&, const ElementType&);
bool operator!=(const SetType&, const SetType&);
bool operator!=(const TupleType&, const TupleType&);
bool operator!=(const ArrayType&, const ArrayType&);

std::ostream& operator<<(std::ostream&, const Type&);
std::ostream& operator<<(std::ostream&, const ScalarType&);
std::ostream& operator<<(std::ostream&, const TensorType&);
std::ostream& operator<<(std::ostream&, const ElementType&);
std::ostream& operator<<(std::ostream&, const SetType&);
std::ostream& operator<<(std::ostream&, const TupleType&);
std::ostream& operator<<(std::ostream&, const ArrayType&);

// Common types
const Type Int = TensorType::make(ScalarType(ScalarType::Int));
const Type Float = TensorType::make(ScalarType(ScalarType::Float));
const Type Boolean = TensorType::make(ScalarType(ScalarType::Boolean));

}}

#endif
