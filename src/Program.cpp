#include "Program.h"
#include "Frontend.h"
#include "Logger.h"
#include "IR.h"

using namespace Simit;
using namespace std;

Program::Program() : frontend(new Frontend()) {}

Program::~Program() {}

void output_errors(string errors) {
  cerr << "Errors: " << errors << endl;
}

int Program::loadString(string program) {
  string errors;
  int status = loadString(program, errors);
  if (status != 0) {
    output_errors(errors);
  }
  return status;
}

int Program::loadString(string program, string &errors) {
  vector<shared_ptr<IRNode> > irNodes;
  return frontend->parseString(program, irNodes, errors);
}

int Program::loadFile(std::string filename) {
  string errors;
  int status = loadFile(filename, errors);
  if (status != 0) {
    output_errors(errors);
  }
  return status;
}

int Program::loadFile(std::string filename, string &errors) {
  vector<shared_ptr<IRNode> > irNodes;
  return frontend->parseFile(filename, irNodes, errors);
}

int Program::compile() {
  return 0;
}
