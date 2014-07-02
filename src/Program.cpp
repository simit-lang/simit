#include "Program.h"
#include "Frontend.h"
#include "Logger.h"
#include "IR.h"

using namespace Simit;
using namespace std;

Program::Program() : frontend(new Frontend()) {}

Program::~Program() {}

int Program::load(std::string programText) {
  string errors;
  int status = load(programText, errors);
  if (status != 0) {
    cerr << errors;
  }
  return status;
}

int Program::load(std::string programText, std::string &errors) {
  std::vector<std::shared_ptr<IRNode> > irNodes;
  return frontend->parse(programText, &irNodes, &errors);
}

int Program::load(std::ifstream programFile) {
  return 1;
}

int Program::compile() {
  return 0;
}
