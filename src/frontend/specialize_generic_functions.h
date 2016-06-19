#ifndef SIMIT_SPECIALIZE_GENERIC_FUNCTIONS_H
#define SIMIT_SPECIALIZE_GENERIC_FUNCTIONS_H

#include <list>
#include <unordered_map>

#include "hir.h"
#include "hir_visitor.h"

namespace simit {
namespace hir {

class SpecializeGenericFunctions : public HIRVisitor {
public:
  void specialize(Program::Ptr);

private:
  virtual void visit(FuncDecl::Ptr);
  virtual void visit(CallExpr::Ptr);
  //virtual void visit(MapExpr::Ptr);

private:
  std::unordered_map<std::string, FuncDecl::Ptr> genericFuncs;
  std::unordered_map<std::string, std::list<FuncDecl::Ptr>> specializedFuncs;
};

}
}

#endif

