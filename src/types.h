#ifndef SIMIT_TYPES_H
#define SIMIT_TYPES_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <memory>

namespace simit {
namespace internal {

class Shape;

/** The type of a Simit \ref Tensor. */
class TensorType {
 public:
  enum ComponentType {INT, FLOAT, ELEMENT};
  static std::size_t componentSize(ComponentType ct);
  static std::string componentTypeString(ComponentType ct);

  TensorType() {}
  virtual ~TensorType();

  virtual unsigned int getOrder() const = 0;
  virtual unsigned int getSize() const = 0;
  virtual bool isScalar() const = 0;
  virtual ComponentType getComponentType() const = 0;

  virtual bool operator==(const TensorType& other) = 0;
  bool operator!=(const TensorType& other) { return !(*this == other); }

  virtual std::string toString() const = 0;
  friend std::ostream &operator<<(std::ostream &os, const TensorType &type);
};


/** The type of a Scalar \ref Tensor. */
class ScalarType : public TensorType {
 public:
  ScalarType(ComponentType componentType) : componentType(componentType) {}
  virtual ~ScalarType();

  virtual unsigned int getOrder() const { return 0; }
  virtual unsigned int getSize() const;
  virtual bool isScalar() const { return true; }
  virtual ComponentType getComponentType() const;

  virtual bool operator==(const TensorType& other);
  virtual std::string toString() const;

 private:
  ComponentType componentType;
};


/** Describes one of the dimensions of a \ref Tensor \ref Shape.  A dimension
  * can be described by a range of integers, a set of objects, or have a
  * variable size. */
class Dimension {
 public:
  enum Type {ANONYMOUS, /*SET,*/ VARIABLE};

  Dimension(unsigned int size) : type(ANONYMOUS), size(size) {}
  Dimension()                  : type(VARIABLE)              {}
  Dimension(const Dimension& other);
  ~Dimension() {}

  unsigned int getSize() const;

  bool operator==(const Dimension &other) const;
  bool operator!=(const Dimension &other) const { return !(*this == other); }
  operator std::string() const;
  friend std::ostream &operator<<(std::ostream &os, const Dimension &obj) {
    return os << std::string(obj);
  }
  
 private:
  Type type;
  union {
    unsigned int size;
  };
};


/** Describes the shape of each \ref Dimension of a \ref Tensor. */
class Shape {
 public:
  Shape()  {}
  Shape(const std::vector<Dimension*> &dimensions) : dimensions(dimensions) {}
  ~Shape();

  unsigned int getOrder() const { return dimensions.size(); }
  unsigned int getSize() const;

  bool operator==(const Shape &other) const;
  bool operator!=(const Shape &other) const { return !(*this == other); }
  operator std::string() const;
  friend std::ostream &operator<<(std::ostream &os, const Shape &obj) {
    return os << std::string(obj);
  }

  std::vector<Dimension *>::const_iterator begin() const {
    return dimensions.begin();
  }
  std::vector<Dimension *>::const_iterator end() const {
    return dimensions.end();
  }

 private:
  std::vector<Dimension*> dimensions;
};


/** The type of a \ref Tensor of order 1 or higher. */
class NDTensorType : public TensorType {
 public:
  NDTensorType(Shape *blockShape, TensorType *blockType)
      : blockShape(blockShape), blockType(blockType) {};
  virtual ~NDTensorType();

  virtual unsigned int getOrder() const { return blockShape->getOrder(); }
  virtual unsigned int getSize() const;
  virtual bool isScalar() const { return false; }

  virtual ComponentType getComponentType() const;

  virtual bool operator==(const TensorType& other);
  virtual std::string toString() const;

 private:
  Shape      *blockShape;
  TensorType *blockType;
};

}} // namespace simit::internal

#endif
