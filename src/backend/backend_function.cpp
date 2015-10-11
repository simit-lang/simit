#include "backend_function.h"

#include <iostream>
using namespace std;

#include "graph.h"

#include "ir.h"
#include "ir_visitor.h"
#include "indices.h"
#include "util/collections.h"
#include "error.h"

namespace simit {
namespace backend {

// class Function
Function::Function(const ir::Func& func)
    : environment(new ir::Environment(func.getEnvironment())) {
  for (const ir::Var& arg : func.getArguments()) {
    string argName = arg.getName();
    arguments.push_back(argName);
    argumentTypes[argName] = arg.getType();
    // System-level arguments should be considered outputs
    if (arg.getType().isSet() ||
        (arg.getType().isTensor() &&
         arg.getType().toTensor()->hasSystemDimensions())) {
      results.insert(argName);
    }
  }

  for (const ir::Var& res : func.getResults()) {
    string resName = res.getName();
    arguments.push_back(resName);
    argumentTypes[resName] = res.getType();
    results.insert(resName);
  }

  // Gather the Simit literal expressions and store them in an array in the
  // function, to prevent their memory from being reclaimed if the IR is
  // deleted. This is necessary because compiled functions are expected to
  // retrieve these values when being run.
  class GatherLiteralsVisitor : private simit::ir::IRVisitor {
  public:
    vector<simit::ir::Expr> gather(simit::ir::Func func) {
      literals.clear();

      // Gather literals in global constant initializers
      for (auto &global : func.getEnvironment().getConstants()) {
        global.second.accept(this);
      }

      // Gather literals in the function body
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
  delete environment;
}

bool Function::hasArg(std::string arg) const {
  return util::contains(argumentTypes, arg);
}

const std::vector<std::string>& Function::getArgs() const {
  return arguments;
}

const ir::Type& Function::getArgType(std::string arg) const {
  return argumentTypes.at(arg);
}

bool Function::isResult(std::string name) const {
  return results.count(name);
}

bool Function::hasGlobal(std::string name) const {
  return environment->hasExtern(name);
}

const ir::Type& Function::getGlobalType(std::string name) const {
  uassert(hasGlobal(name)) << "No global called " << name << " in function";
  return environment->getExtern(name).getVar().getType();
}

bool Function::hasBindable(std::string bindable) const {
  iassert(!(hasArg(bindable) && hasGlobal(bindable)));
  return hasArg(bindable) || hasGlobal(bindable);
}

const ir::Type& Function::getBindableType(std::string bindable) const {
  iassert(hasBindable(bindable));
  return (hasArg(bindable)) ? getArgType(bindable) : getGlobalType(bindable);
}

const ir::Environment& Function::getEnvironment() const {
  return *environment;
}

}}
