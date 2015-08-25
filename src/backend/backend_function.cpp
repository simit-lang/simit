#include "backend_function.h"

#include <iostream>
using namespace std;

#include "graph.h"

#include "ir.h"
#include "ir_visitor.h"
#include "indices.h"
#include "util/collections.h"

namespace simit {
namespace backend {

// class Function
Function::Function(const ir::Func& func) {
  for (const ir::Var& arg : func.getArguments()) {
    string argName = arg.getName();
    formals.push_back(argName);
    formalTypes[argName] = arg.getType();
  }

  for (const ir::Var& res : func.getResults()) {
    string resName = res.getName();
    formals.push_back(resName);
    formalTypes[resName] = res.getType();
  }

  // Gather the Simit literal expressions and store them in an array in the
  // function, to prevent their memory from being reclaimed if the IR is
  // deleted. This is necessary because compiled functions are expected to
  // retrieve these values when being run.
  class GatherLiteralsVisitor : private simit::ir::IRVisitor {
  public:
    vector<simit::ir::Expr> gather(simit::ir::Func func) {
      literals.clear();
      for (auto &global : func.getEnvironment().globals) {
        global.second.accept(this);
      }

      func.accept(this);
      return literals;
    }
  private:
    vector<simit::ir::Expr> literals;
    using simit::ir::IRVisitor::visit;
    void visit(const simit::ir::Literal *op) {
      literals.push_back(op);
    }
  };
  literals = GatherLiteralsVisitor().gather(func);
}

Function::~Function() {
}

bool Function::hasArg(std::string arg) const {
  return formalTypes.find(arg) != formalTypes.end();
}

const std::vector<std::string>& Function::getArgs() const {
  return formals;
}

const ir::Type& Function::getArgType(std::string arg) const {
  return formalTypes.at(arg);
}

}}
