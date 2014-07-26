#include "program.h"

#include "frontend.h"
#include "logger.h"
#include "ir.h"
#include "util.h"
#include "errors.h"

using namespace simit;
using namespace std;


/* Program */
Program::Program() : frontend(new Frontend()) {}

Program::~Program() {
  delete frontend;
}

int Program::loadString(string programString) {
  return frontend->parseString(programString);
}

int Program::loadFile(std::string filename) {
  return frontend->parseFile(filename);
}

string Program::getErrorString() {
  return util::join(frontend->getErrors(), "\n");
}

std::list<simit::Error> &Program::getErrors() {
  return frontend->getErrors();
}

int Program::compile() {
  return 0;
}
