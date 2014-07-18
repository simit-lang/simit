#include "Types.h"

#include <assert.h>
#include <climits>
#include <iostream>

#include "Util.h"

using namespace simit;
using namespace std;


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

unique_ptr<std::list<Shape*> > ScalarType::getShapes() const {
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


/* Dimension */
Dimension::Dimension(const Dimension& other) {
  type = other.type;
  switch (type) {
    case ANONYMOUS:
      size = other.size;
      break;
    case VARIABLE:
      assert(false);  // TODO: Not supported yet
      break;
    default:
      assert(false);
      break;
  };
}

unsigned int Dimension::getSize() const {
  switch (type) {
    case ANONYMOUS:
      return size;
    case VARIABLE:
      assert(false);
      return UINT_MAX; // TODO: Fix this by storing var sizes in types
  }
}

Dimension::operator std::string() const {
  switch (type) {
    case ANONYMOUS:
      return to_string(size);
    case VARIABLE:
      return "*";
  }
  assert(false);
  return "";
}

bool Dimension::operator==(const Dimension &other) const {
  if (type != other.type) {
    return false;
  }
  switch (type) {
    case ANONYMOUS:
      if (size != other.size) {
        return false;
      }
      break;
    case VARIABLE:
      assert(false);  // TODO: Not supported yet
      break;
    default:
      assert(false);
      break;
  }

  return true;
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


/* NDTensorType */
NDTensorType::~NDTensorType() {
  delete blockShape;
  delete blockType;
}

unsigned int NDTensorType::getSize() const {
  return blockShape->getSize() * blockType->getSize();
}

unique_ptr<std::list<Shape*> > NDTensorType::getShapes() const {
  auto subShape = blockType->getShapes();
  subShape->push_front(blockShape);
  return subShape;
}

TensorType::ComponentType NDTensorType::getComponentType() const {
  return blockType->getComponentType();
}

NDTensorType::operator std::string() const {
  string componentTypeStr = componentTypeString(getComponentType());
  string componentShapeStr = util::join(*getShapes(), "");
  return string("Tensor") + componentShapeStr + "(" + componentTypeStr + ")";
}
