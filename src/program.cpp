#include "program.h"

#include <set>
#include <vector>

#include "ir.h"
#include "frontend/frontend.h"
#include "util/util.h"
#include "error.h"
#include "program_context.h"
#include "storage.h"
#include "lower/lower.h"
#include "timers.h"

#include "backend/backend.h"

using namespace std;

namespace simit {

const std::vector<std::string> VALID_BACKENDS = {
  "cpu",
#ifdef GPU
  "gpu",
#endif
};
std::string kBackend;

static
Function compile(ir::Func func, backend::Backend *backend, bool addTimers) {
  ir::Storage storage;
  // Fill in storage path expressions, etc.
  /// map<Var,pe::PathExpressions> pes = assignPathExpressions(func);
  /// storage.addPathExpressions(pes);
  func = lower(func, false, addTimers);
  return Function(backend->compile(func, storage));
}

static Function compile(ir::Func func, backend::Backend *backend) {
  return simit::compile(func, backend, false);
}

// class ProgramContent
struct Program::ProgramContent {
  internal::ProgramContext ctx;
  internal::Frontend *frontend;
  backend::Backend   *backend;
  Diagnostics diags;
};

// class Program
Program::Program() : content(new ProgramContent) {
  content->frontend = new internal::Frontend();
  content->backend =  new backend::Backend(kBackend);
}

Program::~Program() {
  clear();
}

void Program::clear() {
  delete content->frontend;
  delete content->backend;
  delete content;
  content = nullptr;
}

int Program::loadString(const string &programString) {
  std::vector<ParseError> errors;
  int status = content->frontend->parseString(programString, &content->ctx,
                                              &errors);
  for (auto &error : errors) {
    content->diags.report() << error.toString();
  }
  uassert(errors.size() == 0) << "during compilation:" << std::endl 
                              << content->diags.getMessage();
  return status;
}

int Program::loadFile(const std::string &filename) {
  uassert(ifstream(filename).good()) << "Could not load file: " << filename;
  std::vector<ParseError> errors;
  int status = content->frontend->parseFile(filename, &content->ctx, &errors);
  for (auto &error : errors) {
    content->diags.report() << error.toString();
  }
  uassert(errors.size() == 0) << "during compilation:" << std::endl 
                              << content->diags.getMessage();
  return status;
}

std::vector<std::string> Program::getFunctionNames() const {
  vector<string> functionNames;
  for (auto &func : content->ctx.getFunctions()) {
    functionNames.push_back(func.first);
  }
  return functionNames;
}

Function Program::compile(const std::string &function) {
  ir::Func simitFunc = content->ctx.getFunction(function);
  uassert(simitFunc.defined()) << "Attempting to compile an unknown function "
                               << "(" << function << ")";
  return simit::compile(simitFunc, content->backend);
}

Function Program::compileWithTimers(const std::string &function) {
  ir::Func simitFunc = content->ctx.getFunction(function);
  uassert(simitFunc.defined()) << "Attempting to compile an unknown function "
                               << "(" << function << ")";
  return simit::compile(simitFunc, content->backend, true);
}

int Program::verify() {
  // For each test look up the called function. Grab the actual arguments and
  // run the function with them as input.  Then compare the result to the
  // expected literal.
  const std::map<string, ir::Func> &functions = content->ctx.getFunctions();
  std::map<ir::Func, simit::Function*> compiled;

  for (auto &test : content->ctx.getTests()) {
    if (functions.find(test->getCallee()) == functions.end()) {
      content->diags.report() << "Error: attempting to test unknown function "
      << "'" << test->getCallee() << "'";
      return 1;
    }
    ir::Func func = functions.at(test->getCallee());
    Function compiledFunc = simit::compile(func, content->backend);

    bool evaluates = test->evaluate(func, compiledFunc, &content->diags);
    if (!evaluates) {
      return 2;
    }
  }

  return 0;
}

bool Program::hasErrors() const {
  return content->diags.hasErrors();
}

const Diagnostics &Program::getDiagnostics() const {
  return content->diags;
}

std::ostream &operator<<(std::ostream &os, const Program &program) {
  auto it = program.content->ctx.getFunctions().begin();
  auto end = program.content->ctx.getFunctions().end();
  if (it != end) {
    os << it->second << endl;
    ++it;
  }
  while (it != end) {
    os << endl << it->second << endl;
    ++it;
  }
  return os;
}

} // namespace simit
