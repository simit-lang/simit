#include "path_expressions.h"

#include <string>
#include "error.h"

namespace simit {
namespace pe {

// struct ElementVar
struct ElementVar::ElementVarContent {
  std::string setName;
};

ElementVar::ElementVar() : content(nullptr) {
}

ElementVar::ElementVar(std::string setName) : content(new ElementVarContent) {
  content->setName = setName;
}

std::string ElementVar::getSetName() const {
  return content->setName;
}

bool operator==(const ElementVar& lhs, const ElementVar& rhs) {
  return lhs.content == rhs.content;
}

bool operator!=(const ElementVar& lhs, const ElementVar& rhs) {
  return lhs.content != rhs.content;
}

std::ostream &operator<<(std::ostream& os, const ElementVar& v) {
  os << v.getSetName() << "_i" << " in " << v.getSetName();
  return os;
}

// class Link
Link::Link(ElementVar E, ElementVar V, unsigned edgeEndpoint) {
  this->E = E;
  this->V = V;
  this->edgeEndpoint = edgeEndpoint;
}

PathExpression Link::make(ElementVar E, ElementVar V, unsigned edgeEndpoint) {
  return PathExpression(new Link(E, V, edgeEndpoint));
}

ElementVar Link::getPathEndpoint(unsigned pathEndpoint) const {
  iassert(pathEndpoint < 2);
  return (pathEndpoint == 0) ? E : V;
}

void Link::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}

void Link::print(std::ostream &os) const {
  os << "(" << E << ")-" << edgeEndpoint << "-(" << V << ")";
}


// class Predicate
Predicate::Predicate() {
}

ElementVar Predicate::getPathEndpoint(unsigned pathEndpoint) const {
}

void Predicate::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}

void Predicate::print(std::ostream &os) const {
}


// class PathExpression
ElementVar PathExpression::getPathEndpoint(unsigned pathEndpoint) const {
  return ptr->getPathEndpoint(pathEndpoint);
}

std::ostream &operator<<(std::ostream& os, const PathExpression& pe) {
  os << *pe.ptr;
  return os;
}

}}
