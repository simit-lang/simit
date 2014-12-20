#include "var.h"

#include <ostream>
#include "ir.h"

namespace simit {
namespace ir {

// class Var
Var::Var(std::string name, const Type &type) : IntrusivePtr(new VarContent) {
  ptr->name = name;
  ptr->type = type;
}
// Free functions
std::ostream &operator<<(std::ostream &os, const Var &v) {
  return os << v.getName();
}

}}
