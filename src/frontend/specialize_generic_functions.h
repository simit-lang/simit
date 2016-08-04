#ifndef SIMIT_SPECIALIZE_GENERIC_FUNCTIONS_H
#define SIMIT_SPECIALIZE_GENERIC_FUNCTIONS_H

#include <list>
#include <string>
#include <unordered_map>

#include "fir.h"
#include "fir_visitor.h"

namespace simit {
namespace fir {

class SpecializeGenericFunctions : public FIRVisitor {
public:
  SpecializeGenericFunctions(const std::vector<fir::FuncDecl::Ptr> &intrinsics)
      : count(0), intrinsics(intrinsics) {}

  void specialize(Program::Ptr);

private:
  virtual void visit(Program::Ptr);
  virtual void visit(FuncDecl::Ptr);
  virtual void visit(CallExpr::Ptr);
  virtual void visit(MapExpr::Ptr);

  void clone(FuncDecl::Ptr, const std::string &);
  void cloneIfGeneric(const Identifier::Ptr &funcName);

private:
  typedef std::unordered_map<std::string, FuncDecl::Ptr> FuncMap;
  typedef std::unordered_map<std::string, std::list<FuncDecl::Ptr>> FuncListMap;

  struct FindGenericFuncs : public FIRVisitor {
    FindGenericFuncs(FuncMap &genericFuncs) : genericFuncs(genericFuncs) {}

    void find(Program::Ptr program) { program->accept(this); }

    virtual void visit(FuncDecl::Ptr);

    FuncMap &genericFuncs;
  };

private:
  FuncMap     genericFuncs;
  FuncListMap specializedFuncs;
  unsigned    count;
  const std::vector<fir::FuncDecl::Ptr> &intrinsics;
};

}
}

#endif
