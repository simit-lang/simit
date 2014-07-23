#include "Program.h"
#include "Frontend.h"
#include "Logger.h"
#include "IR.h"
#include "Util.h"
#include "Errors.h"

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
