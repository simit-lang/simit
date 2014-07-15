#ifndef SIMIT_TYPES_H
#define SIMIT_TYPES_H

#include <string>
#include <vector>

namespace Simit {
  class Shape;
  class Dimension;

  class Type {
  public:
    virtual operator std::string() const = 0;
    friend std::ostream& operator<<(std::ostream &out, const Type &type);
  };

  class ElementType : public Type {};

  class TensorType :public Type {
  public:
    TensorType();
    virtual ~TensorType();

  };

  class NDTensorType : public TensorType {
  public:
    NDTensorType(TensorType *blockType, Shape *blockShape);
    virtual ~NDTensorType();

  private:
    TensorType *blockType;
    Shape      *blockShape;
  };

  class ScalarType : public TensorType {
  public:
    enum Type {INT, FLOAT, DOUBLE};

    ScalarType();
    virtual ~ScalarType();

  private:
    ScalarType::Type scalarType;
  };

  class Shape {
  public:
  private:
    std::vector<Dimension *> dimensions;
  };

  class Dimension {
  public:
    enum Type {VARIABLE, ANONYMOUS, SET};

    Dimension();
    Dimension(unsigned int size);
    ~Dimension();

    operator std::string() const;

  private:
    Type type;
    union {
      int size;
    };
  };
  
}

#endif
