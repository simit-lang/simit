#include "Types.h"

#include <assert.h>
#include <climits>
#include <iostream>

#include "Util.h"

using namespace std;

namespace simit {
  /* Type */
  Type::~Type() {}


  /* ElementType */
  ElementType::~ElementType() {}

  ElementType::operator std::string() const {
    return "ElementType";
  }


  /* TensorType */
  std::size_t TensorType::componentSize(ComponentType ct) {
    switch (ct) {
      case TensorType::INT:
        return sizeof(int);
      case TensorType::FLOAT:
        return sizeof(double);
    }
    assert(false);
    return 0;
  }

  std::string TensorType::componentTypeString(ComponentType ct) {
    switch (ct) {
      case TensorType::INT:
        return "int";
      case TensorType::FLOAT:
        return "float";
    }
    assert(false);
    return "";
  }
  
  TensorType::~TensorType() {}


  /* ScalarType */
  ScalarType::~ScalarType() {}

  unsigned int ScalarType::getSize() const {
    return 1;
  }

  unique_ptr<std::list<Shape*> > ScalarType::getShape() const {
    return unique_ptr<list<Shape*> >(new list<Shape*>);
  }

  TensorType::ComponentType ScalarType::getComponentType() const {
    return componentType;
  }

  ScalarType::operator std::string() const {
    switch (componentType) {
      case INT:
        return "int";
      case FLOAT:
        return "float";
    }
    assert(false);
    return "";
  }


  /* Shape */
  Shape::~Shape() {
    for (auto dim : dimensions) {
      delete dim;
    }
  }

  unsigned int Shape::getSize() const {
    unsigned int shapeSize = 1;
    for (Dimension *dim : dimensions) {
      shapeSize *= dim->getSize();
    }
    return shapeSize;
  }

  Shape::operator std::string() const {
    return "[" + util::join(dimensions, ",") + "]";
  }


  /* Dimension */
  unsigned int Dimension::getSize() const {
    switch (type) {
      case VARIABLE:
        assert(false);
        return UINT_MAX; // TODO: Fix this by storing var sizes in types
      case ANONYMOUS:
        return size;
    }
  }

  Dimension::operator std::string() const {
    switch (type) {
      case VARIABLE:
        return "*";
      case ANONYMOUS:
        return to_string(size);
    }
    assert(false);
    return "";
  }


  /* NDTensorType */
  NDTensorType::~NDTensorType() {
    delete blockShape;
    delete blockType;
  }

  unsigned int NDTensorType::getSize() const {
    return blockShape->getSize() * blockType->getSize();
  }

  unique_ptr<std::list<Shape*> > NDTensorType::getShape() const {
    auto subShape = blockType->getShape();
    subShape->push_front(blockShape);
    return subShape;
  }

   TensorType::ComponentType NDTensorType::getComponentType() const {
    return blockType->getComponentType();
  }

  NDTensorType::operator std::string() const {
    string componentTypeStr = componentTypeString(getComponentType());
    string componentShapeStr = util::join(*getShape(), "");
    return string("Tensor") + componentShapeStr + "(" + componentTypeStr + ")";
  }
}
