#include "func.h"

#include "ir.h"
#include "ir_printer.h"

namespace simit {
namespace ir {

// class FuncContent
FuncContent::~FuncContent() {
  delete body;
}

// class Func
Func::Func(const std::string& name, const std::vector<Var>& arguments,
           const std::vector<Var>& results, Kind kind)
    : Func(name, arguments, results, Stmt(), kind) {
  iassert(kind != Internal);
}

Func::Func(const std::string& name, const std::vector<Var>& arguments,
           const std::vector<Var>& results, const Stmt& body,
           const Environment& environment, Kind kind)
    : IntrusivePtr(new FuncContent) {
  ptr->kind = kind;
  ptr->name = name;
  ptr->arguments = arguments;
  ptr->results = results;
  ptr->env = environment;
  ptr->body = new Stmt();
  if (body.defined()) {
    *ptr->body = Scope::make(body);
  }
}

Func::Func(const std::string& name, const std::vector<Var>& arguments,
           const std::vector<Var>& results, const Stmt& body, Kind kind)
    : Func(name, arguments, results, body, Environment(), kind) {
}

Func::Func(const Func& func, Stmt body)
    : Func(func.getName(), func.getArguments(), func.getResults(), body,
           func.getEnvironment(), func.getKind()) {
  setStorage(func.getStorage());
}

Func::Func(const Func& func, const Environment& environment)
    : Func(func.getName(), func.getArguments(), func.getResults(),
           func.getBody(), environment, func.getKind()) {
}

Stmt Func::getBody() const {
  return *ptr->body;
}

void Func::accept(IRVisitorStrict *visitor) const {
  try {
    visitor->visit(this);
  }
  catch (SimitException &ex) {
    ex.addContext("func " + getName());
    throw;
  }
}

std::ostream &operator<<(std::ostream& os, const Func& func) {
  IRPrinter printer(os);
  printer.print(func);
  return os;
}

}}
