#ifndef SIMIT_VAR_H
#define SIMIT_VAR_H

#include "types.h"
#include "intrusive_ptr.h"

namespace simit {
namespace ir {

struct ForDomain;

namespace {
struct VarContent {
  std::string name;
  Type type;

  mutable long ref = 0;
  friend inline void aquire(VarContent *c) {++c->ref;}
  friend inline void release(VarContent *c) {if (--c->ref==0) delete c;}
};
}

/// A Simit variable.
class Var : public util::IntrusivePtr<VarContent> {
public:
  Var() : IntrusivePtr() {}
  Var(std::string name, const Type &type);

  const std::string &getName() const {return ptr->name;}
  const Type &getType() const {return ptr->type;}
};

std::ostream &operator<<(std::ostream &os, const Var &);

}}

#endif
