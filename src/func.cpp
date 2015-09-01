#include "func.h"

#include "ir.h"

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
  visitor->visit(this);
}

std::ostream &operator<<(std::ostream& os, const Func& func) {
  os << "func " << func.getName() << "(";
  if (func.getArguments().size() > 0) {
    const Var &arg = func.getArguments()[0];
    os << arg << " : " << arg.getType();
  }
  for (size_t i=1; i < func.getArguments().size(); ++i) {
    const Var &arg = func.getArguments()[i];
    os << ", " << arg << " : " << arg.getType();
  }
  os << ")";

  if (func.getResults().size() > 0) {
    os << " -> (";
    const Var &res = func.getResults()[0];
    os << res << " : " << res.getType();

    for (size_t i=1; i < func.getResults().size(); ++i) {
      const Var &res = func.getResults()[i];
      os << ", " << res << " : " << res.getType();
    }
    os << ")";
  }

  if (func.getBody().defined()) {
    os << ":" << std::endl;

    IRPrinter printer(os, 1);
    printer.print(func.getBody());
  }
  else {
    os << ";";
  }
  return os;
}

}}
