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
  enum Kind { Argument, Result, Assignment, Map, Undefined };

  explicit VarDef(Kind kind=Undefined, Stmt stmt = Stmt(), unsigned loc=0)
      : kind(kind), stmt(stmt), loc(loc) {}

  Kind getKind() const {return kind;}

  Stmt getStmt() const {return stmt;}

  unsigned getLoc() const {return loc;}

private:
  Kind kind;
  Stmt stmt;
  unsigned loc;
};

std::ostream &operator<<(std::ostream &os, const VarDef &vd);


class UseDef : public IRVisitor {
public:
  class const_iterator {
  public:
    const_iterator(std::map<Var,VarDef>::const_iterator it) : it(it) {}
    const_iterator& operator++() {++it; return *this;}
    const Var &operator*() const {return it->first;}

    friend bool operator==(const const_iterator &l, const const_iterator &r) {
      return l.it == r.it;
    }

    friend bool operator!=(const const_iterator &l, const const_iterator &r) {
      return l.it != r.it;
    }

  private:
    std::map<Var,VarDef>::const_iterator it;
  };

  UseDef(Func func) {
    for (const Var &arg : func.getArguments()) {
      usedef[arg] = VarDef(VarDef::Argument);
    }

    for (const Var &result : func.getResults()) {
      usedef[result] = VarDef(VarDef::Result);
    }

    func.accept(this);
  }

  VarDef getDef(const Var &var) const {
    simit_iassert(usedef.find(var) != usedef.end());
    return usedef.at(var);
  }

  const_iterator begin() const {return const_iterator(usedef.begin());}
  const_iterator end() const {return const_iterator(usedef.end());}

private:
  std::map<Var,VarDef> usedef;
  
  using IRVisitor::visit;

  void visit(const AssignStmt *op) {
    usedef[op->var] = VarDef(VarDef::Assignment, op);
  }

  void visit(const Map *op) {
    for (size_t i=0; i<op->vars.size(); ++i) {
      usedef[op->vars[i]] = VarDef(VarDef::Map, op, i);
    }
  }
};

std::ostream &operator<<(std::ostream &os, const UseDef &ud);

}} // namespace simit::ir

#endif
