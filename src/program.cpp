#include "program.h"

#include <set>
#include <vector>

#include "ir.h"
#include "ir_rewriter.h"
#include "frontend.h"
#include "lower.h"
#include "llvm_backend.h"
#include "function.h"
#include "util.h"
#include "error.h"
#include "program_context.h"
#include "storage.h"
#include "temps.h"
#include "flatten.h"

#ifdef GPU
#include "gpu_backend/gpu_backend.h"
#include "gpu_backend/gpu_ir.h"
#endif

using namespace std;

namespace simit {

const std::vector<std::string> VALID_BACKENDS = {
  "llvm",
#ifdef GPU
  "gpu",
#endif
};
std::string kBackend = "llvm";

static Function *compile(ir::Func func, internal::Backend *backend) {
  class OptimizeFunctionsVisitor : public ir::IRRewriterCallGraph {
    using IRRewriter::visit;
    void visit(const ir::Func *op) {
      if (op->getKind() != ir::Func::Internal) {
        func = *op;
        return;
      }

      func = ir::Func(*op, rewrite(op->getBody()));
      func = insertTemporaries(func);
      func = flattenIndexExpressions(func);
      func.setStorage(getStorage(func));
      func = lower(func);
      #ifdef GPU
      if (kBackend == "gpu") {
        func = shardLoops(func);
      }
      #endif
    }
  };
  func = OptimizeFunctionsVisitor().rewrite(func);

  return backend->compile(func);
}


// class ProgramContent
struct Program::ProgramContent {
  internal::ProgramContext ctx;
  internal::Frontend *frontend;
  internal::Backend *backend;
  Diagnostics diags;
};

// class Program
Program::Program() : content(new ProgramContent) {
  content->frontend = new internal::Frontend();
  if (kBackend == "llvm") {
    content->backend  = new internal::LLVMBackend();
  }
#ifdef GPU
  else if (kBackend == "gpu") {
    content->backend = new internal::GPUBackend();
  }
#endif
  else {
    ierror << "Invalid backend choice";
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

std::unique_ptr<Function> Program::compile(const std::string &function,
                                           int floatSize) {
  ir::ScalarType::floatBytes = floatSize;
  ir::Func simitFunc = content->ctx.getFunction(function);
  if (!simitFunc.defined()) {
    content->diags.report() << "Attempting to compile an unknown function ("
                            << function << ")";
    return NULL;
  }
  return std::unique_ptr<Function>(simit::compile(simitFunc, content->backend));
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

      if (compiled.find(func) == compiled.end()) {
        compiled[func] = simit::compile(func, content->backend);
      }
    }

    for (auto &test : content->ctx.getTests()) {
      iassert(functions.find(test->getCallee()) != functions.end());
      ir::Func func = functions.at(test->getCallee());

      iassert(compiled.find(func) != compiled.end());
      Function *compiledFunc = compiled.at(func);

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
