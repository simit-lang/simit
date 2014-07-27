#include "program.h"

#include <memory>

#include "ir.h"
#include "frontend.h"
#include "logger.h"
#include "util.h"
#include "errors.h"

using namespace simit;
using namespace std;

namespace simit {
namespace internal {
class Program {
  friend class simit::Program;
  Program() : frontend(new Frontend()) {}
  unique_ptr<Frontend> frontend;
  std::list<shared_ptr<IRNode>> sources;
};
}
}

/* Program */
Program::Program() : impl(new internal::Program) {
}

Program::~Program() {
  delete impl;
}

int Program::loadString(string programString) {
  int errorCode = impl->frontend->parseString(programString);
  return errorCode;
}

int Program::loadFile(std::string filename) {
  int errorCode = impl->frontend->parseFile(filename);
  return errorCode;
}

int Program::compile() {
  return 0;
}

string Program::getErrorString() {
  return util::join(impl->frontend->getErrors(), "\n");
}

std::list<Error> &Program::getErrors() {
  return impl->frontend->getErrors();
}
