#include "Program.h"
#include "Frontend.h"
#include "Logger.h"
#include "IR.h"
#include "Test.h"

using namespace simit;
using namespace std;

Program::Program() : frontend(new Frontend()) {}

Program::~Program() {
  delete frontend;
  for (vector<IRNode*>::iterator it = irNodes.begin();
      it != irNodes.end(); ++it) {
    delete *it;
  }

  for (vector<Test*>::iterator it = tests.begin();
      it != tests.end(); ++it) {
    delete *it;
  }
}

void output_errors(string errors) {
  cerr << "Errors: " << errors << endl;
}

int Program::loadString(string programString) {
  return frontend->parseString(programString, this);
}

int Program::loadFile(std::string filename) {
  return frontend->parseFile(filename, this);
}

string Program::errors() {
  return errorString;
}

int Program::compile() {
  return 0;
}

void Program::addError(string errors) {
  errorString += errors;
}

void Program::addTest(Test *test) {
  tests.push_back(test);
}
