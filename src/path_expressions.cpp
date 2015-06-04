#include "path_expressions.h"

#include <string>
#include "graph.h"

namespace simit {
namespace pe {

// class EV
EV::EV(simit::Set *E, simit::Set *V, unsigned edgeEndpoint) {
  this->E = E;
  this->V = V;
  this->edgeEndpoint = edgeEndpoint;
}

const simit::Set *EV::getPathEndpoint(unsigned pathEndpoint) const {
  iassert(pathEndpoint < 2);
  return (pathEndpoint == 0) ? E : V;
}

void EV::print(std::ostream &os) const {
  std::string Ename = (E->getName() != "") ? E->getName() : "E";
  std::string Vname = (V->getName() != "") ? V->getName() : "V";
  os << Ename << "-" << Vname;
}

// class VE
VE::VE(simit::Set *V, simit::Set *E, unsigned edgeEndpoint) {
  this->V = V;
  this->E = E;
  this->edgeEndpoint = edgeEndpoint;
}

const simit::Set *VE::getPathEndpoint(unsigned pathEndpoint) const {
  iassert(pathEndpoint < 2);
  return (pathEndpoint == 0) ? V : E;
}

void VE::print(std::ostream &os) const {
  std::string Vname = (V->getName() != "") ? V->getName() : "V";
  std::string Ename = (E->getName() != "") ? E->getName() : "E";
  os << Vname << "-" << Ename;
}

// class PathExpression
const simit::Set *PathExpression::getPathEndpoint(unsigned pathEndpoint) const {
  return ptr->getPathEndpoint(pathEndpoint);
}

std::ostream &operator<<(std::ostream& os, const PathExpression& pe) {
  os << *pe.ptr;
  return os;
}

}}
