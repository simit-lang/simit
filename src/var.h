#ifndef SIMIT_VAR_H
#define SIMIT_VAR_H

#include "types.h"
#include "intrusive_ptr.h"

namespace simit {
namespace ir {

class ReductionOperator;
class VarCleaner;
struct ForDomain;

struct VarContent {
  std::string name;
  Type type;

  mutable long ref = 0;
  friend inline void aquire(VarContent *c) {++c->ref;}
  friend inline void release(VarContent *c) {if (--c->ref==0) delete c;}
};

/// A Simit variable.
class Var : public util::IntrusivePtr<VarContent> {
public:
  Var() : IntrusivePtr() {}
  Var(std::string name, const Type &type);

  const std::string &getName() const {return ptr->name;}
  const Type &getType() const {return ptr->type;}

  friend class VarCleaner;
};

/// A privileged class which allows cleaning Var names. Used for backend
/// code generation when particular symbols are protected for a specific target.
class VarCleaner {
public:
  // Default impl does nothing
  virtual std::string cleanImpl(std::string name) {return name;}
  void clean(Var &inp) {
    inp.ptr->name = cleanImpl(inp.ptr->name);
  }
  // Hack: break the const contract very specifically
  void clean(const Var &inp) {
    Var &inpFree = const_cast<Var&>(inp);
    clean(inpFree);
  }
};

/// A Simit loop variable, consisting of a variable and its iteration domain.
class LoopVar {
public:
  LoopVar(Var var, const ForDomain &domain);
  LoopVar(Var var, const ForDomain &domain, const ReductionOperator &rop);

  const Var &getVar() const;
  const ForDomain &getDomain() const;

  bool hasReduction() const;
  ReductionOperator getReductionOperator() const;

private:
  struct Content;
  std::shared_ptr<Content> content;
};

std::ostream &operator<<(std::ostream &os, const Var &);
std::ostream &operator<<(std::ostream &os, const LoopVar &);

}}

#endif
