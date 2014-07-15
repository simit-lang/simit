#include "Types.h"

#include <assert.h>
#include <iostream>

#include "Util.h"

using namespace std;

namespace simit {
  /* Type */
  Type::~Type() {}


  /* TensorType */
  TensorType::~TensorType() {}


  /* ElementType */
  ElementType::~ElementType() {}

  ElementType::operator std::string() const {
    return "ElementType";
  }


  /* NDTensorType */
  NDTensorType::~NDTensorType() {
    delete blockShape;
    delete blockType;
  }

  unique_ptr<std::list<Shape*> > NDTensorType::getComponentShapes() const {
    auto blockComponentShape = blockType->getComponentShapes();
    blockComponentShape->push_front(blockShape);
    return blockComponentShape;
  }

  ScalarType *NDTensorType::getComponentType() const {
    return blockType->getComponentType();
  }

  NDTensorType::operator std::string() const {
    string componentTypeStr = string(*getComponentType());
    string componentShapeStr = util::join(*getComponentShapes(), "");
    return string("Tensor") + componentShapeStr + "(" + componentTypeStr + ")";
  }


  /* ScalarType */
  ScalarType::~ScalarType() {}

  unique_ptr<std::list<Shape*> > ScalarType::getComponentShapes() const {
    return unique_ptr<list<Shape*> >(new list<Shape*>);
  }

  ScalarType *ScalarType::getComponentType() const {
    return const_cast<ScalarType*>(this);
  }

  ScalarType::operator std::string() const {
    switch (type) {
      case INT:
        return "int";
      case FLOAT:
        return "float";
      case DOUBLE:
        return "double";
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

  Shape::operator std::string() const {
    return "[" + util::join(dimensions, ", ") + "]";
  }


  /* Dimension */
  Dimension::operator std::string() const {
    switch (type) {
      case VARIABLE:
        return "*";
      case ANONYMOUS:
        return to_string(size);
      case SET:
        assert(false); // Not supported yet
    }
    assert(false);
    return "";
  }
}
