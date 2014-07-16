#ifndef SIMIT_TYPES_H
#define SIMIT_TYPES_H

#include <string>
#include <vector>
#include <list>

namespace simit {
  class Shape;
  class Dimension;
  class ScalarType;

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
    TensorType() {}
    virtual ~TensorType();

    virtual std::unique_ptr<std::list<Shape*> > getComponentShapes() const = 0;
    virtual ScalarType *getComponentType() const = 0;
  };


  class NDTensorType : public TensorType {
  public:
    NDTensorType(Shape *blockShape, TensorType *blockType)
        : blockShape(blockShape), blockType(blockType) {};
    virtual ~NDTensorType();

    virtual std::unique_ptr<std::list<Shape*> > getComponentShapes() const;
    virtual ScalarType *getComponentType() const;
    virtual operator std::string() const;

  private:
    Shape      *blockShape;
    TensorType *blockType;
  };


  class ScalarType : public TensorType {
  public:
    enum Type {INT, FLOAT};

    ScalarType(Type type) : type(type) {}
    virtual ~ScalarType();

    virtual std::unique_ptr<std::list<Shape*> > getComponentShapes() const;
    virtual ScalarType *getComponentType() const;
    virtual operator std::string() const;

  private:
    ScalarType::Type type;
  };


  class Shape {
  public:
    Shape()  {}
    Shape(const std::vector<Dimension*> &dimensions) : dimensions(dimensions) {}
    ~Shape();

    operator std::string() const;

  private:
    std::vector<Dimension*> dimensions;
  };


  class Dimension {
  public:
    enum Type {VARIABLE, ANONYMOUS, SET};

    Dimension()                  : type(VARIABLE)              {}
    Dimension(unsigned int size) : type(ANONYMOUS), size(size) {}
    ~Dimension() {}

    operator std::string() const;

  private:
    Type type;
    union {
      int size;
    };
  };
}

#endif
