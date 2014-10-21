#ifndef SIMIT_USEDEF_H
#define SIMIT_USEDEF_H

#include <map>
#include "ir.h"

namespace simit {
namespace ir {

/// For each variable, compute the location where it was defined.  This pass
/// assumes that each variable is only defined once (SSA).
class VarDef {
public:
  enum Kind { Argument, Result, Assignment, Undefined };

  explicit VarDef(Kind kind=Undefined, Stmt stmt = Stmt())
      : kind(kind), stmt(stmt) {}

  Kind getKind() const {return kind;}

  Expr getAssignedValue() const {
    assert(kind == Assignment);
    return to<AssignStmt>(stmt)->value;
  }

private:
  Kind kind;
  Stmt stmt;
};

std::ostream &operator<<(std::ostream &os, const VarDef &vd);


class UseDef : public IRVisitor {
public:
  class const_iterator {
  public:
    const_iterator(std::map<const Var*,VarDef>::const_iterator it) : it(it) {}
    const_iterator& operator++() {++it; return *this;}
    const Var &operator*() const {return *it->first;}

    friend bool operator==(const const_iterator &l, const const_iterator &r) {
      return l.it == r.it;
    }

    friend bool operator!=(const const_iterator &l, const const_iterator &r) {
      return l.it != r.it;
    }

  private:
    std::map<const Var*,VarDef>::const_iterator it;
  };

  UseDef(Func func) {
    for (const Var &arg : func.getArguments()) {
      useDef[&arg] = VarDef(VarDef::Argument);
    }

    for (const Var &result : func.getResults()) {
      useDef[&result] = VarDef(VarDef::Result);
    }
  }

  VarDef getDef(const Var &var) const {
    assert(useDef.find(&var) != useDef.end());
    return useDef.at(&var);
  }

  const_iterator begin() const {return const_iterator(useDef.begin());}
  const_iterator end() const {return const_iterator(useDef.end());}

private:
  std::map<const Var*, VarDef> useDef;
};

std::ostream &operator<<(std::ostream &os, const UseDef &ud);

}} // namespace simit::ir

#endif
