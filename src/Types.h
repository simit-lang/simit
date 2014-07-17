#ifndef SIMIT_TYPES_H
#define SIMIT_TYPES_H

#include <string>
#include <vector>
#include <list>

namespace simit {

class Shape;

class Type {
 public:
  Type() {}
  virtual ~Type();

  virtual operator std::string() const = 0;
};


class ElementType : public Type {
 public:
  ElementType() {}
  virtual ~ElementType();

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

  virtual std::unique_ptr<std::list<Shape*> > getShape() const = 0;
  virtual ComponentType getComponentType() const = 0;
};


class ScalarType : public TensorType {
 public:
  ScalarType(ComponentType componentType) : componentType(componentType) {}
  virtual ~ScalarType();

  virtual unsigned int getOrder() const { return 0; }
  virtual unsigned int getSize() const;

  virtual std::unique_ptr<std::list<Shape*> > getShape() const;
  virtual ComponentType getComponentType() const;

  virtual operator std::string() const;

 private:
  ComponentType componentType;
};


class Dimension {
 public:
  enum Type {VARIABLE, ANONYMOUS, /*SET*/};

  Dimension()                  : type(VARIABLE)              {}
  Dimension(unsigned int size) : type(ANONYMOUS), size(size) {}
  ~Dimension() {}

  unsigned int getSize() const;
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

  virtual std::unique_ptr<std::list<Shape*> > getShape() const;
  virtual ComponentType getComponentType() const;

  virtual operator std::string() const;

 private:
  Shape      *blockShape;
  TensorType *blockType;
};

}

#endif
