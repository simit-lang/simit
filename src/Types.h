#ifndef SIMIT_TYPES_H
#define SIMIT_TYPES_H

#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <memory>

namespace simit {

class Shape;

class Type {
 public:
  Type() {}
  virtual ~Type();

  virtual bool operator==(const Type& other) = 0;
  bool operator!=(const Type& other) { return !(*this == other); }
  virtual operator std::string() const = 0;
};


class ElementType : public Type {
 public:
  ElementType() {}
  virtual ~ElementType();

  virtual bool operator==(const Type& other);
  virtual operator std::string() const;
};


class TensorType :public Type {
 public:
  enum ComponentType {INT, FLOAT};
  static std::size_t componentSize(ComponentType ct);
  static std::string componentTypeString(ComponentType ct);

  TensorType() {}
  virtual ~TensorType();

  virtual unsigned int getOrder() const = 0;
  virtual unsigned int getSize() const = 0;
  virtual bool isScalar() const = 0;

  virtual std::unique_ptr<std::list<Shape*> > getShapes() const = 0;
  virtual ComponentType getComponentType() const = 0;
};


class ScalarType : public TensorType {
 public:
  ScalarType(ComponentType componentType) : componentType(componentType) {}
  virtual ~ScalarType();

  virtual unsigned int getOrder() const { return 0; }
  virtual unsigned int getSize() const;
  virtual bool isScalar() const { return true; }

  virtual std::unique_ptr<std::list<Shape*> > getShapes() const;
  virtual ComponentType getComponentType() const;

  virtual bool operator==(const Type& other);
  virtual operator std::string() const;

 private:
  ComponentType componentType;
};


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

 private:
  Type type;
  union {
    unsigned int size;
  };
};


class Shape {
 public:
  Shape()  {}
  Shape(const std::vector<Dimension*> &dimensions) : dimensions(dimensions) {}
  ~Shape();

  unsigned int getOrder() const { return dimensions.size(); }
  unsigned int getSize() const;

  std::vector<Dimension *>::iterator begin() {
    return dimensions.begin();
  }
  std::vector<Dimension *>::iterator end() {
    return dimensions.end();
  }

  bool operator==(const Shape &other) const;
  bool operator!=(const Shape &other) const { return !(*this == other); }
  operator std::string() const;

 private:
  std::vector<Dimension*> dimensions;
};


class NDTensorType : public TensorType {
 public:
  NDTensorType(Shape *blockShape, TensorType *blockType)
      : blockShape(blockShape), blockType(blockType) {};
  virtual ~NDTensorType();

  virtual unsigned int getOrder() const { return blockShape->getOrder(); }
  virtual unsigned int getSize() const;
  virtual bool isScalar() const { return false; }

  virtual std::unique_ptr<std::list<Shape*> > getShapes() const;
  virtual ComponentType getComponentType() const;

  virtual bool operator==(const Type& other);
  virtual operator std::string() const;

 private:
  Shape      *blockShape;
  TensorType *blockType;
};

}

#endif
