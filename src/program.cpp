#include "program.h"

#include <set>
#include <vector>

#include "ir.h"
#include "frontend.h"
#include "util.h"
#include "error.h"
#include "program_context.h"
#include "storage.h"
#include "lower/lower.h"

#include "backend/llvm/llvm_backend.h"
#ifdef GPU
#include "backend/gpu/gpu_backend.h"
#endif

using namespace std;

namespace simit {

const std::vector<std::string> VALID_BACKENDS = {
  "llvm",
#ifdef GPU
  "gpu",
#endif
};
std::string kBackend;

static Function compile(ir::Func func, backend::Backend *backend) {
  func = lower(func);
  return Function(backend->compile(func));
}


// class ProgramContent
struct Program::ProgramContent {
  internal::ProgramContext ctx;
  internal::Frontend *frontend;
  backend::Backend *backend;
  Diagnostics diags;
};

// class Program
Program::Program() : content(new ProgramContent) {
  content->frontend = new internal::Frontend();
  if (kBackend == "llvm") {
    content->backend  = new backend::LLVMBackend();
  }
#ifdef GPU
  else if (kBackend == "gpu") {
    content->backend = new backend::GPUBackend();
  }
#endif
  else {
    ierror << "Invalid backend choice: " << kBackend
           << ". Did you forget to call simit::init()?";
  }
}

Program::~Program() {
  delete content->frontend;
  delete content->backend;
  delete content;
}

int Program::loadString(const string &programString) {
  std::vector<ParseError> errors;
  int status = content->frontend->parseString(programString, &content->ctx,
                                              &errors);
  for (auto &error : errors) {
    content->diags.report() << error.toString();
  }
  return status;
}

int Program::loadFile(const std::string &filename) {
  std::vector<ParseError> errors;
  int status = content->frontend->parseFile(filename, &content->ctx, &errors);
  for (auto &error : errors) {
    content->diags.report() << error.toString();
  }
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
  if (!simitFunc.defined()) {
    content->diags.report() << "Attempting to compile an unknown function ("
                            << function << ")";
    return NULL;
  }
  return simit::compile(simitFunc, content->backend);
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
