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

bool operator==(const ElementVar &lhs, const ElementVar &rhs) {
  return lhs.content == rhs.content;
}

bool operator<(const ElementVar &lhs, const ElementVar &rhs) {
  return lhs.content < rhs.content;
}

std::ostream &operator<<(std::ostream& os, const ElementVar& v) {
  os << v.getSetName() << "_i" << " in " << v.getSetName();
  return os;
}


// class PathExpression
ElementVar PathExpression::getPathEndpoint(unsigned pathEndpoint) const {
  return ptr->getPathEndpoint(pathEndpoint);
}

void PathExpression::accept(PathExpressionVisitor *visitor) const {
  ptr->accept(visitor);
}

std::ostream &operator<<(std::ostream& os, const PathExpression& pe) {
  os << *pe.ptr;
  return os;
}


// class EV
EV::EV(ElementVar E, ElementVar V) : E(E), V(V) {
}

PathExpression EV::make(ElementVar E, ElementVar V) {
  return PathExpression(new EV(E, V));
}

ElementVar EV::getPathEndpoint(unsigned pathEndpoint) const {
  iassert(pathEndpoint < 2);
  return (pathEndpoint == 0) ? E : V;
}

void EV::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}

void EV::print(std::ostream &os) const {
  os << "(" << E << ")-(" << V << ")";
}


// class VE
VE::VE(ElementVar V, ElementVar E) : V(V), E(E) {
}

PathExpression VE::make(ElementVar V, ElementVar E) {
  return PathExpression(new VE(V, E));
}

ElementVar VE::getPathEndpoint(unsigned pathEndpoint) const {
  iassert(pathEndpoint < 2);
  return (pathEndpoint == 0) ? V : E;
}

void VE::accept(PathExpressionVisitor *visitor) const {
  visitor->visit(this);
}

void VE::print(std::ostream &os) const {
  os << "(" << V << ")-(" << E << ")";
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

}}
